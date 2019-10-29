//
// Created by Lyapunov Shao on 2019-10-13.
//

#ifndef LAB01_LOADER_HPP
#define LAB01_LOADER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <GLFW/glfw3.h>
#include "loader.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

glm::vec3 compute_tri_normal(
        std::vector<glm::vec3> &vertices,
        int vertex_ind[]);

bool _loadObj(const std::string obj,
              std::vector<glm::vec3> &verticesRet,
              std::vector<glm::vec3> &normalsRet
);

void loadObj(const std::string obj,
             std::vector<float> &vertices
);

#endif //LAB01_LOADER_HPP
