// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef GEOMETRYENGINE_H
#define GEOMETRYENGINE_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <deque>
#include <cmath>

#include <QVector2D>
#include <QVector3D>

struct BVHTree {
    std::string name;
    QVector3D offset;
    QMatrix4x4 rotationMatrix;
    std::vector<std::string> channels;
    std::vector<std::vector<float>>channelsValues;
    std::vector<BVHTree*> joints;
    BVHTree* parent = NULL;
    int nbNode = 1;
    int nbLink = 0;
    int vertexIndex;
};

int readNode(const std::vector<std::string>& tokens, int i, BVHTree* node);
int readAnimNode(const std::vector<std::string>& tokens, int i, BVHTree* node, float time);
std::vector<BVHTree*> readBVH(const std::string& file);

class GeometryEngine : protected QOpenGLFunctions
{
public:
    GeometryEngine();
    virtual ~GeometryEngine();

    void updateAnimation(float elapseTime);

    void drawCubeGeometry(QOpenGLShaderProgram *program);
    void drawRepereGeometry(QOpenGLShaderProgram *program);
    void drawBVHGeometry(QOpenGLShaderProgram *program);

private:
    BVHTree loadBVH(std::string filename);
    void printBVHTree(const BVHTree& node, const std::string& dependency = "", const std::string& first = "", const std::string& next = "");
    void initCubeGeometry();
    void initRepereGeometry();
    void initBVHGeometry(std::string filename);

    int nbVertex;
    int nbIndex;

    std::vector<BVHTree*> rootList;

    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
};

#endif // GEOMETRYENGINE_H
