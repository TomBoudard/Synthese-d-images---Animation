// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "../header/geometryengine.h"

struct VertexData
{
    QVector3D position;
    QVector3D color;
    QVector2D texCoord;
};

//! [0]
GeometryEngine::GeometryEngine()
    : indexBuf(QOpenGLBuffer::IndexBuffer)
{
    initializeOpenGLFunctions();

    // Generate 2 VBOs
    arrayBuf.create();
    indexBuf.create();

    // Initializes cube geometry and transfers it to VBOs
    // initCubeGeometry();
    // initRepereGeometry();
    initBVHGeometry("../models/run1.bvh");
}

GeometryEngine::~GeometryEngine()
{
    arrayBuf.destroy();
    indexBuf.destroy();
}
//! [0]

std::map<std::string, int> equivalence = {
    {"Xposition", 0},
    {"Yposition", 1},
    {"Zposition", 2},
    {"Xrotation", 3},
    {"Yrotation", 4},
    {"Zrotation", 5}
};

void GeometryEngine::updateAnimation(float elapseTime) {
    VertexData vertices[nbVertex];

    float radius = 0.05;
    float scale = 1.0f/200.0f;

    QVector3D globalOffset = QVector3D(-350.0f, 0.0f, 0.0f) * scale;

    std::deque<BVHTree*> nodeQueue;
    for (auto root: rootList) {
        nodeQueue.push_back(root);
        while (!nodeQueue.empty()) {
            BVHTree* node = nodeQueue.front();
            nodeQueue.pop_front();
            int indexVertices = node->vertexIndex;

            QVector3D worldPos = QVector3D(0.0f, 0.0f, 0.0f);

            if (node->channels.empty()) {    
                worldPos = vertices[node->parent->vertexIndex].position;
                worldPos += node->parent->rotationMatrix * node->offset * scale;
            } else {
                int i = 0;
                while (i+2 < static_cast<int>(node->channelsValues.size()) && node->channelsValues[i+1][0] < elapseTime) i++;

                float p = (elapseTime - node->channelsValues[i][0]) / (node->channelsValues[i+1][0] - node->channelsValues[i][0]);

                p = std::max(0.0f, std::min(1.0f, p));

                float values[6] = {0, 0, 0, 0, 0, 0};

                bool hasNewOffset = false;

                for (int k = 0; k < static_cast<int>(node->channels.size()); k++) {
                    int equivalenceIndex = equivalence[node->channels[k]];
                    if (equivalenceIndex < 3) {
                        hasNewOffset = true;
                    }
                    float prev = node->channelsValues[ i ][k + 1];
                    float next = node->channelsValues[i+1][k + 1];
                    if (equivalenceIndex > 2  && abs(prev + 360 - next) < abs(prev - next)) {
                        prev += 360;
                    } else if (equivalenceIndex > 2  && abs(prev - (next + 360)) < abs(prev - next)) {
                        next += 360;
                    }
                    values[equivalenceIndex] = (1-p) * prev + p * next;
                }

                QVector3D nodeAnimOffset = node->offset;
                if (hasNewOffset) {
                    nodeAnimOffset = QVector3D(values[0], values[1], values[2]);
                }

                float theta = values[3];
                float phi = values[4];
                float psi = values[5];

                // theta -> z
                // phi -> y
                // psi -> x

                float c1 = cos(theta * M_PI / 180.0);
                float s1 = sin(theta * M_PI / 180.0);
                float c2 = cos(phi * M_PI / 180.0);
                float s2 = sin(phi * M_PI / 180.0);
                float c3 = cos(psi * M_PI / 180.0);
                float s3 = sin(psi * M_PI / 180.0);

                //     (a b c 0)
                // R = (d e f 0)
                //     (g h i 0)
                //     (0 0 0 1)

                float a = c2 * c3;
                float b = s1 * s2 * c3 - c1 * s3;
                float c = c1 * s2 * c3 + s1 * s3;
                float d = c2 * s3;
                float e = s1 * s2 * s3 + c1 * c3;
                float f = c1 * s2 * s3 - s1 * c3;
                float g = -s2;
                float h = s1 * c2;
                float j = c1 * c2;

                QMatrix4x4 localRotation = {a, b, c, 0, d, e, f, 0, g, h, j, 0, 0, 0, 0, 1};

                if (node->parent){

                    node->rotationMatrix = node->parent->rotationMatrix * localRotation;

                    worldPos = node->parent->rotationMatrix * nodeAnimOffset * scale;
                    worldPos += vertices[node->parent->vertexIndex].position;
                } else {
                    node->rotationMatrix = localRotation;

                    worldPos = nodeAnimOffset * scale + globalOffset;
                    // worldPos = QVector3D(0.0f, 0.0f, 0.0f);
                }
            }

            VertexData vertex0 = {worldPos + node->rotationMatrix * QVector3D(   0.0f,    0.0f,    0.0f), QVector3D(1.0f, 1.0f, 1.0f), QVector2D(0.0f, 0.0f)};
            VertexData vertex1 = {worldPos + node->rotationMatrix * QVector3D( radius,    0.0f,    0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector2D(0.0f, 0.0f)};
            VertexData vertex2 = {worldPos + node->rotationMatrix * QVector3D(-radius,    0.0f,    0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector2D(0.0f, 0.0f)};
            VertexData vertex3 = {worldPos + node->rotationMatrix * QVector3D(   0.0f,  radius,    0.0f), QVector3D(0.0f, 1.0f, 0.0f), QVector2D(0.0f, 0.0f)};
            VertexData vertex4 = {worldPos + node->rotationMatrix * QVector3D(   0.0f, -radius,    0.0f), QVector3D(0.0f, 1.0f, 0.0f), QVector2D(0.0f, 0.0f)};
            VertexData vertex5 = {worldPos + node->rotationMatrix * QVector3D(   0.0f,    0.0f,  radius), QVector3D(0.0f, 0.0f, 1.0f), QVector2D(0.0f, 0.0f)};
            VertexData vertex6 = {worldPos + node->rotationMatrix * QVector3D(   0.0f,    0.0f, -radius), QVector3D(0.0f, 0.0f, 1.0f), QVector2D(0.0f, 0.0f)};
            vertices[indexVertices] = vertex0;
            vertices[indexVertices + 1] = vertex1;
            vertices[indexVertices + 2] = vertex2;
            vertices[indexVertices + 3] = vertex3;
            vertices[indexVertices + 4] = vertex4;
            vertices[indexVertices + 5] = vertex5;
            vertices[indexVertices + 6] = vertex6;

            for (auto child: node->joints) {
                nodeQueue.push_back(child);
            }
        }
    }

    arrayBuf.bind();
    arrayBuf.write(0, vertices, nbVertex * sizeof(VertexData));
}

void GeometryEngine::initCubeGeometry()
{
    // For cube we would need only 8 vertices but we have to
    // duplicate vertex for each face because texture coordinate
    // is different.
    VertexData vertices[] = {
        // Vertex data for face 0
        {QVector3D(-1.0f, -1.0f,  1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(0.0f, 0.0f)},  // v0
        {QVector3D( 1.0f, -1.0f,  1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(0.33f, 0.0f)}, // v1
        {QVector3D(-1.0f,  1.0f,  1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(0.0f, 0.5f)},  // v2
        {QVector3D( 1.0f,  1.0f,  1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(0.33f, 0.5f)}, // v3

        // Vertex data for face 1
        {QVector3D( 1.0f, -1.0f,  1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D( 0.0f, 0.5f)}, // v4
        {QVector3D( 1.0f, -1.0f, -1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(0.33f, 0.5f)}, // v5
        {QVector3D( 1.0f,  1.0f,  1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(0.0f, 1.0f)},  // v6
        {QVector3D( 1.0f,  1.0f, -1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(0.33f, 1.0f)}, // v7

        // Vertex data for face 2
        {QVector3D( 1.0f, -1.0f, -1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(0.66f, 0.5f)}, // v8
        {QVector3D(-1.0f, -1.0f, -1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(1.0f, 0.5f)},  // v9
        {QVector3D( 1.0f,  1.0f, -1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(0.66f, 1.0f)}, // v10
        {QVector3D(-1.0f,  1.0f, -1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(1.0f, 1.0f)},  // v11

        // Vertex data for face 3
        {QVector3D(-1.0f, -1.0f, -1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(0.66f, 0.0f)}, // v12
        {QVector3D(-1.0f, -1.0f,  1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(1.0f, 0.0f)},  // v13
        {QVector3D(-1.0f,  1.0f, -1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(0.66f, 0.5f)}, // v14
        {QVector3D(-1.0f,  1.0f,  1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(1.0f, 0.5f)},  // v15

        // Vertex data for face 4
        {QVector3D(-1.0f, -1.0f, -1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(0.33f, 0.0f)}, // v16
        {QVector3D( 1.0f, -1.0f, -1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(0.66f, 0.0f)}, // v17
        {QVector3D(-1.0f, -1.0f,  1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(0.33f, 0.5f)}, // v18
        {QVector3D( 1.0f, -1.0f,  1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(0.66f, 0.5f)}, // v19

        // Vertex data for face 5
        {QVector3D(-1.0f,  1.0f,  1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(0.33f, 0.5f)}, // v20
        {QVector3D( 1.0f,  1.0f,  1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(0.66f, 0.5f)}, // v21
        {QVector3D(-1.0f,  1.0f, -1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(0.33f, 1.0f)}, // v22
        {QVector3D( 1.0f,  1.0f, -1.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector2D(0.66f, 1.0f)}  // v23
    };

    // Indices for drawing cube faces using triangle strips.
    // Triangle strips can be connected by duplicating indices
    // between the strips. If connecting strips have opposite
    // vertex order then last index of the first strip and first
    // index of the second strip needs to be duplicated. If
    // connecting strips have same vertex order then only last
    // index of the first strip needs to be duplicated.
    GLushort indices[] = {
         0,  1,  2,  3,  3,     // Face 0 - triangle strip ( v0,  v1,  v2,  v3)
         4,  4,  5,  6,  7,  7, // Face 1 - triangle strip ( v4,  v5,  v6,  v7)
         8,  8,  9, 10, 11, 11, // Face 2 - triangle strip ( v8,  v9, v10, v11)
        12, 12, 13, 14, 15, 15, // Face 3 - triangle strip (v12, v13, v14, v15)
        16, 16, 17, 18, 19, 19, // Face 4 - triangle strip (v16, v17, v18, v19)
        20, 20, 21, 22, 23      // Face 5 - triangle strip (v20, v21, v22, v23)
    };

//! [1]
    // Transfer vertex data to VBO 0
    arrayBuf.bind();
    arrayBuf.allocate(vertices, 24 * sizeof(VertexData));

    // Transfer index data to VBO 1
    indexBuf.bind();
    indexBuf.allocate(indices, 34 * sizeof(GLushort));
//! [1]
}

//! [2]
void GeometryEngine::drawCubeGeometry(QOpenGLShaderProgram *program)
{
    // Tell OpenGL which VBOs to use
    arrayBuf.bind();
    indexBuf.bind();

    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = program->attributeLocation("a_position");
    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    // Offset for texture coordinate
    offset += sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    int colorLocation = program->attributeLocation("a_color");
    program->enableAttributeArray(colorLocation);
    program->setAttributeBuffer(colorLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    // Offset for texture coordinate
    offset += sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    int texcoordLocation = program->attributeLocation("a_texcoord");
    program->enableAttributeArray(texcoordLocation);
    program->setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

    // Draw cube geometry using indices from VBO 1
    glDrawElements(GL_TRIANGLE_STRIP, 34, GL_UNSIGNED_SHORT, nullptr);
}
//! [2]

void GeometryEngine::initRepereGeometry() {

    VertexData vertices[] = {
        {QVector3D(0.0f, 0.0f, 0.0f), QVector3D(1.0f, 1.0f, 1.0f), QVector2D(0.0f, 0.0f)}, // v0
        {QVector3D(1.0f, 0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector2D(0.0f, 0.0f)}, // v1
        {QVector3D(0.0f, 1.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f), QVector2D(0.0f, 0.0f)}, // v2
        {QVector3D(0.0f, 0.0f, 1.0f), QVector3D(0.0f, 0.0f, 1.0f), QVector2D(0.0f, 0.0f)}, // v3
    };

    GLushort indices[] = {
        0, 1,
        0, 2,
        0, 3
    };

    arrayBuf.bind();
    arrayBuf.allocate(vertices, 4 * sizeof(VertexData));

    indexBuf.bind();
    indexBuf.allocate(indices, 6 * sizeof(GLushort));
}

void GeometryEngine::drawRepereGeometry(QOpenGLShaderProgram *program) {
    // Tell OpenGL which VBOs to use
    arrayBuf.bind();
    indexBuf.bind();

    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = program->attributeLocation("a_position");
    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    // Offset for texture coordinate
    offset += sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex color data
    int colorLocation = program->attributeLocation("a_color");
    program->enableAttributeArray(colorLocation);
    program->setAttributeBuffer(colorLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    // Draw lines geometry using indices from VBO 1
    glDrawElements(GL_LINES, 6, GL_UNSIGNED_SHORT, nullptr);
}

int readNode(const std::vector<std::string>& tokens, int i, BVHTree* node) {
    // if (tokens[i] != keyWord) {
    //     throw std::invalid_argument("\"" + keyWord + "\" token not found");
    // }

    node->name = tokens[i + 1];
    // std::cout << node->name << "\n";

    std::string accolade = tokens[i + 2];
    if (accolade != "{") {
        throw std::invalid_argument("\"{\" token not found");
    }

    std::string offset = tokens[i + 3];
    if (offset != "OFFSET") {
        throw std::invalid_argument("\"OFFSET\" token not found");
    }

    node->offset = QVector3D(std::stof(tokens[i + 4]), std::stof(tokens[i + 5]), std::stof(tokens[i + 6]));

    std::string channels = tokens[i + 7];
    if (channels != "CHANNELS") {
        throw std::invalid_argument("\"CHANNELS\" token not found");
    }

    int channelsLen = std::stoi(tokens[i + 8]);

    for (int j = 0; j < channelsLen; j++) {
        node->channels.push_back(tokens[i + 9 + j]);
    }

    return i + 9 + channelsLen;
}

int readAnimNode(const std::vector<std::string>& tokens, int i, BVHTree* node, float time) {
    if (node->channels.empty()) {
        return i;
    }
    int nbChannels = node->channels.size();
    std::vector<float> keyFrame;
    keyFrame.push_back(time);
    for (int j = 0; j < nbChannels; j++) {
        keyFrame.push_back(std::stof(tokens[i]));
        i++;
    }
    node->channelsValues.push_back(keyFrame);
    return i;
}

std::vector<BVHTree*> readBVH(const std::string& file) {
    std::ifstream fch(file);
    if (!fch.is_open()) {
        throw std::runtime_error("Error opening file: " + file);
    }

    std::stringstream buffer;
    buffer << fch.rdbuf();
    std::string content = buffer.str();

    std::istringstream iss(content);
    std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>{}};

    int i = 0;
    if (tokens[i] != "HIERARCHY") {
        throw std::invalid_argument("\"HIERARCHY\" token not found");
    }
    i++;

    std::string nextKeyword = tokens[i];

    std::vector<BVHTree*> rootList;
    std::vector<BVHTree*> nodeQueue;

    while (nextKeyword == "ROOT") {
        BVHTree* root = new BVHTree();
        i = readNode(tokens, i, root);
        rootList.push_back(root);
        nodeQueue.push_back(root);

        while (!nodeQueue.empty()) {
            BVHTree* node = new BVHTree();
            std::string t = tokens[i];

            if (t == "JOINT") {
                i = readNode(tokens, i, node);
                nodeQueue.back()->joints.push_back(node);
                node->parent = nodeQueue.back();
                nodeQueue.push_back(node);
            } else if (t == "End") {

                node->name = tokens[i + 1];
                t = tokens[i + 2];
                if (t != "{") {
                    throw std::invalid_argument("\"{\" token not found");
                }

                t = tokens[i + 3];
                if (t != "OFFSET") {
                    throw std::invalid_argument("\"OFFSET\" token not found");
                }

                node->offset = QVector3D(std::stof(tokens[i + 4]), std::stof(tokens[i + 5]), std::stof(tokens[i + 6]));
                
                nodeQueue.back()->joints.push_back(node);
                node->parent = nodeQueue.back();

                t = tokens[i + 7];
                if (t != "}") {
                    throw std::invalid_argument("\"}\" token not found");
                }

                i += 8;
            } else if (t == "}") {
                BVHTree* closingNode = nodeQueue.back();
                for (auto child: closingNode->joints) {
                    closingNode->nbNode += child->nbNode;
                    closingNode->nbLink += child->nbLink + 1;
                }
                nodeQueue.pop_back();
                i++;
            } else {
                throw std::invalid_argument("Missing valid token");
            }
        }

        nextKeyword = tokens[i];
    }

    if (tokens[i] != "MOTION") {
        throw std::invalid_argument("\"MOTION\" token not found");
    }

    if (tokens[i+1] != "Frames:") {
        throw std::invalid_argument("\"Frames:\" token not found");
    }

    int nbFrames = std::stoi(tokens[i+2]);

    if (tokens[i+3] != "Frame" && tokens[i+4] != "Time:") {
        throw std::invalid_argument("\"Frame Time:\" token not found");
    }

    float frameInterval = std::stof(tokens[i+5]);
    float time = 0;

    i += 6;

    for (int j = 0; j < nbFrames; j++) {
        int k = 0;
        while (k < static_cast<int>(rootList.size())) {
            BVHTree* root = rootList[k];
            nodeQueue.push_back(root);
            while (!nodeQueue.empty()) {
                BVHTree* node = nodeQueue.back();
                nodeQueue.pop_back();
                i = readAnimNode(tokens, i, node, time);
                for (int childIndex = node->joints.size()-1; childIndex >= 0; childIndex--) {
                    BVHTree* child = node->joints[childIndex];
                    nodeQueue.push_back(child);
                }
            }
            k++;
        }
        time += frameInterval;
    }
    
    if (i < static_cast<int>(tokens.size())) {
        throw std::invalid_argument("The file contains more values than expected");
    }


    return rootList;
}

void GeometryEngine::printBVHTree(const BVHTree& node, const std::string& dependency, const std::string& first, const std::string& next) {
    std::string out = dependency + first + "Node \"" + node.name + "\", NbSubTreeNodes :" + std::to_string(node.nbNode) + ", Offset: (" + std::to_string(node.offset.x()) + ", " + std::to_string(node.offset.y()) + ", " + std::to_string(node.offset.z()) + ")\n";
    std::cout << out;

    for (size_t i = 0; i < node.joints.size(); ++i) {
        const BVHTree* child = node.joints[i];
        std::string fst = (i == node.joints.size() - 1) ? " *> " : " +> ";
        std::string snd = (i == node.joints.size() - 1) ? "   " : " | ";
        printBVHTree(*child, dependency + next, fst, snd);
    }
}

void GeometryEngine::initBVHGeometry(std::string filename) {
    rootList = readBVH(filename);
    printBVHTree(*rootList[0]);

    int nbTotNode = 0;
    int nbTotLink = 0;
    for (auto root: rootList) {
        nbTotNode += root->nbNode;
        nbTotLink += root->nbLink;
    }

    nbVertex = nbTotNode * 7;
    nbIndex = nbTotNode * 6 + nbTotLink * 2;

    VertexData vertices[nbTotNode * 7];
    GLushort indices[nbTotNode * 6 + nbTotLink * 2];

    int indexVertices = 0;
    int indexIndices = 0;
    float radius = 0.05;
    float scale = 1.0f/100.0f;
    std::deque<BVHTree*> nodeQueue;
    for (auto root: rootList) {
        nodeQueue.push_back(root);
        while (!nodeQueue.empty()) {
            BVHTree* node = nodeQueue.front();
            nodeQueue.pop_front();
            node->vertexIndex = indexVertices;
            QVector3D worldPos = node->offset * scale;
            if (node->parent){
                worldPos += vertices[node->parent->vertexIndex].position;
            } else {
                worldPos = QVector3D(0.0f, 0.0f, 0.0f);
            }
            VertexData vertex0 = {worldPos + QVector3D(   0.0f,    0.0f,    0.0f), QVector3D(1.0f, 1.0f, 1.0f), QVector2D(0.0f, 0.0f)};
            VertexData vertex1 = {worldPos + QVector3D( radius,    0.0f,    0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector2D(0.0f, 0.0f)};
            VertexData vertex2 = {worldPos + QVector3D(-radius,    0.0f,    0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector2D(0.0f, 0.0f)};
            VertexData vertex3 = {worldPos + QVector3D(   0.0f,  radius,    0.0f), QVector3D(0.0f, 1.0f, 0.0f), QVector2D(0.0f, 0.0f)};
            VertexData vertex4 = {worldPos + QVector3D(   0.0f, -radius,    0.0f), QVector3D(0.0f, 1.0f, 0.0f), QVector2D(0.0f, 0.0f)};
            VertexData vertex5 = {worldPos + QVector3D(   0.0f,    0.0f,  radius), QVector3D(0.0f, 0.0f, 1.0f), QVector2D(0.0f, 0.0f)};
            VertexData vertex6 = {worldPos + QVector3D(   0.0f,    0.0f, -radius), QVector3D(0.0f, 0.0f, 1.0f), QVector2D(0.0f, 0.0f)};
            vertices[indexVertices] = vertex0;
            vertices[indexVertices + 1] = vertex1;
            vertices[indexVertices + 2] = vertex2;
            vertices[indexVertices + 3] = vertex3;
            vertices[indexVertices + 4] = vertex4;
            vertices[indexVertices + 5] = vertex5;
            vertices[indexVertices + 6] = vertex6;

            // Star pattern            
            indices[indexIndices] = indexVertices + 1;
            indices[indexIndices + 1] = indexVertices + 2;
            indices[indexIndices + 2] = indexVertices + 3;
            indices[indexIndices + 3] = indexVertices + 4;
            indices[indexIndices + 4] = indexVertices + 5;
            indices[indexIndices + 5] = indexVertices + 6;
            indexIndices += 6;

            // Link line(s)
            for (auto child: node->joints) {
                indices[indexIndices] = indexVertices;
                indices[indexIndices + 1] = indexVertices + 7 * (nodeQueue.size() + 1);
                indexIndices += 2;
                nodeQueue.push_back(child);
            }

            indexVertices += 7;
        }
    }

    arrayBuf.bind();
    arrayBuf.allocate(vertices, nbTotNode * 7 * sizeof(VertexData));

    indexBuf.bind();
    indexBuf.allocate(indices, (nbTotNode * 6 + nbTotLink * 2) * sizeof(GLushort));
}

void GeometryEngine::drawBVHGeometry(QOpenGLShaderProgram *program) {
    // Tell OpenGL which VBOs to use
    arrayBuf.bind();
    indexBuf.bind();

    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = program->attributeLocation("a_position");
    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    // Offset for texture coordinate
    offset += sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex color data
    int colorLocation = program->attributeLocation("a_color");
    program->enableAttributeArray(colorLocation);
    program->setAttributeBuffer(colorLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    // Draw lines geometry using indices from VBO 1
    glDrawElements(GL_LINES, nbIndex, GL_UNSIGNED_SHORT, nullptr);
}
