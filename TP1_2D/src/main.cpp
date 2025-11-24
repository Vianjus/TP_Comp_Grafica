#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "VTKLoader.h"
#include "TreeRenderer.h"

using namespace std;

// Variáveis globais
TreeRenderer treeRenderer;
VTKLoader vtkLoader;
vector<string> treeFiles;
size_t currentTreeIndex = 0;

// Controles
float backgroundColor[3] = {0.1f, 0.1f, 0.1f};
float translation[2] = {0.0f, 0.0f};
float scale = 1.0f;
float rotation = 0.0f;
bool showWireframe = false;

// Matriz de transformação
float transformMatrix[16] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

void loadTreeFiles() {
    // Adicione aqui os caminhos para seus arquivos VTK
    treeFiles = {
        "data/Nterm_064/tree2D_Nterm0064_step0064.vtk",
        "data/Nterm_128/tree2D_Nterm0128_step0128.vtk",
        "data/Nterm_256/tree2D_Nterm0256_step0256.vtk"
    };
    
    // Para teste, crie um arquivo simples se não existir
    cout << "Arquivos de arvore carregados: " << treeFiles.size() << endl;
}

void updateTransformMatrix() {
    // Reset para matriz identidade
    for(int i = 0; i < 16; i++) transformMatrix[i] = 0.0f;
    transformMatrix[0] = transformMatrix[5] = transformMatrix[10] = transformMatrix[15] = 1.0f;
    
    // Aplica translação
    transformMatrix[12] = translation[0];
    transformMatrix[13] = translation[1];
    
    // Aplica rotação (simplificado - para 2D)
    float cosR = cos(rotation);
    float sinR = sin(rotation);
    float temp[16] = {
        cosR, -sinR, 0.0f, 0.0f,
        sinR,  cosR, 0.0f, 0.0f,
        0.0f,  0.0f, 1.0f, 0.0f,
        0.0f,  0.0f, 0.0f, 1.0f
    };
    
    // Multiplica matrizes (simplificado)
    float result[16] = {0};
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            for(int k = 0; k < 4; k++) {
                result[i*4+j] += transformMatrix[i*4+k] * temp[k*4+j];
            }
        }
    }
    
    // Aplica escala
    for(int i = 0; i < 8; i++) {
        result[i] *= scale;
    }
    
    // Copia resultado
    for(int i = 0; i < 16; i++) {
        transformMatrix[i] = result[i];
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, true);
                break;
            case GLFW_KEY_R:
                // Reset transformações
                translation[0] = translation[1] = 0.0f;
                scale = 1.0f;
                rotation = 0.0f;
                updateTransformMatrix();
                cout << "Transformacoes resetadas" << endl;
                break;
            case GLFW_KEY_W:
                // Alternar wireframe
                showWireframe = !showWireframe;
                glPolygonMode(GL_FRONT_AND_BACK, showWireframe ? GL_LINE : GL_FILL);
                cout << "Wireframe: " << (showWireframe ? "ON" : "OFF") << endl;
                break;
            case GLFW_KEY_RIGHT:
                // Próxima árvore
                if (!treeFiles.empty()) {
                    currentTreeIndex = (currentTreeIndex + 1) % treeFiles.size();
                    if (vtkLoader.loadFile(treeFiles[currentTreeIndex])) {
                        cout << "Arvore carregada: " << treeFiles[currentTreeIndex] << endl;
                    }
                }
                break;
            case GLFW_KEY_LEFT:
                // Árvore anterior
                if (!treeFiles.empty()) {
                    currentTreeIndex = (currentTreeIndex == 0) ? treeFiles.size() - 1 : currentTreeIndex - 1;
                    if (vtkLoader.loadFile(treeFiles[currentTreeIndex])) {
                        cout << "Arvore carregada: " << treeFiles[currentTreeIndex] << endl;
                    }
                }
                break;
            case GLFW_KEY_1:
                // Cor vermelha
                treeRenderer.setColor(1.0f, 0.0f, 0.0f);
                cout << "Cor: Vermelho" << endl;
                break;
            case GLFW_KEY_2:
                // Cor verde
                treeRenderer.setColor(0.0f, 1.0f, 0.0f);
                cout << "Cor: Verde" << endl;
                break;
            case GLFW_KEY_3:
                // Cor azul
                treeRenderer.setColor(0.0f, 0.0f, 1.0f);
                cout << "Cor: Azul" << endl;
                break;
        }
    }
}

void processInput(GLFWwindow* window) {
    // Controles de transformação
    bool transformed = false;
    
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { 
        translation[0] -= 0.01f; 
        transformed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { 
        translation[0] += 0.01f; 
        transformed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { 
        translation[1] += 0.01f; 
        transformed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { 
        translation[1] -= 0.01f; 
        transformed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) { 
        scale *= 1.01f; 
        transformed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) { 
        scale /= 1.01f; 
        transformed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) { 
        rotation += 0.02f; 
        transformed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) { 
        rotation -= 0.02f; 
        transformed = true;
    }
    
    if (transformed) {
        updateTransformMatrix();
    }
}

int main() {
    // Inicializa GLFW
    if (!glfwInit()) {
        cout << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1200, 800, "TP1 - Visualizacao de Arvores Arteriais 2D", NULL, NULL);
    if (!window) {
        cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    // Inicializa GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD\n";
        return -1;
    }

    // Inicializa renderizador
    if (!treeRenderer.initialize()) {
        cout << "Failed to initialize tree renderer\n";
        return -1;
    }

    // Carrega arquivos
    loadTreeFiles();
    if (!treeFiles.empty()) {
        vtkLoader.loadFile(treeFiles[0]);
    }

    // Configuração OpenGL
    glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 1.0f);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    
    // Inicializa matriz de transformação
    updateTransformMatrix();

    cout << "=== TP1 - Visualizador de Arvores Arteriais 2D ===" << endl;
    cout << "Controles:" << endl;
    cout << "ESC - Sair" << endl;
    cout << "R - Resetar visualizacao" << endl;
    cout << "W - Alternar wireframe" << endl;
    cout << "SETAS - Navegar entre arvores" << endl;
    cout << "A/D - Mover horizontalmente" << endl;
    cout << "W/S - Mover verticalmente" << endl;
    cout << "Q/E - Zoom in/out" << endl;
    cout << "Z/X - Rotacionar" << endl;
    cout << "1/2/3 - Mudar cor (Vermelho/Verde/Azul)" << endl;

    // Loop principal
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        
        glClear(GL_COLOR_BUFFER_BIT);

        // Renderiza árvore
        treeRenderer.render(vtkLoader.getSegments());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    cout << "Programa finalizado!" << endl;
    return 0;
}