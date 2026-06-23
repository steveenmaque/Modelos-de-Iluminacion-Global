// common.glsl - constantes, generador aleatorio y muestreo.
// (Sin #version: se inserta via #include en un shader que ya lo declara.)

const float PI     = 3.14159265358979323846;
const float INV_PI = 0.31830988618379067154;
const float EPS    = 1e-3;
const float INF    = 1e30;

// Factor de punto fijo para acumular flujo de fotones con enteros atomicos.
const float CAUSTIC_SCALE = 2048.0;

// Tipos de material
const int MAT_DIFFUSE    = 0;
const int MAT_MIRROR     = 1;
const int MAT_DIELECTRIC = 2;
const int MAT_EMISSIVE   = 3;

// Modos / algoritmos
const int MODE_LOCAL    = 0;  // Phong directo (iluminacion local)
const int MODE_WHITTED  = 1;  // trazado de rayos recursivo
const int MODE_RADIOSITY= 2;  // GI difusa (radiosidad por Monte Carlo)
const int MODE_PATH     = 3;  // path tracing completo
const int MODE_PHOTON   = 4;  // mapeo de fotones (causticas)
const int MODE_AO       = 5;  // oclusion ambiental / GI tiempo real

// ---------------------------------------------------------------------------
// PRNG: PCG hash + secuencia. 'gSeed' es estado por hilo.
uint gSeed;

uint pcgHash(uint v) {
    uint state = v * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

void seedRng(uvec3 p) {
    gSeed = pcgHash(p.x + pcgHash(p.y + pcgHash(p.z)));
}

float rnd() {
    gSeed = gSeed * 747796405u + 2891336453u;
    uint word = ((gSeed >> ((gSeed >> 28u) + 4u)) ^ gSeed) * 277803737u;
    word = (word >> 22u) ^ word;
    return float(word) * (1.0 / 4294967296.0);
}

vec2 rnd2() { return vec2(rnd(), rnd()); }

// ---------------------------------------------------------------------------
// Muestreo
// Base ortonormal a partir de una normal (Duff et al. 2017)
void onb(vec3 n, out vec3 t, out vec3 b) {
    float s = (n.z >= 0.0) ? 1.0 : -1.0;
    float a = -1.0 / (s + n.z);
    float d = n.x * n.y * a;
    t = vec3(1.0 + s * n.x * n.x * a, s * d, -s * n.x);
    b = vec3(d, s + n.y * n.y * a, -n.y);
}

// Direccion en el hemisferio orientado por 'n', ponderada por coseno.
vec3 cosineSampleHemisphere(vec3 n, vec2 u) {
    float r = sqrt(u.x);
    float phi = 2.0 * PI * u.y;
    float x = r * cos(phi);
    float y = r * sin(phi);
    float z = sqrt(max(0.0, 1.0 - u.x));
    vec3 t, b;
    onb(n, t, b);
    return normalize(t * x + b * y + n * z);
}

// Direccion uniforme en el hemisferio (para oclusion ambiental).
vec3 uniformSampleHemisphere(vec3 n, vec2 u) {
    float z = u.x;
    float r = sqrt(max(0.0, 1.0 - z * z));
    float phi = 2.0 * PI * u.y;
    vec3 t, b;
    onb(n, t, b);
    return normalize(t * (r * cos(phi)) + b * (r * sin(phi)) + n * z);
}

float maxComp(vec3 v) { return max(v.x, max(v.y, v.z)); }

// Reflectancia de Fresnel (Schlick) para dielectricos.
float fresnelSchlick(float cosTheta, float n1, float n2) {
    float r0 = (n1 - n2) / (n1 + n2);
    r0 = r0 * r0;
    float m = clamp(1.0 - cosTheta, 0.0, 1.0);
    return r0 + (1.0 - r0) * m * m * m * m * m;
}

// Dispersion en un dielectrico (Fresnel estocastico + reflexion total interna).
// I = direccion incidente normalizada, N = normal geometrica. Devuelve la nueva direccion.
vec3 scatterDielectric(vec3 I, vec3 N, float ior) {
    float cosi = clamp(dot(I, N), -1.0, 1.0);
    vec3 n = N;
    float eta;
    if (cosi < 0.0) { cosi = -cosi; eta = 1.0 / ior; }   // entra al medio
    else            { n = -N;       eta = ior;       }   // sale del medio
    float k = 1.0 - eta * eta * (1.0 - cosi * cosi);
    float F = (k < 0.0) ? 1.0 : fresnelSchlick(cosi, 1.0, ior);
    if (rnd() < F) return reflect(I, n);
    return normalize(eta * I + (eta * cosi - sqrt(k)) * n);
}
