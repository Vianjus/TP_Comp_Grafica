#include "VTKLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>
#include <cmath>

VTKLoader::VTKLoader() {
    // Construtor vazio
}

bool VTKLoader::loadFile(const std::string& filename) {
    std::cout << "Carregando: " << filename << std::endl;
    
    segments.clear();
    points.clear();

    // Primeiro tenta carregar o arquivo real
    if (loadRealVTKFile(filename)) {
        std::cout << "Arquivo VTK carregado com " << segments.size() << " segmentos" << std::endl;
        return true;
    }
    
    // Se não conseguir, gera árvore procedural
    std::cout << "Gerando árvore arterial procedural..." << std::endl;
    generateProceduralTree();
    std::cout << "Árvore procedural criada com " << segments.size() << " segmentos" << std::endl;
    
    return true;
}

bool VTKLoader::loadRealVTKFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    std::vector<Point2D> tempPoints;
    std::vector<std::pair<int, int>> connections;
    std::vector<float> radii;

    bool readingPoints = false;
    bool readingLines = false;
    bool readingRadii = false;
    int pointsCount = 0;
    int linesCount = 0;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        
        if (line.empty()) continue;
        
        // Verifica se é um cabeçalho/comando
        if (line.find("POINTS") == 0) {
            iss >> token >> pointsCount;
            readingPoints = true;
            readingLines = false;
            readingRadii = false;
            tempPoints.reserve(pointsCount);
            continue;
        }
        else if (line.find("LINES") == 0) {
            iss >> token >> linesCount;
            readingPoints = false;
            readingLines = true;
            readingRadii = false;
            connections.reserve(linesCount);
            continue;
        }
        else if (line.find("RADIUS") == 0 || line.find("RADII") == 0) {
            readingPoints = false;
            readingLines = false;
            readingRadii = true;
            radii.reserve(pointsCount);
            continue;
        }
        else if (line.find("SCALARS") == 0 || line.find("LOOKUP_TABLE") == 0) {
            // Ignora seções de cor/tabela
            continue;
        }

        // Processa dados baseado no que está lendo
        if (readingPoints && pointsCount > 0) {
            float x, y, z;
            if (iss >> x >> y >> z) {
                tempPoints.push_back(Point2D(x, y));
                pointsCount--;
            }
        }
        else if (readingLines && linesCount > 0) {
            int numPoints, p1, p2;
            if (iss >> numPoints) {
                if (numPoints == 2) {
                    if (iss >> p1 >> p2) {
                        connections.push_back(std::make_pair(p1, p2));
                        linesCount--;
                    }
                }
            }
        }
        else if (readingRadii) {
            float radius;
            while (iss >> radius) {
                radii.push_back(radius);
            }
        }
    }

    file.close();

    // Constrói os segmentos a partir dos pontos e conexões
    if (!tempPoints.empty() && !connections.empty()) {
        points = tempPoints;
        
        for (const auto& conn : connections) {
            if (conn.first < points.size() && conn.second < points.size()) {
                Segment seg;
                seg.start = points[conn.first];
                seg.end = points[conn.second];
                
                // Usa raios padrão ou dos arrays se disponíveis
                if (conn.first < radii.size() && conn.second < radii.size()) {
                    seg.startRadius = radii[conn.first];
                    seg.endRadius = radii[conn.second];
                } else {
                    seg.startRadius = 0.05f;
                    seg.endRadius = 0.02f;
                }
                
                seg.parentIndex = -1; // Seria necessário calcular hierarquia
                segments.push_back(seg);
            }
        }
        
        return !segments.empty();
    }
    
    return false;
}

void VTKLoader::generateProceduralTree() {
    segments.clear();
    points.clear();
    
    std::mt19937 rng(42); // Seed fixa para reproducibilidade
    std::uniform_real_distribution<float> dist(-0.05f, 0.05f);
    
    Point2D root(0.0f, -0.8f);
    points.push_back(root);
    
    // Função recursiva para gerar ramos
    auto generateBranch = [&](Point2D start, Point2D direction, float length, 
                             float startRadius, int depth, int parentPointIdx, 
                             auto&& self) -> int {
        if (depth <= 0 || length < 0.01f) return -1;
        
        // Calcula ponto final com alguma variação aleatória
        Point2D end;
        end.x = start.x + direction.x * length + dist(rng);
        end.y = start.y + direction.y * length + dist(rng);
        
        int endPointIdx = points.size();
        points.push_back(end);
        
        // Cria segmento
        Segment seg;
        seg.start = start;
        seg.end = end;
        seg.startRadius = startRadius;
        seg.endRadius = startRadius * 0.7f;
        seg.parentIndex = parentPointIdx;
        segments.push_back(seg);
        
        // Gera sub-ramos
        if (depth > 1) {
            int numBranches = (depth > 3) ? 2 : 1;
            
            for (int i = 0; i < numBranches; i++) {
                float angle = (i == 0) ? 0.5f : -0.5f; // Ângulos opostos
                
                // Rotaciona a direção
                Point2D newDir;
                newDir.x = direction.x * cos(angle) - direction.y * sin(angle);
                newDir.y = direction.x * sin(angle) + direction.y * cos(angle);
                
                // Normaliza
                float mag = sqrt(newDir.x * newDir.x + newDir.y * newDir.y);
                if (mag > 0) {
                    newDir.x /= mag;
                    newDir.y /= mag;
                }
                
                self(end, newDir, length * 0.6f, startRadius * 0.7f, 
                     depth - 1, endPointIdx, self);
            }
        }
        
        return endPointIdx;
    };
    
    // Gera tronco principal
    generateBranch(root, Point2D(0.0f, 1.0f), 0.6f, 0.08f, 6, 0, generateBranch);
    
    // Ramos laterais da base
    generateBranch(Point2D(0.0f, -0.6f), Point2D(0.8f, 0.4f), 0.3f, 0.04f, 4, 0, generateBranch);
    generateBranch(Point2D(0.0f, -0.6f), Point2D(-0.8f, 0.4f), 0.3f, 0.04f, 4, 0, generateBranch);
    
    // Ramos médios
    generateBranch(Point2D(0.0f, -0.3f), Point2D(0.9f, 0.2f), 0.25f, 0.03f, 3, 0, generateBranch);
    generateBranch(Point2D(0.0f, -0.3f), Point2D(-0.9f, 0.2f), 0.25f, 0.03f, 3, 0, generateBranch);
}

void VTKLoader::clear() {
    segments.clear();
    points.clear();
}