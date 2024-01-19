#include "../header/mesh.h"

mesh readMesh(const std::string& fileName){
    std::ifstream inputFile(fileName);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Error opening file: " + fileName);
    }

    mesh myMesh;

    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    std::string content = buffer.str();

    std::istringstream tokens(content);

    std::string currentToken;

    tokens >> currentToken;

    if (currentToken.compare("OFF") != 0) {
        std::cerr << "Error in file content with header\n";
    }

    tokens >> currentToken;
    int nbVertex = std::stoi(currentToken);
    myMesh.nbVertices = nbVertex;

    tokens >> currentToken;
    int nbFace = std::stoi(currentToken);
    myMesh.nbFaces = nbFace;

    tokens >> currentToken;
    // int nbEdge = std::stoi(currentToken); // Not used

    for (int vertexIndex = 0; vertexIndex < nbVertex; vertexIndex++){
        
        tokens >> currentToken;
        float x = std::stof(currentToken)/100.0;

        tokens >> currentToken;
        float y = std::stof(currentToken)/100.0;

        tokens >> currentToken;
        float z = std::stof(currentToken)/100.0;

        myMesh.vertexList.push_back(QVector3D(x, y, z));
    }

    for (int faceIndex = 0; faceIndex < nbFace; faceIndex++){

            tokens >> currentToken;

            if (currentToken.compare("3") != 0) {
                std::cerr << "Error in file content with a face size (not a triangle)\n";
            }

            tokens >> currentToken;
            int i = std::stoi(currentToken);

            tokens >> currentToken;
            int j = std::stoi(currentToken);

            tokens >> currentToken;
            int k = std::stoi(currentToken);

            myMesh.indexList.push_back(int3{i, j, k});
    }

    return myMesh;
}

std::vector<std::vector<weight>> readWeights(const std::string& fileName, int nbVertex){
    std::ifstream inputFile(fileName);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Error opening file: " + fileName);
    }

    std::vector<std::vector<weight>> weightList;

    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    std::string content = buffer.str();

    std::istringstream tokens(content);

    std::string currentToken;

    int nbColumn = 32;
    for (int i = 0; i < nbColumn; i++){
        tokens >> currentToken;
    }

    for (int vertexIndex = 0; vertexIndex < nbVertex; vertexIndex++){
        
        tokens >> currentToken; //Index of the vertex in the first column

        std::vector<weight> vertexWeights;

        for (int jointIndex = 0; jointIndex < nbColumn - 1; jointIndex++){
            
            tokens >> currentToken;

            float w = std::stof(currentToken);

            if (w != 0){
                vertexWeights.push_back(weight{jointIndex, w});
            }
        }
        // std::cout << "Nombre de poid non nul: " << vertexWeights.size() << std::endl;

        weightList.push_back(vertexWeights);

    }


    return weightList;
}