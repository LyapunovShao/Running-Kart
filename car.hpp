//
// Created by Lyapunov Shao on 2019-10-29.
//

#ifndef LAB02_CAR_HPP
#define LAB02_CAR_HPP

#include <iostream>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"

// the status of user input on direction keys
struct controlStatus {
    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
};

// this class controls the movement and behavior of the car, especially model matrix
class Car {
private:
    // when the car base is first loaded, we use this mat to define its original position
    const glm::mat4 baseTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    const glm::mat4 baseRotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    // physics parameters
    const float pi = 3.14159265358;
    glm::vec4 position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    float direction = 0;
    const float velocityLimitUpper = 180;
    const float velocityLimitLower = -50;
    const float wheelThetaLimitUpper = 45.0f;
    const float wheelThetaLimitLower = -45.0f;
    const float wheelDistance = 100.0f;

    float velocity = 0;
    float wheelTheta = 0;

    float mu = 20;
    float gamma = 100;

    float a = 0.06;
    float omega = 0.1;
    // output base model
    glm::mat4 baseModelMatrix;


public:
    Car() {
        baseModelMatrix = baseRotation * baseTranslation;
        position = baseTranslation * position;
    }

    void Change(controlStatus status, float deltaTime) {
        // deal with car base
        // if both the keys on the same line are pressed, there is no effect
        if (status.forward == status.backward)
            status.forward = status.backward = false;
        if (status.left == status.right)
            status.left = status.right = false;

        if (wheelTheta > 0) {
            wheelTheta = max(0.0f, wheelTheta - deltaTime * gamma);
        } else {
            wheelTheta = min(0.0f, wheelTheta + deltaTime * gamma);
        }
        wheelTheta += status.right * omega - status.left * omega;

        if (wheelTheta > wheelThetaLimitUpper)
            wheelTheta = wheelThetaLimitUpper;
        if (wheelTheta < wheelThetaLimitLower)
            wheelTheta = wheelThetaLimitLower;

        float dphidt = 10*velocity * tan(wheelTheta * pi / 180) / wheelDistance;

        direction += dphidt * deltaTime;
        if (direction > 360.0f)
            direction -= 360.0f;
        if (direction < 0.0f)
            direction += 360.0f;

        //baseModelMatrix = glm::translate(baseModelMatrix, vec3(-position.x,-position.y, -position.z));

        baseModelMatrix = glm::rotate(baseModelMatrix,
                                      glm::radians(-deltaTime * dphidt), glm::vec3(0.0f, 0.0f, -1.0f)
        ) ;

        //baseModelMatrix = glm::translate(baseModelMatrix, vec3(position.x,position.y, position.z));

        if (velocity > 0) {
            velocity = max(0.0f, velocity - deltaTime * mu);
        } else {
            velocity = min(0.0f, velocity + deltaTime * mu);
        }
        velocity += status.forward * a - status.backward * a;

        if (velocity > velocityLimitUpper)
            velocity = velocityLimitUpper;
        if (velocity < velocityLimitLower)
            velocity = velocityLimitLower;

        glm::mat4 tranMatrix = glm::translate(glm::mat4(1.0f),
                                              glm::vec3(velocity * deltaTime * cos(direction * pi / 180),
                                                        0,
                                                        velocity * deltaTime * sin(direction * pi / 180))
        );
        baseModelMatrix = tranMatrix * baseModelMatrix;
        position = tranMatrix * position;

    }

    glm::mat4 GetBaseModelTransform() {
        return baseModelMatrix;
    }
};


#endif //LAB02_CAR_HPP
