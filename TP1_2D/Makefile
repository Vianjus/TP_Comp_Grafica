# Makefile com comandos corrigidos para Windows
CXX = g++
CXXFLAGS = -g -std=c++17 -Ilib
LDFLAGS = -Llib -lglfw3dll -lgdi32 -lopengl32
TARGET = programa.exe
SOURCES = src/main.cpp lib/glad/glad.c
DLL_SOURCE = lib/GLFW/glfw3.dll
DLL = glfw3.dll

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)
	@echo "=== Programa compilado com sucesso ==="

run: $(TARGET)
	@echo "=== Copiando DLL... ==="
	@xcopy "$(DLL_SOURCE)" "." /Y >nul
	@echo "=== Executando programa... ==="
	@.\$(TARGET)

clean:
	@if exist "$(TARGET)" del "$(TARGET)"
	@if exist "glfw3.dll" del "glfw3.dll"
	@echo "=== Arquivos limpos ==="

# Compila e executa em um comando
go: all run

.PHONY: all run clean go