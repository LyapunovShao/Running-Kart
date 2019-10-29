//
// Created by Lyapunov Shao on 2019-10-13.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "loader.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

glm::vec3 compute_tri_normal(
        std::vector<glm::vec3> &vertices,
        int vertex_ind[]) {
    glm::vec3 v1 = vertices[vertex_ind[0]], v2 = vertices[vertex_ind[1]], v3 = vertices[vertex_ind[2]];
    // Compute according to the definition of normals
    glm::vec3 ans = glm::vec3(
            (v2.y - v1.y) * (v3.z - v1.z) - (v2.z - v1.z) * (v3.y - v1.y),
            (v2.z - v1.z) * (v3.x - v1.x) - (v2.x - v1.x) * (v3.z - v1.z),
            (v2.x - v1.x) * (v3.y - v1.y) - (v2.y - v1.y) * (v3.x - v1.x)
    );
    float norm = sqrt(ans.x * ans.x + ans.y * ans.y + ans.z * ans.z);
    // Normalize
    ans.x = ans.x / norm;
    ans.y = ans.y / norm;
    ans.z = ans.z / norm;
    return ans;
}

void loadObj(const std::string obj,
             std::vector<float> &vertices) {
    std::vector<glm::vec3> verticesTem;
    std::vector<glm::vec3> normalsTem;
    bool flag = _loadObj(obj, verticesTem, normalsTem);
    unsigned int len = verticesTem.size();
    for (unsigned int i = 0; i < len; ++i) {
        vertices.push_back(verticesTem[i].x);
        vertices.push_back(verticesTem[i].y);
        vertices.push_back(verticesTem[i].z);
        vertices.push_back(normalsTem[i].x);
        vertices.push_back(normalsTem[i].y);
        vertices.push_back(normalsTem[i].z);
    }
}


bool _loadObj(const std::string obj,
              std::vector<glm::vec3> &verticesRet,
              std::vector<glm::vec3> &normalsRet
) {
    std::cout << "Begin loading " << obj << std::endl;
    int vertex_ind[3];
    std::ifstream in(obj);
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<int> new_ind;
    std::string line, id;
    std::vector<std::vector<glm::vec3>> normals_collection;
    while (std::getline(in, line)) {
        if (in.eof())
            break;
        std::istringstream iss(line);
        iss >> id;
        // Positions of vertices
        if (id == "v") {
            glm::vec3 tem;
            iss >> tem.x >> tem.y >> tem.z;
            vertices.push_back(tem);

        } else if (id == "f") { // Triangles
            if (normals_collection.empty()) {
                normals_collection.resize(vertices.size());
            }


            iss >> vertex_ind[0] >> vertex_ind[1] >> vertex_ind[2];
            for (int i = 0; i < 3; ++i) {
                vertex_ind[i] -= 1;
                new_ind.push_back(vertex_ind[i]);
                verticesRet.push_back(vertices[vertex_ind[i]]);
            }
            glm::vec3 triangle_normal = compute_tri_normal(vertices, vertex_ind);
            for (int i = 0; i < 3; ++i)
                normals_collection[vertex_ind[i]].push_back(triangle_normal);
        }

    }
    int vertices_num = vertices.size();
    // Get the average normal for each vertex
    for (int i = 0; i < vertices_num; ++i) {
        int n = normals_collection[i].size();
        float value[3] = {0, 0, 0};
        for (int j = 0; j < n; ++j) {
            value[0] += normals_collection[i][j].x;
            value[1] += normals_collection[i][j].y;
            value[2] += normals_collection[i][j].z;
        }
        glm::vec3 tem = glm::vec3(value[0], value[1], value[2]);
        float norm = sqrt(tem.x * tem.x + tem.y * tem.y + tem.z * tem.z);
        tem.x = tem.x / norm;
        tem.y = tem.y / norm;
        tem.z = tem.z / norm;
        normals.push_back(tem);
    }
    std::vector<glm::vec3>().swap(vertices);
    std::vector<std::vector<glm::vec3>>().swap(normals_collection);
    // Build outputs
    for (int i = 0; i < vertices_num * 6; ++i) {
        normalsRet.push_back(normals[new_ind[i]]);
    }

    std::cout << "Object loading complete" << std::endl;
    return true;
}