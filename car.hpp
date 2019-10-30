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
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
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
    const float baseX = 0.0f;
    const float baseY = 0.0f;
    const float baseZ = 0.0f;
    const float baseBiasX = 200.0f;
    const float baseBiasY = 0.0f;
    const float baseBiasZ = 120.0f;
    const float wheelBiasX = 10.0f;
    const float wheelBiasY = 0.0f;
    const float wheelBiasZ = 10.0f;
    glm::mat4 baseTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(baseX, baseY, baseZ));
    const glm::mat4 baseRotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    const glm::mat4 wheelTranslation[4] = {
            glm::translate(glm::mat4(1.0f), glm::vec3(baseX + 2.0f, baseY - 10.0f, baseZ - 3.0f)), //left back
            glm::translate(glm::mat4(1.0f), glm::vec3(baseX + 2.0f, baseY - 10.0f, baseZ + 51.0f)),  //right back
            glm::translate(glm::mat4(1.0f), glm::vec3(baseX + 63.0f, baseY - 10.0f, baseZ - 3.0f)),  //left front
            glm::translate(glm::mat4(1.0f), glm::vec3(baseX + 63.0f, baseY - 10.0f, baseZ + 51.0f))   //right front
    };
    const float wheelBias[4][3] = {
            {2.0f,  -10.0f, -3.0f},
            {2.0f,  -10.0f, 51.0f},
            {63.0f, -10.0f, -3.0f},
            {63.0f, -10.0f, 51.0f}
    };


    // physics parameters
    const float pi = 3.14159265358;
    // car centroid position approximation


    glm::vec4 basePosition = glm::vec4(baseBiasX, baseBiasY, baseBiasZ, 1.0f);
    glm::vec4 wheelPosition[4] = {
            glm::vec4(wheelBiasX, wheelBiasY, wheelBiasZ, 1.0f),
            glm::vec4(wheelBiasX, wheelBiasY, wheelBiasZ, 1.0f),
            glm::vec4(wheelBiasX, wheelBiasY, wheelBiasZ, 1.0f),
            glm::vec4(wheelBiasX, wheelBiasY, wheelBiasZ, 1.0f)
    };

    float bX = 0, bZ = 0;
    float direction = 0;

    const float velocityLimitUpper = 180;
    const float velocityLimitLower = -50;
    const float wheelThetaLimitUpper = 45.0f;
    const float wheelThetaLimitLower = -45.0f;
    const float wheelDistance = 100.0f;

    float s = 0;
    float velocity = 0;
    float wheelTheta = 0;
    float dphidt;

    float mu = 20;
    float gamma = 100;

    float a = 0.06;
    float omega = 0.1;

    float delta;
    glm::mat4 tranMatrix;
    // output base model
    glm::mat4 baseModelMatrix;
    glm::mat4 wheelModelMatrix[4];

    void baseChange(controlStatus &status, float deltaTime) {
        // deal with car base


        //baseModelMatrix = glm::translate(baseModelMatrix, vec3(basePosition.x, basePosition.y, basePosition.z));

        baseModelMatrix = glm::rotate(baseModelMatrix,
                                      glm::radians(-deltaTime * dphidt), glm::vec3(0.0f, 0.0f, -1.0f)
        );
        float dTheta = -delta * dphidt;
        glm::mat4 _tranMatrix = glm::translate(glm::mat4(1.0f),
                                               vec3((baseBiasX) *
                                                    (1 - cos(dTheta * pi / 180)) +
                                                    (baseBiasZ) * sin(dTheta * pi / 180),
                                                    0,
                                                    (baseBiasZ) *
                                                    (1 - cos(dTheta * pi / 180)) +
                                                    (baseBiasX) *
                                                    sin(dTheta * pi / 180)));
        baseModelMatrix = _tranMatrix * baseModelMatrix;
        basePosition = _tranMatrix * basePosition;

        //baseModelMatrix = glm::translate(baseModelMatrix, vec3(basePosition.x, basePosition.y, basePosition.z));



        tranMatrix = glm::translate(glm::mat4(1.0f),
                                    glm::vec3(velocity * deltaTime * cos(direction * pi / 180),
                                              0,
                                              velocity * deltaTime * sin(direction * pi / 180))
        );


        baseModelMatrix = tranMatrix * baseModelMatrix;
        basePosition = tranMatrix * basePosition;


    }

    void _baseChange() {
        float dTheta = delta * dphidt;
        direction += dTheta;
        if (direction > 360.0f)
            direction -= 360.0f;
        if (direction < 0.0f)
            direction += 360.0f;

        bX += velocity * delta * cos(direction * pi / 180);
        bZ += velocity * delta * sin(direction * pi / 180);
/*      // change the axis about which the car rotates
        bX += baseBiasX * (1 - cos(dTheta * pi / 180)) + baseBiasZ * sin(dTheta * pi / 180)
              + velocity * delta * cos(direction * pi / 180);
        bZ += baseBiasX * (1 - cos(dTheta * pi / 180)) + baseBiasZ * sin(dTheta * pi / 180)
              + velocity * delta * sin(direction * pi / 180);
              */
    }

    void _wheelChange() {
        s -= 1.5 * velocity * delta;
        if (s > 360.0f)
            s -= 360.0f;
        if (s < 0.0f)
            s += 360.0f;
    }

    void wheelChange(controlStatus status, int index) {

        wheelModelMatrix[index] = glm::rotate(
                wheelModelMatrix[index], glm::radians(-delta * dphidt),
                glm::vec3(0.0f, 0.0f, -1.0f)
        );

        wheelModelMatrix[index] = tranMatrix * wheelModelMatrix[index];
        wheelPosition[index] = tranMatrix * wheelPosition[index];
        float dTheta = -delta * dphidt;
        glm::mat4 _tranMatrix = glm::translate(glm::mat4(1.0f),
                                               vec3((basePosition.x - wheelPosition[index].x) *
                                                    (1 - cos(dTheta * pi / 180)) +
                                                    (basePosition.z - wheelPosition[index].z) * sin(dTheta * pi / 180),
                                                    0,
                                                    (basePosition.z - wheelPosition[index].z) *
                                                    (1 - cos(dTheta * pi / 180)) +
                                                    (basePosition.x - wheelPosition[index].x) *
                                                    sin(dTheta * pi / 180)));
        wheelModelMatrix[index] = _tranMatrix * wheelModelMatrix[index];
        wheelPosition[index] = _tranMatrix * wheelPosition[index];
    }

public:
    Car() {
        baseModelMatrix = baseRotation * baseTranslation;
        basePosition = baseTranslation * basePosition;
        for (int i = 0; i < 4; ++i) {
            wheelModelMatrix[i] = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            wheelModelMatrix[i] = wheelTranslation[i] * wheelModelMatrix[i];
            wheelPosition[i] = wheelTranslation[i] * wheelPosition[i];
        }
    }

    void Change(controlStatus status, float deltaTime) {
        delta = deltaTime;
        // if both the keys on the same line are pressed, there is no effect
        if (status.forward == status.backward)
            status.forward = status.backward = false;
        if (status.left == status.right)
            status.left = status.right = false;

        // relative angle of front wheels
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

        dphidt = 10 * velocity * tan(wheelTheta * pi / 180) / wheelDistance;



        // velocity of car base
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
        _baseChange();
        _wheelChange();

    }


    glm::mat4 GetBaseModelTransform() {

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(bX, 0, bZ));
        model = glm::rotate(model, glm::radians(direction), glm::vec3(0.0f, -1.0f, 0.0f));
        return model;
    }

    glm::mat4 GetWheelModelTransform(int index) {
        float angle = direction * pi / 180;
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model,
                               glm::vec3(bX + wheelBias[index][0] * cos(angle) - wheelBias[index][2] * sin(angle),
                                         wheelBias[index][1],
                                         bZ + wheelBias[index][2] * cos(angle) + wheelBias[index][0] * sin(angle)));
        model = glm::rotate(model, glm::radians(direction + (index > 1 ? wheelTheta / 9 : 0)),
                            glm::vec3(0.0f, -1.0f, 0.0f));

        model = glm::rotate(model, glm::radians(s), glm::vec3(0.0f, 0.0f, 1.0f));
        float r = -12;
        model = glm::translate(model, glm::vec3(r * (1 - sqrt(2) * sin(pi / 4 + s * pi / 180)),
                                                r * (1 - sqrt(2) * cos(pi / 4 + s * pi / 180)), 0.0f));
        return model;
    }
};


#endif //LAB02_CAR_HPP
