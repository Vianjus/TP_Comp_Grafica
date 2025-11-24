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
bool thickLines = false;

// Matriz de transformação
float transformMatrix[16] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

void loadTreeFiles() {
    treeFiles = {
        "data/Nterm_064/tree2D_Nterm0064_step0064.vtk",
        "data/Nterm_064/tree2D_Nterm0064_step0008.vtk",
        "data/Nterm_128/tree2D_Nterm0128_step0128.vtk",
        "data/Nterm_256/tree2D_Nterm0256_step0256.vtk"
    };
    cout << "Arquivos de arvore carregados: " << treeFiles.size() << endl;
}

void updateTransformMatrix() {
    // Reset para matriz identidade
    for(int i = 0; i < 16; i++) transformMatrix[i] = 0.0f;
    transformMatrix[0] = transformMatrix[5] = transformMatrix[10] = transformMatrix[15] = 1.0f;
    
    // Aplica escala
    transformMatrix[0] = scale;
    transformMatrix[5] = scale;
    
    // Aplica rotação
    float cosR = cos(rotation);
    float sinR = sin(rotation);
    transformMatrix[0] = cosR * scale;
    transformMatrix[1] = -sinR * scale;
    transformMatrix[4] = sinR * scale;
    transformMatrix[5] = cosR * scale;
    
    // Aplica translação
    transformMatrix[12] = translation[0];
    transformMatrix[13] = translation[1];
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    scale += yoffset * 0.1f;
    scale = max(0.1f, min(scale, 5.0f));
    updateTransformMatrix();
    cout << "Zoom: " << scale << endl;
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
            case GLFW_KEY_T:
                // Alternar wireframe
                showWireframe = !showWireframe;
                glPolygonMode(GL_FRONT_AND_BACK, showWireframe ? GL_LINE : GL_FILL);
                cout << "Wireframe: " << (showWireframe ? "ON" : "OFF") << endl;
                break;
            case GLFW_KEY_L:
                thickLines = !thickLines;
                treeRenderer.setLineWidth(thickLines ? 5.0f : 2.0f);
                cout << "Linhas grossas: " << (thickLines ? "ON" : "OFF") << endl;
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
                treeRenderer.setColor(1.0f, 0.0f, 0.0f);
                cout << "Cor: Vermelho" << endl;
                break;
            case GLFW_KEY_2:
                treeRenderer.setColor(0.0f, 1.0f, 0.0f);
                cout << "Cor: Verde" << endl;
                break;
            case GLFW_KEY_3:
                treeRenderer.setColor(0.0f, 0.0f, 1.0f);
                cout << "Cor: Azul" << endl;
                break;
            case GLFW_KEY_4: //Cor original
                treeRenderer.setColor(0.2f, 0.8f, 0.3f);
                cout << "Cor: Original (Verde)" << endl;
                break;
        }
    }
}

void processInput(GLFWwindow* window) {
    bool transformed = false;
    
    // Movimento com WASD
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
    
    // Rotação com Q/E
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) { 
        rotation += 0.02f; 
        transformed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) { 
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
        cout << "Falha ao inicializar GLFW" << endl;;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1200, 800, "TP1 - Visualizaçâo de Árvores Arteriais 2D", NULL, NULL);
    if (!window) {
        cout << "Falha ao criar janela GLFW" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Inicializa GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Falha ao iniciar GLAD\n";
        return -1;
    }

    // Inicializa renderizador
    if (!treeRenderer.initialize()) {
        cout << "Falha ao iniciar renderizacao da arvore\n";
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
    cout << "WASD - Mover (W = cima, S = baixo, A = esquerda, D = direita)" << endl;
    cout << "Q/E - Rotacionar" << endl;
    cout << "Scroll Mouse - Zoom" << endl;
    cout << "T - Alternar Wireframe" << endl;
    cout << "L - Alternar Linhas Grossas" << endl;
    cout << "SETAS - Navegar entre arvores" << endl;
    cout << "1/2/3/4 - Cores (Vermelho/Verde/Azul/Original)" << endl;
    cout << endl;

    // Loop principal
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        
        glClear(GL_COLOR_BUFFER_BIT);

        // Aplica transformações e renderiza
        treeRenderer.applyTransform(transformMatrix);
        treeRenderer.render(vtkLoader.getSegments());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    cout << "Programa finalizado!" << endl;
    return 0;
}