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
    const float wheelBias[4][3] = {
            {2.0f,  -10.0f, -3.0f},
            {2.0f,  -10.0f, 51.0f},
            {63.0f, -10.0f, -3.0f},
            {63.0f, -10.0f, 51.0f}
    };
    const float steeringBias[3] = {
            59.0f, 13.5f, 45.0f
    };

    // physics parameters
    const float pi = 3.14159265358;

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


public:

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

    glm::mat4 GetSteeringModelTransform() {
        float angle = direction * pi / 180;
        mat4 model = mat4(1.0f);
        model = translate(model, vec3(bX + steeringBias[0] * cos(angle) - steeringBias[2] * sin(angle), steeringBias[1],
                                      bZ + steeringBias[2] * cos(angle) + steeringBias[0] * sin(angle)));
        model = rotate(model, radians(direction), vec3(0.0, -1.0f, 0.0f));


        model = rotate(model, radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
        model = rotate(model, radians(24.0f), vec3(1.0f, 0.0f, 0.0f));
        float r = -9.8;
        model = rotate(model, radians(wheelTheta),vec3(0.0f,0.0f,1.0f));
        model = translate(model, vec3(r * (1 - sqrt(2) * sin(pi / 4 + wheelTheta * pi / 180)),
                                      r * (1 - sqrt(2) * cos(pi / 4 + wheelTheta * pi / 180)), 0.0f));
        return model;
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
