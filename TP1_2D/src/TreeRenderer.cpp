#include "TreeRenderer.h"
#include "glad/glad.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <queue>
#include <unordered_map>

TreeRenderer::TreeRenderer() : shaderProgram(0), VAO(0), VBO(0), lineWidth(2.0f), useMonochrome(false) {}

TreeRenderer::~TreeRenderer() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);
}

bool TreeRenderer::initialize() {
    // Shader para coloração hierárquica
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in float aHierarchyLevel; // Nível hierárquico (0 a 1)
        uniform mat4 transform;
        out float hierarchyLevel;
        
        void main() {
            gl_Position = transform * vec4(aPos, 0.0, 1.0);
            hierarchyLevel = aHierarchyLevel;
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 330 core
        in float hierarchyLevel;
        out vec4 FragColor;
        uniform bool monochrome;
        
        void main() {
            if (monochrome) {
                // Modo monocromático verde
                float intensity = 0.3 + hierarchyLevel * 0.5;
                FragColor = vec4(0.1, intensity, 0.2, 1.0);
            } else {
                // Gradiente hierárquico: Vermelho (raiz) -> Violeta/Azul (folhas)
                // hierarchyLevel = 0.0 (raiz) -> 1.0 (folhas)
                float red = 1.0 - hierarchyLevel * 0.8;
                float green = 0.1 + hierarchyLevel * 0.2;
                float blue = 0.2 + hierarchyLevel * 0.8;
                
                // Intensifica as cores
                red = clamp(red, 0.2, 1.0);
                green = clamp(green, 0.1, 0.4);
                blue = clamp(blue, 0.2, 1.0);
                
                FragColor = vec4(red, green, blue, 1.0);
            }
        }
    )";
    
    shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    if (!shaderProgram) return false;
    
    // Configura VAO e VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // Configura atributos: posição (2 floats) + nível hierárquico (1 float)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    std::cout << "TreeRenderer inicializado com sucesso" << std::endl;
    return true;
}

// NOVO: Encontra o segmento raiz (aquele que não tem pai)
int TreeRenderer::findRootSegment(const std::vector<Segment>& segments) {
    // Procura por segmentos que começam em pontos que não são finais de outros segmentos
    std::vector<bool> isEndPoint(segments.size() * 2, false);
    
    // Marca todos os pontos finais
    for (int i = 0; i < segments.size(); i++) {
        // Encontra índice do ponto final
        for (int j = 0; j < segments.size(); j++) {
            if (i != j) {
                // Verifica se o ponto final deste segmento é ponto inicial de outro
                float distStart = std::abs(segments[j].start.x - segments[i].end.x) + 
                                std::abs(segments[j].start.y - segments[i].end.y);
                if (distStart < 0.001f) {
                    isEndPoint[i] = true;
                    break;
                }
            }
        }
    }
    
    // O segmento raiz é aquele cujo ponto inicial não é ponto final de nenhum outro
    for (int i = 0; i < segments.size(); i++) {
        bool isStartPointOfOthers = false;
        for (int j = 0; j < segments.size(); j++) {
            if (i != j) {
                float distEnd = std::abs(segments[j].end.x - segments[i].start.x) + 
                              std::abs(segments[j].end.y - segments[i].start.y);
                if (distEnd < 0.001f) {
                    isStartPointOfOthers = true;
                    break;
                }
            }
        }
        if (!isStartPointOfOthers) {
            return i;
        }
    }
    
    return 0; // Fallback
}

// NOVO: Calcula profundidade hierárquica usando BFS
std::vector<int> TreeRenderer::calculateHierarchyDepth(const std::vector<Segment>& segments) {
    std::vector<int> depth(segments.size(), -1);
    
    if (segments.empty()) return depth;
    
    // Encontra a raiz
    int rootIndex = findRootSegment(segments);
    depth[rootIndex] = 0;
    
    // Fila para BFS
    std::queue<int> q;
    q.push(rootIndex);
    
    while (!q.empty()) {
        int current = q.front();
        q.pop();
        
        // Encontra filhos (segmentos que começam onde este termina)
        for (int i = 0; i < segments.size(); i++) {
            if (depth[i] == -1) { // Não visitado
                float dist = std::abs(segments[i].start.x - segments[current].end.x) + 
                           std::abs(segments[i].start.y - segments[current].end.y);
                if (dist < 0.001f) { // São conectados
                    depth[i] = depth[current] + 1;
                    q.push(i);
                }
            }
        }
    }
    
    // Encontra a profundidade máxima
    int maxDepth = 0;
    for (int d : depth) {
        if (d > maxDepth) maxDepth = d;
    }
    
    // Normaliza para 0-1
    for (int i = 0; i < depth.size(); i++) {
        if (depth[i] != -1 && maxDepth > 0) {
            depth[i] = depth[i]; // Mantemos inteiro por enquanto, normalizamos depois
        }
    }
    
    return depth;
}

void TreeRenderer::applyTransform(const float* transformMatrix) {
    glUseProgram(shaderProgram);
    GLuint transformLoc = glGetUniformLocation(shaderProgram, "transform");
    if (transformLoc != -1) {
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transformMatrix);
    }
    
    // Define modo de cor
    GLuint monoLoc = glGetUniformLocation(shaderProgram, "monochrome");
    if (monoLoc != -1) {
        glUniform1i(monoLoc, useMonochrome);
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
    
    // Árvore hierárquica mais clara
    Segment seg;
    
    // Tronco principal (nível 0)
    seg = {Point2D(0.0f, -1.0f), Point2D(0.0f, -0.5f), 0.1f, 0.08f};
    testSegments.push_back(seg);
    
    // Ramos primários (nível 1)
    seg = {Point2D(0.0f, -0.5f), Point2D(0.3f, -0.2f), 0.08f, 0.06f};
    testSegments.push_back(seg);
    seg = {Point2D(0.0f, -0.5f), Point2D(-0.3f, -0.2f), 0.08f, 0.06f};
    testSegments.push_back(seg);
    
    // Ramos secundários (nível 2)
    seg = {Point2D(0.3f, -0.2f), Point2D(0.5f, 0.1f), 0.06f, 0.04f};
    testSegments.push_back(seg);
    seg = {Point2D(-0.3f, -0.2f), Point2D(-0.5f, 0.1f), 0.06f, 0.04f};
    testSegments.push_back(seg);
    
    // Ramos terciários (nível 3 - folhas)
    seg = {Point2D(0.5f, 0.1f), Point2D(0.6f, 0.4f), 0.04f, 0.02f};
    testSegments.push_back(seg);
    seg = {Point2D(0.5f, 0.1f), Point2D(0.4f, 0.4f), 0.04f, 0.02f};
    testSegments.push_back(seg);
    seg = {Point2D(-0.5f, 0.1f), Point2D(-0.6f, 0.4f), 0.04f, 0.02f};
    testSegments.push_back(seg);
    seg = {Point2D(-0.5f, 0.1f), Point2D(-0.4f, 0.4f), 0.04f, 0.02f};
    testSegments.push_back(seg);
    
    return testSegments;
}

void TreeRenderer::renderSegments(const std::vector<Segment>& segments) {
    std::vector<float> vertices;
    
    // Calcula hierarquia
    std::vector<int> hierarchyDepth = calculateHierarchyDepth(segments);
    
    // Encontra profundidade máxima para normalização
    int maxDepth = 0;
    for (int depth : hierarchyDepth) {
        if (depth > maxDepth) maxDepth = depth;
    }
    
    if (maxDepth == 0) maxDepth = 1; // Evita divisão por zero
    
    // Prepara vértices com dados hierárquicos
    for (int i = 0; i < segments.size(); i++) {
        const auto& segment = segments[i];
        
        // Nível hierárquico normalizado (0 = raiz, 1 = folha mais profunda)
        float hierarchyLevel = static_cast<float>(hierarchyDepth[i]) / maxDepth;
        
        // Vértice inicial
        vertices.push_back(segment.start.x);
        vertices.push_back(segment.start.y);
        vertices.push_back(hierarchyLevel);
        
        // Vértice final
        vertices.push_back(segment.end.x);
        vertices.push_back(segment.end.y);
        vertices.push_back(hierarchyLevel);
    }
    
    glUseProgram(shaderProgram);
    glLineWidth(lineWidth);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glDrawArrays(GL_LINES, 0, vertices.size() / 3); // 3 componentes por vértice
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