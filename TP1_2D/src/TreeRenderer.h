#ifndef TREERENDERER_H
#define TREERENDERER_H

#include "VTKLoader.h"
#include <vector>
#include <string>

class TreeRenderer {
public:
    TreeRenderer();
    ~TreeRenderer();
    
    bool initialize();
    void render(const std::vector<Segment>& segments);
    void setColor(float r, float g, float b) { color[0] = r; color[1] = g; color[2] = b; }
    void setLineWidth(float width) { lineWidth = width; }
    void applyTransform(const float* transformMatrix);
    
private:
    unsigned int shaderProgram;
    unsigned int VAO, VBO;
    float color[3];
    float lineWidth;
    
    std::vector<Segment> createTestTree();
    void renderTestTree(const std::vector<Segment>& segments);
    void renderSegments(const std::vector<Segment>& segments);
    
    unsigned int compileShader(const std::string& source, unsigned int type);
    unsigned int createShaderProgram(const std::string& vertexSource, const std::string& fragmentSource);
    std::string loadShaderSource(const std::string& filepath);
};

#endif