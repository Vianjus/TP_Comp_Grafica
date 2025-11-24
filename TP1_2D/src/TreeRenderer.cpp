#include "TreeRenderer.h"
#include "glad/glad.h"
#include <fstream>
#include <sstream>
#include <iostream>

TreeRenderer::TreeRenderer() : shaderProgram(0), VAO(0), VBO(0), lineWidth(3.0f) {
    color[0] = 0.0f; color[1] = 0.8f; color[2] = 0.2f;
}

TreeRenderer::~TreeRenderer() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);
}

bool TreeRenderer::initialize() {
    // Shader simples para 2D
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
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

void TreeRenderer::render(const std::vector<Segment>& segments) {
    static bool firstRender = true; // Variável estática para controlar a mensagem
    
    if (segments.empty()) {
        if (firstRender) {
            std::cout << "Nenhuma arvore carregada, renderizando arvore de teste..." << std::endl;
            firstRender = false;
        }
        
        // Renderiza uma árvore de exemplo para teste
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
    Segment testSeg;
    
    // Cria uma árvore de teste mais elaborada
    // Tronco principal
    testSeg.start = {0.0f, -0.8f};
    testSeg.end = {0.0f, 0.0f};
    testSeg.startRadius = 0.08f;
    testSeg.endRadius = 0.06f;
    testSegments.push_back(testSeg);
    
    // Ramos à direita
    testSeg.start = {0.0f, 0.0f};
    testSeg.end = {0.4f, 0.3f};
    testSeg.startRadius = 0.06f;
    testSeg.endRadius = 0.03f;
    testSegments.push_back(testSeg);
    
    testSeg.start = {0.4f, 0.3f};
    testSeg.end = {0.6f, 0.5f};
    testSeg.startRadius = 0.03f;
    testSeg.endRadius = 0.01f;
    testSegments.push_back(testSeg);
    
    // Ramos à esquerda
    testSeg.start = {0.0f, 0.0f};
    testSeg.end = {-0.4f, 0.2f};
    testSeg.startRadius = 0.06f;
    testSeg.endRadius = 0.03f;
    testSegments.push_back(testSeg);
    
    testSeg.start = {-0.4f, 0.2f};
    testSeg.end = {-0.5f, 0.4f};
    testSeg.startRadius = 0.03f;
    testSeg.endRadius = 0.01f;
    testSegments.push_back(testSeg);
    
    // Ramos inferiores
    testSeg.start = {0.0f, -0.4f};
    testSeg.end = {0.2f, -0.7f};
    testSeg.startRadius = 0.05f;
    testSeg.endRadius = 0.02f;
    testSegments.push_back(testSeg);
    
    testSeg.start = {0.0f, -0.4f};
    testSeg.end = {-0.2f, -0.7f};
    testSeg.startRadius = 0.05f;
    testSeg.endRadius = 0.02f;
    testSegments.push_back(testSeg);
    
    return testSegments;
}

void TreeRenderer::renderTestTree(const std::vector<Segment>& segments) {
    // Este método agora é usado apenas para debug externo
    std::cout << "Renderizando arvore de teste com " << segments.size() << " segmentos" << std::endl;
    renderSegments(segments);
}

void TreeRenderer::renderSegments(const std::vector<Segment>& segments) {
    // Prepara dados dos segmentos
    std::vector<float> vertices;
    for (const auto& segment : segments) {
        vertices.push_back(segment.start.x);
        vertices.push_back(segment.start.y);
        vertices.push_back(segment.end.x);
        vertices.push_back(segment.end.y);
    }
    
    // Renderiza
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