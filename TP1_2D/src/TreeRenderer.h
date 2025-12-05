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
    void setLineWidth(float width) { lineWidth = width; }
    void applyTransform(const float* transformMatrix);
    void setColorMode(bool monochrome) { useMonochrome = monochrome; }
    
private:
    unsigned int shaderProgram;
    unsigned int VAO, VBO;
    float lineWidth;
    bool useMonochrome = false;
    
    std::vector<Segment> createTestTree();
    void renderSegments(const std::vector<Segment>& segments);
    
    // NOVO: Calcula hierarquia para coloração
    std::vector<int> calculateHierarchyDepth(const std::vector<Segment>& segments);
    int findRootSegment(const std::vector<Segment>& segments);
    
    unsigned int compileShader(const std::string& source, unsigned int type);
    unsigned int createShaderProgram(const std::string& vertexSource, const std::string& fragmentSource);
    std::string loadShaderSource(const std::string& filepath);
};

#endif