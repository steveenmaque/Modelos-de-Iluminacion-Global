#version 430 core
// Pasada de presentacion: promedia el acumulador HDR, aplica exposicion,
// tone mapping (ACES o Reinhard) y correccion gamma a sRGB.
in vec2 vUV;
out vec4 fragColor;

uniform sampler2D uAccum;   // RGBA32F: suma de radiancias, .a = nº de muestras acumuladas
uniform float uExposure;    // multiplicador de exposicion
uniform int   uTonemap;     // 0 = ninguno, 1 = Reinhard, 2 = ACES
uniform float uGamma;       // tipicamente 2.2

vec3 acesFilmic(vec3 x) {
    // Aproximacion de Narkowicz a la curva ACES
    const float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec4 acc = texture(uAccum, vUV);
    float n = max(acc.a, 1.0);
    vec3 hdr = acc.rgb / n;            // radiancia media por muestra
    hdr *= uExposure;

    vec3 mapped = hdr;
    if (uTonemap == 1) mapped = hdr / (1.0 + hdr);          // Reinhard
    else if (uTonemap == 2) mapped = acesFilmic(hdr);       // ACES

    mapped = pow(max(mapped, 0.0), vec3(1.0 / uGamma));     // gamma -> sRGB
    fragColor = vec4(mapped, 1.0);
}
