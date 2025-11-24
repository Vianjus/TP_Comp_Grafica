#include "TreeRenderer.h"
#include "glad/glad.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

TreeRenderer::TreeRenderer() : shaderProgram(0), VAO(0), VBO(0), lineWidth(2.0f) {
    color[0] = 0.2f; color[1] = 0.8f; color[2] = 0.3f;
}

TreeRenderer::~TreeRenderer() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);
}

bool TreeRenderer::initialize() {
    // Shader atualizado com suporte a transformações
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        uniform mat4 transform;
        void main() {
            gl_Position = transform * vec4(aPos, 0.0, 1.0);
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 color;
        void main() {
            FragColor = vec4(color, 1.0);
        }
    )";
    
    shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    if (!shaderProgram) return false;
    
    // Configura VAO e VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    
    std::cout << "TreeRenderer inicializado com sucesso" << std::endl;
    return true;
}

// NOVO MÉTODO: Aplica transformações
void TreeRenderer::applyTransform(const float* transformMatrix) {
    glUseProgram(shaderProgram);
    GLuint transformLoc = glGetUniformLocation(shaderProgram, "transform");
    if (transformLoc != -1) {
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transformMatrix);
    }
}

void TreeRenderer::render(const std::vector<Segment>& segments) {
    static bool firstRender = true;
    
    if (segments.empty()) {
        if (firstRender) {
            std::cout << "Nenhuma arvore carregada, renderizando arvore de teste..." << std::endl;
            firstRender = false;
        }
        std::vector<Segment> testSegments = createTestTree();
        renderSegments(testSegments);
        return;
    }
    
    if (firstRender) {
        std::cout << "Renderizando arvore com " << segments.size() << " segmentos" << std::endl;
        firstRender = false;
    }
    
    renderSegments(segments);
}

std::vector<Segment> TreeRenderer::createTestTree() {
    std::vector<Segment> testSegments;
    
    // Árvore mais realista
    Segment seg;
    
    // Tronco principal
    seg = {Point2D(0.0f, -1.0f), Point2D(0.0f, -0.2f), 0.1f, 0.08f};
    testSegments.push_back(seg);
    
    // Ramos principais
    seg = {Point2D(0.0f, -0.2f), Point2D(0.4f, 0.3f), 0.08f, 0.05f};
    testSegments.push_back(seg);
    seg = {Point2D(0.0f, -0.2f), Point2D(-0.4f, 0.2f), 0.08f, 0.05f};
    testSegments.push_back(seg);
    
    // Sub-ramos
    seg = {Point2D(0.4f, 0.3f), Point2D(0.7f, 0.6f), 0.05f, 0.02f};
    testSegments.push_back(seg);
    seg = {Point2D(-0.4f, 0.2f), Point2D(-0.6f, 0.5f), 0.05f, 0.02f};
    testSegments.push_back(seg);
    
    // Ramos basais
    seg = {Point2D(0.0f, -0.6f), Point2D(0.3f, -0.9f), 0.06f, 0.03f};
    testSegments.push_back(seg);
    seg = {Point2D(0.0f, -0.6f), Point2D(-0.3f, -0.9f), 0.06f, 0.03f};
    testSegments.push_back(seg);
    
    return testSegments;
}

void TreeRenderer::renderSegments(const std::vector<Segment>& segments) {
    std::vector<float> vertices;
    for (const auto& segment : segments) {
        vertices.push_back(segment.start.x);
        vertices.push_back(segment.start.y);
        vertices.push_back(segment.end.x);
        vertices.push_back(segment.end.y);
    }
    
    glUseProgram(shaderProgram);
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), color[0], color[1], color[2]);
    glLineWidth(lineWidth);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glDrawArrays(GL_LINES, 0, vertices.size() / 2);
    glBindVertexArray(0);
}



std::string TreeRenderer::loadShaderSource(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir shader: " << filepath << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

unsigned int TreeRenderer::compileShader(const std::string& source, unsigned int type) {
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Erro de compilação do shader: " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

unsigned int TreeRenderer::createShaderProgram(const std::string& vertexSource, const std::string& fragmentSource) {
    unsigned int vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    
    if (!vertexShader || !fragmentShader) {
        std::cerr << "Erro: Shaders não compilados corretamente" << std::endl;
        return 0;
    }
    
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Erro de linking do programa: " << infoLog << std::endl;
        glDeleteProgram(program);
        program = 0;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}