#include <iostream>
#include <glad/glad.h> // this header should be put in front of all the OpenGL headers
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "loader.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "car.hpp"


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const glm::vec3 lightPos = glm::vec3(0.0f, 4000.0f, 0.0f);
float windowRatio = (float) SCR_WIDTH / (float) SCR_HEIGHT;
// camera
const glm::vec3 cameraInitialPos = glm::vec3(0.0f, 500.0f, 0.0f);
Camera camera(cameraInitialPos);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), windowRatio, 0.1f,
                                        10000.0f);

// car
controlStatus status;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

int main() {

    // initialize and configure glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // for macOS
#endif

    // create a window object
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lab02", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);


    // GLFW mouse capture
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



    // initialize glad
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    // create car
    Car car;

    // build and compile shader program
    Shader carBase("base.vs", "base.fs");


    // load data
    std::vector<float> carBaseVertices;
    loadObj("base.obj", carBaseVertices);

    // configure the car base VA0, etc
    unsigned int VBO, carBaseVAO;
    glGenVertexArrays(1, &carBaseVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, carBaseVertices.size() * sizeof(float), &carBaseVertices[0], GL_DYNAMIC_DRAW);

    glBindVertexArray(carBaseVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // render loop
    while (!glfwWindowShouldClose(window)) {

        // timing process
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;

        // input
        processInput(window);

        // render

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // car base
        carBase.use();
        carBase.setVec3("baseColor", 1.0f, 0.5f, 0.3f);
        carBase.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        carBase.setVec3("lightPos", lightPos);
        carBase.setVec3("viewPos", camera.Position);
        glm::mat4 model = glm::mat4(1.0f);
        // put the car in correct angle
        //model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));


        car.Change(status, deltaTime);

        carBase.setMat4("projection", projection);
        carBase.setMat4("view", camera.GetViewMatrix());
        carBase.setMat4("model", car.GetBaseModelTransform());

        // render the car base
        glBindVertexArray(carBaseVAO);
        glDrawArrays(GL_TRIANGLES, 0, carBaseVertices.size() / 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    windowRatio = (float) width / (float) height;
    projection = glm::perspective(glm::radians(camera.Zoom), windowRatio, 0.1f,
                                  10000.0f);
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    status.forward = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;
    status.backward = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;
    status.left = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
    status.right = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

