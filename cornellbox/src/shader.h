// shader.h - utilidades para cargar/compilar shaders GLSL y fijar uniforms
#pragma once
#include <GL/glew.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#ifndef SHADER_DIR
#define SHADER_DIR "shaders"
#endif

namespace gfx {

inline std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) {
        std::cerr << "[shader] No se pudo abrir: " << path << "\n";
        return "";
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

inline std::string shaderPath(const std::string& name) {
    return std::string(SHADER_DIR) + "/" + name;
}

// Resuelve directivas  #include "fichero.glsl"  (un nivel, suficiente aqui).
inline std::string preprocess(const std::string& src) {
    std::stringstream in(src), out;
    std::string line;
    while (std::getline(in, line)) {
        // detecta:  #include "algo"
        size_t h = line.find("#include");
        if (h != std::string::npos) {
            size_t q1 = line.find('"', h);
            size_t q2 = (q1 == std::string::npos) ? std::string::npos : line.find('"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos) {
                std::string inc = line.substr(q1 + 1, q2 - q1 - 1);
                out << "// >>> include " << inc << "\n";
                out << readFile(shaderPath(inc)) << "\n";
                out << "// <<< include " << inc << "\n";
                continue;
            }
        }
        out << line << "\n";
    }
    return out.str();
}

inline GLuint compile(GLenum type, const std::string& src, const std::string& tag) {
    GLuint s = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(s, 1, &c, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len > 1 ? len : 1);
        glGetShaderInfoLog(s, len, nullptr, log.data());
        std::cerr << "[shader] Error compilando '" << tag << "':\n" << log.data() << "\n";
        glDeleteShader(s);
        return 0;
    }
    return s;
}

inline GLuint link(const std::vector<GLuint>& shaders, const std::string& tag) {
    GLuint p = glCreateProgram();
    for (GLuint s : shaders) glAttachShader(p, s);
    glLinkProgram(p);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(p, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len > 1 ? len : 1);
        glGetProgramInfoLog(p, len, nullptr, log.data());
        std::cerr << "[shader] Error enlazando '" << tag << "':\n" << log.data() << "\n";
        glDeleteProgram(p);
        return 0;
    }
    for (GLuint s : shaders) { glDetachShader(p, s); glDeleteShader(s); }
    return p;
}

// Programa grafico desde un vertex + fragment (ficheros en SHADER_DIR).
inline GLuint loadProgram(const std::string& vsName, const std::string& fsName) {
    std::string vs = preprocess(readFile(shaderPath(vsName)));
    std::string fs = preprocess(readFile(shaderPath(fsName)));
    if (vs.empty() || fs.empty()) return 0;
    GLuint v = compile(GL_VERTEX_SHADER, vs, vsName);
    GLuint f = compile(GL_FRAGMENT_SHADER, fs, fsName);
    if (!v || !f) return 0;
    return link({v, f}, vsName + "+" + fsName);
}

// Programa de computo desde un .comp.
inline GLuint loadCompute(const std::string& csName) {
    std::string cs = preprocess(readFile(shaderPath(csName)));
    if (cs.empty()) return 0;
    GLuint c = compile(GL_COMPUTE_SHADER, cs, csName);
    if (!c) return 0;
    return link({c}, csName);
}

} // namespace gfx
