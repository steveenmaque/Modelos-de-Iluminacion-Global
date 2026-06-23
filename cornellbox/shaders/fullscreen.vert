#version 430 core
// Triangulo a pantalla completa sin VBO (se dibuja con glDrawArrays(GL_TRIANGLES,0,3)).
out vec2 vUV;
void main() {
    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    vUV = p;                       // [0,2] -> cubre [0,1] en pantalla
    gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
