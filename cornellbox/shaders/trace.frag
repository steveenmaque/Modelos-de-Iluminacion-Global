#version 430 core
// trace.frag - nucleo de render. Implementa los 6 algoritmos del informe sobre
// la caja de Cornell y escribe la SUMA de las muestras del frame (blending aditivo).
#include "common.glsl"
#include "scene.glsl"

in  vec2 vUV;
out vec4 fragColor;

// --- Camara ---
uniform vec2  uResolution;
uniform vec3  uCamPos, uCamFwd, uCamRight, uCamUp;
uniform float uTanHalfFovy, uAspect;

// --- Control de render ---
uniform int   uMode;
uniform int   uFrame;
uniform int   uSamplesPerFrame;
uniform int   uMaxBounces;
uniform int   uUseRR;
uniform int   uSoftShadows;
uniform float uClamp;

// --- Iluminacion local / Whitted (Phong) ---
uniform float uAmbient;
uniform float uKs;
uniform float uShininess;

// --- Oclusion ambiental / GI tiempo real ---
uniform int   uAOSamples;
uniform float uAORadius;
uniform float uAOAmbient;
uniform int   uAOView;       // 0 = combinada, 1 = solo oclusion (grises)

// --- Mapeo de fotones ---
uniform usampler2D uCaustic;
uniform float uCausticGain;
uniform int   uPhotonCount;
uniform int   uPhotonFG;

const vec3 BG = vec3(0.0);   // fondo (cara frontal abierta)

// ============ Iluminacion directa ============
// NEE para superficie difusa (estimador Monte Carlo de 1 muestra de la luz).
vec3 directDiffuse(vec3 p, vec3 n, vec3 albedo) {
    vec3 nl; float area;
    vec2 u = (uSoftShadows == 1) ? rnd2() : vec2(0.5);
    vec3 lp = sampleLight(u, nl, area);
    vec3 wi = lp - p;
    float d2 = dot(wi, wi);
    float dist = sqrt(d2);
    wi /= dist;
    float cosS = dot(n, wi);
    float cosL = dot(nl, -wi);
    if (cosS <= 0.0 || cosL <= 0.0) return vec3(0.0);
    if (occluded(p, lp)) return vec3(0.0);
    float G = (cosS * cosL) / d2;
    return albedo * INV_PI * uLightEmission * G * area;   // /pdf = *area
}

// Phong directo (difuso + especular) + termino ambiente: iluminacion LOCAL.
vec3 directPhong(vec3 p, vec3 n, vec3 wo, vec3 albedo, bool doSpec) {
    vec3 res = uAmbient * albedo;          // ambiente constante (aproxima la indirecta)
    vec3 nl; float area;
    vec2 u = (uSoftShadows == 1) ? rnd2() : vec2(0.5);
    vec3 lp = sampleLight(u, nl, area);
    vec3 wi = lp - p;
    float d2 = dot(wi, wi);
    float dist = sqrt(d2);
    wi /= dist;
    float cosS = dot(n, wi);
    float cosL = dot(nl, -wi);
    if (cosS > 0.0 && cosL > 0.0 && !occluded(p, lp)) {
        float G = (cosS * cosL) / d2;
        vec3 diff = albedo * INV_PI * uLightEmission * G * area;
        vec3 spec = vec3(0.0);
        if (doSpec) {
            vec3 r = reflect(-wi, n);
            float s = pow(max(dot(r, wo), 0.0), uShininess);
            spec = uKs * s * uLightEmission * INV_PI * G * area;
        }
        res += diff + spec;
    }
    return res;
}

// ============ Lectura del mapa de causticas (suelo) ============
vec3 causticAt(Hit h) {
    if (h.n.y < 0.9 || h.p.y > -0.99) return vec3(0.0);   // solo el suelo
    vec2 uv = h.p.xz * 0.5 + 0.5;
    ivec2 sz = textureSize(uCaustic, 0);
    uint v = texelFetch(uCaustic, ivec2(clamp(uv, 0.0, 1.0) * vec2(sz)), 0).r;
    float e = (float(v) / CAUSTIC_SCALE) / float(max(uPhotonCount, 1));
    return vec3(e) * uCausticGain;
}

// ============ Algoritmos ============

// (0) Iluminacion LOCAL: Phong directo, sin rebotes.
vec3 shadeLocal(vec3 ro, vec3 rd) {
    Hit h;
    if (!intersectScene(ro, rd, h)) return BG;
    if (h.mat == MAT_EMISSIVE) return h.emission;
    return directPhong(h.p, h.n, -rd, h.albedo, true);  // espejo/vidrio se ven como plastico
}

// (1) WHITTED: especular recursivo + Phong directo en difusas + sombras.
vec3 shadeWhitted(vec3 ro, vec3 rd) {
    vec3 thr = vec3(1.0), L = vec3(0.0);
    for (int depth = 0; depth <= uMaxBounces; ++depth) {
        Hit h;
        if (!intersectScene(ro, rd, h)) { L += thr * BG; break; }
        if (h.mat == MAT_EMISSIVE) { L += thr * h.emission; break; }
        if (h.mat == MAT_DIFFUSE) {
            L += thr * directPhong(h.p, h.n, -rd, h.albedo, true);
            break;                                  // Whitted no continua en difusas
        } else if (h.mat == MAT_MIRROR) {
            rd = reflect(rd, h.n); ro = h.p + rd * EPS; thr *= h.albedo;
        } else {                                    // dielectrico
            rd = scatterDielectric(rd, h.n, uGlassIOR); ro = h.p + rd * EPS;
        }
    }
    return L;
}

// (3) PATH TRACING / (2) RADIOSIDAD (forceDiffuse=true -> todo lambertiano).
vec3 shadePath(vec3 ro, vec3 rd, bool forceDiffuse) {
    vec3 thr = vec3(1.0), L = vec3(0.0);
    bool specularBounce = true;
    for (int depth = 0; depth < uMaxBounces; ++depth) {
        Hit h;
        if (!intersectScene(ro, rd, h)) { L += thr * BG; break; }

        int mat = h.mat;
        vec3 albedo = h.albedo;
        if (forceDiffuse && mat != MAT_EMISSIVE) {     // radiosidad: solo difusas
            albedo = (mat == MAT_DIFFUSE) ? h.albedo : vec3(0.7);
            mat = MAT_DIFFUSE;
        }

        if (mat == MAT_EMISSIVE) {
            if (specularBounce) L += thr * h.emission;  // evita doble conteo con NEE
            break;
        }
        if (mat == MAT_DIFFUSE) {
            L += thr * directDiffuse(h.p, h.n, albedo);
            specularBounce = false;
            rd = cosineSampleHemisphere(h.n, rnd2());
            ro = h.p + h.n * EPS;
            thr *= albedo;
        } else if (mat == MAT_MIRROR) {
            specularBounce = true;
            rd = reflect(rd, h.n); ro = h.p + rd * EPS; thr *= albedo;
        } else {
            specularBounce = true;
            rd = scatterDielectric(rd, h.n, uGlassIOR); ro = h.p + rd * EPS;
        }
        if (uUseRR == 1 && depth > 3) {
            float q = clamp(maxComp(thr), 0.05, 1.0);
            if (rnd() > q) break;
            thr /= q;
        }
    }
    return L;
}

// (4) MAPEO DE FOTONES: especular hasta la primera difusa + directo + causticas
// + rebotes difusos limitados (final gather) para el sangrado de color.
vec3 shadePhoton(vec3 ro, vec3 rd) {
    vec3 thr = vec3(1.0), L = vec3(0.0);
    bool specular = true;
    int diffBounces = 0;
    for (int depth = 0; depth < uMaxBounces; ++depth) {
        Hit h;
        if (!intersectScene(ro, rd, h)) { break; }
        if (h.mat == MAT_EMISSIVE) { if (specular) L += thr * h.emission; break; }
        if (h.mat == MAT_DIFFUSE) {
            L += thr * (directDiffuse(h.p, h.n, h.albedo) + h.albedo * causticAt(h));
            if (++diffBounces > uPhotonFG) break;
            specular = false;
            rd = cosineSampleHemisphere(h.n, rnd2());
            ro = h.p + h.n * EPS; thr *= h.albedo;
        } else if (h.mat == MAT_MIRROR) {
            if (!specular) break;                 // no reentrar a especular tras difusa
            rd = reflect(rd, h.n); ro = h.p + rd * EPS; thr *= h.albedo;
        } else {
            if (!specular) break;
            rd = scatterDielectric(rd, h.n, uGlassIOR); ro = h.p + rd * EPS;
        }
    }
    return L;
}

// (5) OCLUSION AMBIENTAL / GI tiempo real.
vec3 shadeAO(vec3 ro, vec3 rd) {
    Hit h;
    if (!intersectScene(ro, rd, h)) return BG;
    if (h.mat == MAT_EMISSIVE) return h.emission;
    float ao = 0.0;
    int K = max(uAOSamples, 1);
    for (int i = 0; i < K; ++i) {
        vec3 d = cosineSampleHemisphere(h.n, rnd2());
        Hit hh;
        if (intersectScene(h.p + h.n * EPS, d, hh) && hh.t < uAORadius) ao += 0.0;
        else ao += 1.0;
    }
    ao /= float(K);
    if (uAOView == 1) return vec3(ao);          // solo oclusion: blanco=abierto, negro=encerrado
    vec3 direct  = directDiffuse(h.p, h.n, h.albedo);
    vec3 ambient = uAOAmbient * h.albedo * ao;
    return direct + ambient;
}

vec3 shade(vec3 ro, vec3 rd) {
    if (uMode == MODE_LOCAL)     return shadeLocal(ro, rd);
    if (uMode == MODE_WHITTED)   return shadeWhitted(ro, rd);
    if (uMode == MODE_RADIOSITY) return shadePath(ro, rd, true);
    if (uMode == MODE_PATH)      return shadePath(ro, rd, false);
    if (uMode == MODE_PHOTON)    return shadePhoton(ro, rd);
    return shadeAO(ro, rd);
}

void main() {
    vec3 sum = vec3(0.0);
    int spp = max(uSamplesPerFrame, 1);
    for (int s = 0; s < spp; ++s) {
        seedRng(uvec3(uint(gl_FragCoord.x), uint(gl_FragCoord.y),
                      uint(uFrame * spp + s) + 1u));
        vec2 jitter = rnd2() - 0.5;
        vec2 ndc = ((gl_FragCoord.xy + jitter) / uResolution) * 2.0 - 1.0;
        vec3 rd = normalize(uCamFwd
                          + ndc.x * uAspect * uTanHalfFovy * uCamRight
                          + ndc.y * uTanHalfFovy * uCamUp);
        vec3 c = shade(uCamPos, rd);
        c = min(c, vec3(uClamp));          // anti-firefly
        sum += c;
    }
    fragColor = vec4(sum, float(spp));     // blending aditivo -> acumulador
}
