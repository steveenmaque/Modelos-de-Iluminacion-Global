// scene.glsl - geometria analitica de la caja de Cornell e intersecciones.
// Requiere common.glsl (constantes MAT_*, EPS, INF) incluido antes.
// La sala es el cubo [-1,1]^3; la cara frontal (z=+1) esta abierta (camara).

// ---- Uniforms de escena (compartidos por trace.frag y photon.comp) ----
uniform int   uScene;          // 0=clasica, 1=cajas+vidrio, 2=vidrio+espejo, 3=completa
uniform vec3  uLightCenter;    // centro de la luz de area (en el techo)
uniform vec2  uLightHalf;      // semiejes (x,z) de la luz
uniform vec3  uLightEmission;  // radiancia emitida (Le)
uniform float uGlassIOR;       // indice de refraccion del vidrio
uniform vec3  uWallTint;       // multiplicador de color de paredes (para experimentar)

struct Hit {
    float t;
    vec3  p;
    vec3  n;
    vec3  albedo;
    vec3  emission;
    int   mat;
};

// ---- Colores de las paredes (caja de Cornell clasica) ----
const vec3 C_RED   = vec3(0.630, 0.065, 0.050);
const vec3 C_GREEN = vec3(0.140, 0.450, 0.091);
const vec3 C_WHITE = vec3(0.725, 0.710, 0.680);

// ---- Transformaciones de los objetos dinamicos ----
const vec3  TB_C = vec3(-0.35, -0.40, -0.28); const vec3 TB_H = vec3(0.30, 0.60, 0.30); const float TB_A =  0.29;
const vec3  SB_C = vec3( 0.37, -0.70,  0.34); const vec3 SB_H = vec3(0.30, 0.30, 0.30); const float SB_A = -0.30;
const vec3  GS_C = vec3( 0.42, -0.60,  0.28); const float GS_R = 0.40;  // esfera de vidrio
const vec3  MS_C = vec3(-0.42, -0.62,  0.36); const float MS_R = 0.38;  // esfera espejo
const vec3  MS3_C= vec3( 0.30, -0.82, -0.52); const float MS3_R= 0.18;  // espejo pequeno (escena completa)

// =====================  Primitivas  =====================
void tryRect(int ax, float c, vec2 mn, vec2 mx, vec3 nrm,
             vec3 alb, vec3 emi, int mat,
             vec3 ro, vec3 rd, inout Hit h, inout bool any) {
    float rda = rd[ax];
    if (abs(rda) < 1e-9) return;
    float t = (c - ro[ax]) / rda;
    if (t < EPS || t > h.t) return;
    vec3 p = ro + rd * t;
    int u = (ax + 1) % 3, w = (ax + 2) % 3;
    if (p[u] < mn.x || p[u] > mx.x || p[w] < mn.y || p[w] > mx.y) return;
    h.t = t; h.p = p; h.n = nrm; h.albedo = alb; h.emission = emi; h.mat = mat; any = true;
}

void trySphere(vec3 c, float r, vec3 alb, vec3 emi, int mat,
               vec3 ro, vec3 rd, inout Hit h, inout bool any) {
    vec3 oc = ro - c;
    float b = dot(oc, rd);
    float cc = dot(oc, oc) - r * r;
    float disc = b * b - cc;
    if (disc < 0.0) return;
    float s = sqrt(disc);
    float t = -b - s;
    if (t < EPS) t = -b + s;
    if (t < EPS || t > h.t) return;
    vec3 p = ro + rd * t;
    h.t = t; h.p = p; h.n = normalize(p - c);
    h.albedo = alb; h.emission = emi; h.mat = mat; any = true;
}

// Caja orientada (giro 'a' alrededor de Y).
void tryBoxY(vec3 cb, vec3 hsz, float a, vec3 alb, vec3 emi, int mat,
             vec3 ro, vec3 rd, inout Hit h, inout bool any) {
    float ca = cos(a), sa = sin(a);
    vec3 lo = ro - cb;
    vec3 rol = vec3(ca * lo.x + sa * lo.z, lo.y, -sa * lo.x + ca * lo.z);
    vec3 rdl = vec3(ca * rd.x + sa * rd.z, rd.y, -sa * rd.x + ca * rd.z);
    vec3 inv = 1.0 / rdl;
    vec3 t0 = (-hsz - rol) * inv;
    vec3 t1 = ( hsz - rol) * inv;
    vec3 tmn = min(t0, t1), tmx = max(t0, t1);
    float tn = max(max(tmn.x, tmn.y), tmn.z);
    float tf = min(min(tmx.x, tmx.y), tmx.z);
    if (tn > tf || tf < EPS) return;
    float t = (tn > EPS) ? tn : tf;
    if (t > h.t) return;
    vec3 pl = rol + rdl * t;
    vec3 d = abs(pl) - hsz;
    vec3 nl;
    if (d.x > d.y && d.x > d.z) nl = vec3(sign(pl.x), 0.0, 0.0);
    else if (d.y > d.z)        nl = vec3(0.0, sign(pl.y), 0.0);
    else                       nl = vec3(0.0, 0.0, sign(pl.z));
    vec3 nw = normalize(vec3(ca * nl.x - sa * nl.z, nl.y, sa * nl.x + ca * nl.z));
    h.t = t; h.p = ro + rd * t; h.n = nw; h.albedo = alb; h.emission = emi; h.mat = mat; any = true;
}

// =====================  Escena completa  =====================
bool intersectScene(vec3 ro, vec3 rd, out Hit h) {
    h.t = INF; h.emission = vec3(0.0); h.mat = MAT_DIFFUSE;
    bool any = false;
    vec3 wt = uWallTint;

    // Caja de la sala (5 caras; frontal abierta)
    tryRect(0, -1.0, vec2(-1.0), vec2(1.0), vec3( 1.0, 0.0, 0.0), C_RED   * wt, vec3(0.0), MAT_DIFFUSE, ro, rd, h, any); // izq roja
    tryRect(0,  1.0, vec2(-1.0), vec2(1.0), vec3(-1.0, 0.0, 0.0), C_GREEN * wt, vec3(0.0), MAT_DIFFUSE, ro, rd, h, any); // der verde
    tryRect(1, -1.0, vec2(-1.0), vec2(1.0), vec3( 0.0, 1.0, 0.0), C_WHITE * wt, vec3(0.0), MAT_DIFFUSE, ro, rd, h, any); // suelo
    tryRect(2, -1.0, vec2(-1.0), vec2(1.0), vec3( 0.0, 0.0, 1.0), C_WHITE * wt, vec3(0.0), MAT_DIFFUSE, ro, rd, h, any); // fondo

    // Luz de area (se prueba antes que el techo para que prevalezca)
    vec2 lmn = uLightCenter.xz - uLightHalf;
    vec2 lmx = uLightCenter.xz + uLightHalf;
    tryRect(1, 1.0, lmn, lmx, vec3(0.0, -1.0, 0.0), vec3(0.0), uLightEmission, MAT_EMISSIVE, ro, rd, h, any);
    // Techo blanco
    tryRect(1, 1.0, vec2(-1.0), vec2(1.0), vec3(0.0, -1.0, 0.0), C_WHITE * wt, vec3(0.0), MAT_DIFFUSE, ro, rd, h, any);

    // Objetos dinamicos segun la escena
    if (uScene == 0) {                 // clasica: dos cajas difusas
        tryBoxY(TB_C, TB_H, TB_A, C_WHITE, vec3(0.0), MAT_DIFFUSE, ro, rd, h, any);
        tryBoxY(SB_C, SB_H, SB_A, C_WHITE, vec3(0.0), MAT_DIFFUSE, ro, rd, h, any);
    } else if (uScene == 1) {          // caja difusa + esfera de vidrio
        tryBoxY(TB_C, TB_H, TB_A, C_WHITE, vec3(0.0), MAT_DIFFUSE, ro, rd, h, any);
        trySphere(GS_C, GS_R, vec3(1.0), vec3(0.0), MAT_DIELECTRIC, ro, rd, h, any);
    } else if (uScene == 2) {          // vidrio + espejo
        trySphere(GS_C, GS_R, vec3(1.0),  vec3(0.0), MAT_DIELECTRIC, ro, rd, h, any);
        trySphere(MS_C, MS_R, vec3(0.97), vec3(0.0), MAT_MIRROR,     ro, rd, h, any);
    } else {                           // completa: caja + vidrio + espejo pequeno
        tryBoxY(TB_C, TB_H, TB_A, C_WHITE, vec3(0.0), MAT_DIFFUSE, ro, rd, h, any);
        trySphere(GS_C,  GS_R,  vec3(1.0),  vec3(0.0), MAT_DIELECTRIC, ro, rd, h, any);
        trySphere(MS3_C, MS3_R, vec3(0.97), vec3(0.0), MAT_MIRROR,     ro, rd, h, any);
    }
    return any;
}

// Visibilidad (sombra) entre dos puntos: ¿hay algo entre 'from' y 'to'?
bool occluded(vec3 from, vec3 to) {
    vec3 d = to - from;
    float dist = length(d);
    d /= dist;
    Hit h;
    if (!intersectScene(from + d * EPS, d, h)) return false;
    return h.t < dist - 2.0 * EPS;
}

// Muestrea un punto en la luz de area y devuelve su normal/area.
vec3 sampleLight(vec2 u, out vec3 nl, out float area) {
    vec3 p;
    p.x = uLightCenter.x + (u.x * 2.0 - 1.0) * uLightHalf.x;
    p.z = uLightCenter.z + (u.y * 2.0 - 1.0) * uLightHalf.y;
    p.y = uLightCenter.y;
    nl = vec3(0.0, -1.0, 0.0);
    area = (2.0 * uLightHalf.x) * (2.0 * uLightHalf.y);
    return p;
}
