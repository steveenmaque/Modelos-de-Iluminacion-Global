// main.cpp - Caja de Cornell: Modelos Globales de Iluminacion (C++ / OpenGL)
// Path tracer progresivo en GPU con 6 algoritmos conmutables y menu ImGui.
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "shader.h"
#include "mathx.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <vector>
#include <string>
#include <algorithm>

// Modos (deben coincidir con common.glsl)
enum { MODE_LOCAL = 0, MODE_WHITTED, MODE_RADIOSITY, MODE_PATH, MODE_PHOTON, MODE_AO };

static const char* kModeNames[] = {
    "0  Iluminacion local (Phong)",
    "1  Trazado de rayos (Whitted)",
    "2  Radiosidad (GI difusa)",
    "3  Path tracing",
    "4  Mapeo de fotones (causticas)",
    "5  Oclusion ambiental / GI tiempo real"
};

struct ResOpt { int w, h; const char* name; };
static const ResOpt kRes[] = {
    {640, 360,  "640 x 360"},
    {960, 540,  "960 x 540"},
    {1280, 720, "1280 x 720  (720p)"},
    {1600, 900, "1600 x 900"},
    {1920, 1080,"1920 x 1080 (1080p)"},
};

static const char* kSceneNames[] = {
    "0  Clasica (2 cajas difusas)",
    "1  Caja difusa + esfera de vidrio",
    "2  Vidrio + espejo",
    "3  Completa (caja + vidrio + espejo)"
};

struct Params {
    int   mode = MODE_PATH;
    int   resIndex = 2;          // 720p
    int   scene = 3;             // completa
    int   samplesPerFrame = 2;
    int   maxBounces = 8;
    bool  useRR = true;
    bool  softShadows = true;
    float clampVal = 8.0f;

    // Local / Whitted
    float ambient = 0.08f;
    float ks = 0.4f;
    float shininess = 80.0f;

    // Oclusion ambiental
    int   aoSamples = 3;
    float aoRadius = 1.0f;
    float aoAmbient = 1.3f;
    int   aoView = 0;            // 0 = combinada, 1 = solo oclusion (grises)

    // Fotones
    int   photonCount = 200000;
    int   photonFG = 1;
    float causticGain = 2.5f;

    // Material / luz
    float glassIOR = 1.5f;
    float lightColor[3] = {1.0f, 0.92f, 0.78f};
    float lightIntensity = 18.0f;
    float lightHalf = 0.28f;
    float wallTint[3] = {1.0f, 1.0f, 1.0f};

    // Presentacion
    float exposure = 1.0f;
    int   tonemap = 2;           // ACES
    float gamma = 2.2f;
    bool  vsync = true;
};

// ---- estado global de GL ----
static GLuint gAccumTex = 0, gAccumFBO = 0;
static GLuint gShotTex = 0,  gShotFBO = 0;
static GLuint gCausticTex = 0;
static int gRenderW = 0, gRenderH = 0;
static const int kCausticRes = 512;

// helpers de uniforms
static void u1i(GLuint p, const char* n, int v)   { int l = glGetUniformLocation(p, n); if (l >= 0) glUniform1i(l, v); }
static void u1f(GLuint p, const char* n, float v) { int l = glGetUniformLocation(p, n); if (l >= 0) glUniform1f(l, v); }
static void u2f(GLuint p, const char* n, float a, float b)            { int l = glGetUniformLocation(p, n); if (l >= 0) glUniform2f(l, a, b); }
static void u3f(GLuint p, const char* n, float a, float b, float c)   { int l = glGetUniformLocation(p, n); if (l >= 0) glUniform3f(l, a, b, c); }

static void makeAccum(int w, int h) {
    if (gAccumTex) { glDeleteTextures(1, &gAccumTex); glDeleteFramebuffers(1, &gAccumFBO); }
    if (gShotTex)  { glDeleteTextures(1, &gShotTex);  glDeleteFramebuffers(1, &gShotFBO); }
    gRenderW = w; gRenderH = h;

    // Acumulador HDR RGBA32F
    glGenTextures(1, &gAccumTex);
    glBindTexture(GL_TEXTURE_2D, gAccumTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenFramebuffers(1, &gAccumFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gAccumFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gAccumTex, 0);

    // FBO de captura RGBA8
    glGenTextures(1, &gShotTex);
    glBindTexture(GL_TEXTURE_2D, gShotTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenFramebuffers(1, &gShotFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gShotFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gShotTex, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void makeCaustic() {
    glGenTextures(1, &gCausticTex);
    glBindTexture(GL_TEXTURE_2D, gCausticTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, kCausticRes, kCausticRes, 0,
                 GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLuint zero = 0;
    glClearTexImage(gCausticTex, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);
}

static void clearAccum() {
    glBindFramebuffer(GL_FRAMEBUFFER, gAccumFBO);
    glViewport(0, 0, gRenderW, gRenderH);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void saveScreenshot(GLuint presentProg, GLuint vao, const Params& p, long samples) {
    // Renderiza el acumulador a un FBO limpio (sin ImGui) y guarda PNG.
    glBindFramebuffer(GL_FRAMEBUFFER, gShotFBO);
    glViewport(0, 0, gRenderW, gRenderH);
    glDisable(GL_BLEND);
    glUseProgram(presentProg);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gAccumTex);
    u1i(presentProg, "uAccum", 0);
    u1f(presentProg, "uExposure", p.exposure);
    u1i(presentProg, "uTonemap", p.tonemap);
    u1f(presentProg, "uGamma", p.gamma);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    std::vector<unsigned char> px(gRenderW * gRenderH * 4);
    glReadPixels(0, 0, gRenderW, gRenderH, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    // voltear verticalmente
    std::vector<unsigned char> flip(px.size());
    for (int y = 0; y < gRenderH; ++y)
        std::memcpy(&flip[(gRenderH - 1 - y) * gRenderW * 4], &px[y * gRenderW * 4], gRenderW * 4);

    char name[256];
    std::time_t t = std::time(nullptr);
    std::tm* lt = std::localtime(&t);
    std::snprintf(name, sizeof(name), "captura_modo%d_%ldspp_%04d%02d%02d_%02d%02d%02d.png",
                  p.mode, samples,
                  lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
                  lt->tm_hour, lt->tm_min, lt->tm_sec);
    stbi_write_png(name, gRenderW, gRenderH, 4, flip.data(), gRenderW * 4);
    std::printf("[captura] guardada: %s\n", name);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Emision fisica de la luz a partir de color * intensidad.
static void lightEmission(const Params& p, float Le[3]) {
    Le[0] = p.lightColor[0] * p.lightIntensity;
    Le[1] = p.lightColor[1] * p.lightIntensity;
    Le[2] = p.lightColor[2] * p.lightIntensity;
}

// Pasada de fotones (solo en modo MODE_PHOTON).
static void photonPass(GLuint prog, const Params& p, int frame, const float Le[3]) {
    GLuint zero = 0;
    glClearTexImage(gCausticTex, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);
    glUseProgram(prog);
    u1i(prog, "uPhotonCount", p.photonCount);
    u1i(prog, "uPhotonFrame", frame);
    u1i(prog, "uMaxBounces", p.maxBounces);
    u1i(prog, "uScene", p.scene);
    u3f(prog, "uLightCenter", 0.0f, 1.0f, 0.0f);
    u2f(prog, "uLightHalf", p.lightHalf, p.lightHalf);
    u3f(prog, "uLightEmission", Le[0], Le[1], Le[2]);
    u1f(prog, "uGlassIOR", p.glassIOR);
    u3f(prog, "uWallTint", p.wallTint[0], p.wallTint[1], p.wallTint[2]);
    glBindImageTexture(0, gCausticTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glDispatchCompute((p.photonCount + 63) / 64, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

// Traza un frame y lo SUMA al acumulador (blending aditivo).
static void traceFrame(GLuint prog, GLuint vao, const Params& p,
                       const OrbitCamera& cam, int frame, const float Le[3]) {
    Vec3 fwd, right, up; cam.basis(fwd, right, up);
    Vec3 pos = cam.position();
    float aspect = (float)gRenderW / (float)gRenderH;
    float tanHalf = std::tan(radians(cam.fovY * 0.5f));

    glBindFramebuffer(GL_FRAMEBUFFER, gAccumFBO);
    glViewport(0, 0, gRenderW, gRenderH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glUseProgram(prog);
    u2f(prog, "uResolution", (float)gRenderW, (float)gRenderH);
    u3f(prog, "uCamPos", pos.x, pos.y, pos.z);
    u3f(prog, "uCamFwd", fwd.x, fwd.y, fwd.z);
    u3f(prog, "uCamRight", right.x, right.y, right.z);
    u3f(prog, "uCamUp", up.x, up.y, up.z);
    u1f(prog, "uTanHalfFovy", tanHalf);
    u1f(prog, "uAspect", aspect);
    u1i(prog, "uMode", p.mode);
    u1i(prog, "uFrame", frame);
    u1i(prog, "uSamplesPerFrame", p.samplesPerFrame);
    u1i(prog, "uMaxBounces", p.maxBounces);
    u1i(prog, "uUseRR", p.useRR ? 1 : 0);
    u1i(prog, "uSoftShadows", p.softShadows ? 1 : 0);
    u1f(prog, "uClamp", p.clampVal);
    u1f(prog, "uAmbient", p.ambient);
    u1f(prog, "uKs", p.ks);
    u1f(prog, "uShininess", p.shininess);
    u1i(prog, "uAOSamples", p.aoSamples);
    u1f(prog, "uAORadius", p.aoRadius);
    u1f(prog, "uAOAmbient", p.aoAmbient);
    u1i(prog, "uAOView", p.aoView);
    u1f(prog, "uCausticGain", p.causticGain);
    u1i(prog, "uPhotonCount", p.photonCount);
    u1i(prog, "uPhotonFG", p.photonFG);
    u1i(prog, "uScene", p.scene);
    u3f(prog, "uLightCenter", 0.0f, 1.0f, 0.0f);
    u2f(prog, "uLightHalf", p.lightHalf, p.lightHalf);
    u3f(prog, "uLightEmission", Le[0], Le[1], Le[2]);
    u1f(prog, "uGlassIOR", p.glassIOR);
    u3f(prog, "uWallTint", p.wallTint[0], p.wallTint[1], p.wallTint[2]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gCausticTex);
    u1i(prog, "uCaustic", 0);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisable(GL_BLEND);
}

int main(int argc, char** argv) {
    // Modo headless: ./cornellbox --render <modo> <escena> <spp> <frames> <resIdx> [out.png]
    bool headless = (argc >= 2 && std::strcmp(argv[1], "--render") == 0);

    // En Linux, GLEW usa GLX: pedimos el backend X11 (XWayland funciona). En
    // Windows/macOS este hint no aplica y se omite.
#if defined(__linux__) && defined(GLFW_PLATFORM_X11)
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif
    if (!glfwInit()) { std::fprintf(stderr, "Error: glfwInit\n"); return 1; }
    if (headless) glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* win = glfwCreateWindow(1280, 720,
        "Caja de Cornell - Modelos Globales de Iluminacion (C++/OpenGL)", nullptr, nullptr);
    if (!win) { std::fprintf(stderr, "Error: ventana GLFW\n"); glfwTerminate(); return 1; }
    glfwMakeContextCurrent(win);

    glewExperimental = GL_TRUE;
    GLenum gerr = glewInit();
    // GLEW_ERROR_NO_GLX_DISPLAY (4) se da bajo Wayland/EGL y es inofensivo.
    if (gerr != GLEW_OK && gerr != 4) {
        std::fprintf(stderr, "Error: glewInit (%s)\n", glewGetErrorString(gerr));
        return 1;
    }
    glGetError();  // descarta error espurio de GLEW

    std::printf("OpenGL %s\n", glGetString(GL_VERSION));
    std::printf("GPU    %s\n", glGetString(GL_RENDERER));

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    // Programas
    GLuint traceProg   = gfx::loadProgram("fullscreen.vert", "trace.frag");
    GLuint presentProg = gfx::loadProgram("fullscreen.vert", "present.frag");
    GLuint photonProg  = gfx::loadCompute("photon.comp");
    if (!traceProg || !presentProg || !photonProg) {
        std::fprintf(stderr, "Error: fallo al compilar shaders\n");
        return 1;
    }

    GLuint vao; glGenVertexArrays(1, &vao);   // VAO vacio para el triangulo a pantalla completa

    Params p;
    makeAccum(kRes[p.resIndex].w, kRes[p.resIndex].h);
    makeCaustic();
    clearAccum();
    glfwSwapInterval(p.vsync ? 1 : 0);

    OrbitCamera cam;
    cam.distance = 3.5f;
    cam.yaw = 0.0f;
    cam.pitch = 0.0f;
    cam.fovY = 40.0f;

    // ---------------- Modo headless: render a PNG y salir ----------------
    if (headless) {
        if (argc >= 3) p.mode  = std::atoi(argv[2]);
        if (argc >= 4) p.scene = std::atoi(argv[3]);
        if (argc >= 5) p.samplesPerFrame = std::max(1, std::atoi(argv[4]));
        int frames = (argc >= 6) ? std::max(1, std::atoi(argv[5])) : 64;
        if (argc >= 7) { p.resIndex = std::min(std::max(0, std::atoi(argv[6])), 4);
                         makeAccum(kRes[p.resIndex].w, kRes[p.resIndex].h); }
        if (argc >= 9) p.aoView = std::atoi(argv[8]);   // arg extra: vista AO (0/1)
        cam.yaw = 8.0f; cam.pitch = 4.0f;          // ligero angulo para lucir relieve
        float Le[3]; lightEmission(p, Le);
        clearAccum();
        double t0 = glfwGetTime();
        for (int f = 0; f < frames; ++f) {
            if (p.mode == MODE_PHOTON) photonPass(photonProg, p, f, Le);
            traceFrame(traceProg, vao, p, cam, f, Le);
        }
        glFinish();
        double dt = (glfwGetTime() - t0) * 1000.0;
        long spp = (long)frames * p.samplesPerFrame;
        const char* out = (argc >= 8) ? argv[7] : nullptr;
        // saveScreenshot genera su propio nombre; si dan 'out' lo respetamos abajo.
        glBindFramebuffer(GL_FRAMEBUFFER, gShotFBO);
        glViewport(0, 0, gRenderW, gRenderH);
        glDisable(GL_BLEND);
        glUseProgram(presentProg);
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, gAccumTex);
        u1i(presentProg, "uAccum", 0); u1f(presentProg, "uExposure", p.exposure);
        u1i(presentProg, "uTonemap", p.tonemap); u1f(presentProg, "uGamma", p.gamma);
        glBindVertexArray(vao); glDrawArrays(GL_TRIANGLES, 0, 3);
        std::vector<unsigned char> px(gRenderW * gRenderH * 4), flip(gRenderW * gRenderH * 4);
        glReadPixels(0, 0, gRenderW, gRenderH, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
        for (int y = 0; y < gRenderH; ++y)
            std::memcpy(&flip[(gRenderH - 1 - y) * gRenderW * 4], &px[y * gRenderW * 4], gRenderW * 4);
        char name[256];
        if (out) std::snprintf(name, sizeof(name), "%s", out);
        else std::snprintf(name, sizeof(name), "render_modo%d_escena%d_%ldspp.png", p.mode, p.scene, spp);
        stbi_write_png(name, gRenderW, gRenderH, 4, flip.data(), gRenderW * 4);
        std::printf("[render] %s  (%dx%d, %ld spp, %.1f ms)\n",
                    name, gRenderW, gRenderH, spp, dt);
        glfwDestroyWindow(win); glfwTerminate();
        return 0;
    }

    int frame = 0;
    long accumSamples = 0;
    double lastTime = glfwGetTime();
    double msAccum = 0.0; int msCount = 0; double msShown = 0.0;

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuiIO& io = ImGui::GetIO();

        bool reset = false;
        bool resizeRender = false;

        // -------- Menu --------
        ImGui::SetNextWindowSize(ImVec2(390, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::Begin("Panel de control");

        ImGui::TextWrapped("Algoritmo de iluminacion global");
        if (ImGui::Combo("##mode", &p.mode, kModeNames, IM_ARRAYSIZE(kModeNames))) reset = true;

        ImGui::SeparatorText("Escena y resolucion");
        if (ImGui::Combo("Escena", &p.scene, kSceneNames, IM_ARRAYSIZE(kSceneNames))) reset = true;
        {
            std::vector<const char*> rn; for (auto& r : kRes) rn.push_back(r.name);
            if (ImGui::Combo("Resolucion", &p.resIndex, rn.data(), (int)rn.size())) {
                resizeRender = true; reset = true;
            }
        }

        ImGui::SeparatorText("Muestreo");
        if (ImGui::SliderInt("Muestras / frame", &p.samplesPerFrame, 1, 16)) reset = true;
        if (ImGui::SliderInt("Rebotes maximos", &p.maxBounces, 1, 16)) reset = true;
        if (ImGui::Checkbox("Ruleta rusa", &p.useRR)) reset = true;
        ImGui::SameLine();
        if (ImGui::Checkbox("Sombras suaves", &p.softShadows)) reset = true;
        if (ImGui::SliderFloat("Recorte anti-firefly", &p.clampVal, 1.0f, 50.0f)) reset = true;

        if (p.mode == MODE_LOCAL || p.mode == MODE_WHITTED) {
            ImGui::SeparatorText("Phong (local / Whitted)");
            if (ImGui::SliderFloat("Ambiente", &p.ambient, 0.0f, 0.5f)) reset = true;
            if (ImGui::SliderFloat("Especular ks", &p.ks, 0.0f, 1.0f)) reset = true;
            if (ImGui::SliderFloat("Brillo (shininess)", &p.shininess, 1.0f, 256.0f)) reset = true;
        }
        if (p.mode == MODE_AO) {
            ImGui::SeparatorText("Oclusion ambiental");
            const char* aoViews[] = {"Combinada (luz + AO)", "Solo oclusion (grises)"};
            if (ImGui::Combo("Visualizacion", &p.aoView, aoViews, 2)) reset = true;
            if (ImGui::SliderInt("Muestras AO / frame", &p.aoSamples, 1, 16)) reset = true;
            if (ImGui::SliderFloat("Radio AO", &p.aoRadius, 0.05f, 2.0f)) reset = true;
            if (ImGui::SliderFloat("Ambiente AO", &p.aoAmbient, 0.0f, 3.0f)) reset = true;
        }
        if (p.mode == MODE_PHOTON) {
            ImGui::SeparatorText("Mapeo de fotones");
            if (ImGui::SliderInt("Fotones / frame", &p.photonCount, 10000, 1000000)) reset = true;
            if (ImGui::SliderInt("Rebotes difusos (final gather)", &p.photonFG, 0, 3)) reset = true;
            if (ImGui::SliderFloat("Ganancia causticas", &p.causticGain, 0.0f, 20.0f)) reset = true;
        }

        ImGui::SeparatorText("Luz y materiales");
        if (ImGui::ColorEdit3("Color de luz", p.lightColor)) reset = true;
        if (ImGui::SliderFloat("Intensidad de luz", &p.lightIntensity, 1.0f, 60.0f)) reset = true;
        if (ImGui::SliderFloat("Tamano de luz", &p.lightHalf, 0.05f, 0.6f)) reset = true;
        if (ImGui::SliderFloat("IOR del vidrio", &p.glassIOR, 1.0f, 2.5f)) reset = true;
        if (ImGui::ColorEdit3("Tinte de paredes", p.wallTint)) reset = true;

        ImGui::SeparatorText("Imagen (tone mapping)");
        ImGui::SliderFloat("Exposicion", &p.exposure, 0.05f, 5.0f);   // no resetea
        const char* tms[] = {"Ninguno", "Reinhard", "ACES"};
        ImGui::Combo("Tone map", &p.tonemap, tms, 3);
        ImGui::SliderFloat("Gamma", &p.gamma, 1.0f, 3.0f);
        if (ImGui::Checkbox("VSync", &p.vsync)) glfwSwapInterval(p.vsync ? 1 : 0);

        ImGui::SeparatorText("Acciones");
        if (ImGui::Button("Reiniciar acumulacion")) reset = true;
        ImGui::SameLine();
        if (ImGui::Button("Guardar captura PNG"))
            saveScreenshot(presentProg, vao, p, accumSamples);
        if (ImGui::Button("Reset camara")) { cam.yaw = 0; cam.pitch = 0; cam.distance = 3.5f; reset = true; }

        ImGui::SeparatorText("Metricas");
        ImGui::Text("Resolucion render : %d x %d", gRenderW, gRenderH);
        ImGui::Text("Muestras acumuladas: %ld", accumSamples);
        ImGui::Text("ms/frame: %.2f   FPS: %.1f", msShown, msShown > 0 ? 1000.0 / msShown : 0.0);
        ImGui::TextWrapped("Arrastra con el raton para orbitar; rueda para acercar.");
        ImGui::End();

        // -------- Camara (raton) --------
        if (!io.WantCaptureMouse) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                ImVec2 d = io.MouseDelta;
                if (d.x != 0.0f || d.y != 0.0f) {
                    cam.yaw   -= d.x * 0.3f;
                    cam.pitch += d.y * 0.3f;
                    cam.pitch = clampf(cam.pitch, -85.0f, 85.0f);
                    reset = true;
                }
            }
            if (io.MouseWheel != 0.0f) {
                cam.distance = clampf(cam.distance - io.MouseWheel * 0.25f, 1.6f, 8.0f);
                reset = true;
            }
        }

        // -------- Recursos / reinicio --------
        if (resizeRender) makeAccum(kRes[p.resIndex].w, kRes[p.resIndex].h);
        if (reset) { clearAccum(); frame = 0; accumSamples = 0; }

        // Emision fisica de la luz
        float Le[3]; lightEmission(p, Le);

        // -------- Pasada 1 (solo fotones): traza y deposita causticas --------
        if (p.mode == MODE_PHOTON) photonPass(photonProg, p, frame, Le);

        // -------- Pasada 2: traza la escena y ACUMULA (blending aditivo) --------
        traceFrame(traceProg, vao, p, cam, frame, Le);
        frame++;
        accumSamples += p.samplesPerFrame;

        // -------- Pasada 3: presentacion (tone mapping) con letterbox --------
        int winW, winH; glfwGetFramebufferSize(win, &winW, &winH);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, winW, winH);
        glClearColor(0.05f, 0.05f, 0.06f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float sc = std::min((float)winW / gRenderW, (float)winH / gRenderH);
        int vpW = (int)(gRenderW * sc), vpH = (int)(gRenderH * sc);
        glViewport((winW - vpW) / 2, (winH - vpH) / 2, vpW, vpH);
        glUseProgram(presentProg);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gAccumTex);
        u1i(presentProg, "uAccum", 0);
        u1f(presentProg, "uExposure", p.exposure);
        u1i(presentProg, "uTonemap", p.tonemap);
        u1f(presentProg, "uGamma", p.gamma);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // ImGui encima
        glViewport(0, 0, winW, winH);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(win);

        // timing
        double now = glfwGetTime();
        msAccum += (now - lastTime) * 1000.0; lastTime = now; msCount++;
        if (msAccum > 250.0) { msShown = msAccum / msCount; msAccum = 0; msCount = 0; }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
