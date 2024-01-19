#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <QVector3D>

struct int3{
    int i;
    int j;
    int k;
};

struct mesh{
    int nbVertices;
    int nbFaces;
    std::vector<QVector3D> vertexList;
    std::vector<int3> indexList;
};

struct weight{
    int i;
    float w;
};

mesh readMesh(const std::string& fileName);
std::vector<std::vector<weight>> readWeights(const std::string& fileName, int nbVertex);