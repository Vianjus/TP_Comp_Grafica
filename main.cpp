#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

using namespace std;

// Variáveis globais
float backgroundColor[3] = {0.0f, 0.0f, 0.0f};
bool mousePressed = false;
double mouseX = 0, mouseY = 0;

// Função callback para clique do mouse
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mousePressed = true;
            cout << "Mouse pressionado!\n";
            
            // Alterna entre azul e preto
            if (backgroundColor[2] == 0.0f) {
                backgroundColor[2] = 1.0f;
            } else {
                backgroundColor[2] = 0.0f;
            }
        } else if (action == GLFW_RELEASE) {
            mousePressed = false;
            cout << "Mouse liberado!\n";
        }
    }
}

// Função callback para movimento do mouse
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    mouseX = xpos;
    mouseY = ypos;
}

// Função callback para teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
        cout << "Fechando janela...\n";
    }
    
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        backgroundColor[0] = 1.0f;  // Vermelho
        backgroundColor[1] = 0.0f;
        backgroundColor[2] = 0.0f;
        cout << "Cor alterada para Vermelho!\n";
    }
    
    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        backgroundColor[0] = 0.0f;
        backgroundColor[1] = 1.0f;  // Verde
        backgroundColor[2] = 0.0f;
        cout << "Cor alterada para Verde!\n";
    }
    
    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        backgroundColor[0] = 0.0f;
        backgroundColor[1] = 0.0f;
        backgroundColor[2] = 1.0f;  // Azul
        cout << "Cor alterada para Azul!\n";
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    cout << "Janela redimensionada: " << width << "x" << height << "\n";
}

// Função para processar input contínuo (teclas pressionadas)
void processInput(GLFWwindow* window) {
    // Exemplo: W, A, S, D para controlar componentes de cor
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        backgroundColor[0] += 0.01f;  // Aumenta vermelho
        if (backgroundColor[0] > 1.0f) backgroundColor[0] = 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        backgroundColor[0] -= 0.01f;  // Diminui vermelho
        if (backgroundColor[0] < 0.0f) backgroundColor[0] = 0.0f;
    }
}

int main() {
    // Inicializa a biblioteca
    if (!glfwInit()) {
        cout << "Failed to initialize GLFW.\n";
        return -1;
    }

    // Configurações da janela OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Cria a janela
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Tutorial - Evoluído", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to open GLFW window.\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Configura callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);

    // Inicializa GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD.\n";
        return -1;
    }

    // Configuração inicial do OpenGL
    glViewport(0, 0, 800, 600);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    cout << "Controles:\n";
    cout << "- Clique com mouse: Alterna azul/preto\n";
    cout << "- R, G, B: Cores vermelho, verde, azul\n";
    cout << "- W/S: Controla componente vermelho\n";
    cout << "- ESC: Sair\n";

    // Loop principal
    while(!glfwWindowShouldClose(window)) {
        // Processa input contínuo
        processInput(window);
        
        // Limpa a tela com a cor atual
        glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Aqui você pode adicionar desenhos futuros!
        
        // Troca buffers e processa eventos
        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    glfwTerminate();
    cout << "Programa finalizado!\n";
    return 0;
}