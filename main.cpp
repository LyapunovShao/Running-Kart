#include <iostream>
#include <glad/glad.h> // this header should be put in front of all the OpenGL headers
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include "loader.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "car.hpp"
#include <vector>


// settings
int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;
const glm::vec3 lightPos = glm::vec3(-100.0f, 3000.0f, -100.0f);
float windowRatio = (float) SCR_WIDTH / (float) SCR_HEIGHT;
// camera
const glm::vec3 cameraInitialPos = glm::vec3(0.0f, 500.0f, 0.0f);
Camera camera(cameraInitialPos);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;


// car
controlStatus status;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void processInput(GLFWwindow *window);

void renderScene(const Shader &shader);

unsigned int loadCubemap(std::vector<std::string> faces);

unsigned int roadVBO, roadVAO;
unsigned int carBaseVBO, carBaseVAO;
unsigned int carWheelVBO, carWheelVAO;
unsigned int carSteeringVBO, carSteeringVAO;

std::vector<float> carBaseVertices;
std::vector<float> carWheelVertices;
std::vector<float> roadVertices;
std::vector<float> carSteeringVertices;

Car car;
mat4 projection = glm::perspective(glm::radians(camera.Zoom), windowRatio, 0.1f, 10000.0f);
bool showShadow = false;
bool changeEnable = true;
const int intervalLimit = 300;
int changeInterval = intervalLimit;

bool followMode = false;

float skyboxVertices[] = {
        // positions
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f
};

int main() {

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // create a window object
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lab02", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCR_WIDTH, &SCR_HEIGHT);

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


    // build and compile shader program
    Shader drawShader("draw.vs", "draw.fs");
    Shader depthShader("simpleDepth.vs", "simpleDepth.fs");
    Shader skyboxShader("skyBox.vs", "skyBox.fs");
    // load data

    loadObj("base.obj", carBaseVertices);
    loadObj("wheel.obj", carWheelVertices);
    loadObj("road.obj", roadVertices);
    loadObj("steering.obj", carSteeringVertices);

    // configure the car base VA0, etc

    glGenVertexArrays(1, &carBaseVAO);
    glGenBuffers(1, &carBaseVBO);
    glBindVertexArray(carBaseVAO);
    glBindBuffer(GL_ARRAY_BUFFER, carBaseVBO);
    glBufferData(GL_ARRAY_BUFFER, carBaseVertices.size() * sizeof(float), &carBaseVertices[0], GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));
    glBindVertexArray(0);
    // wheel data
    glGenVertexArrays(1, &carWheelVAO);
    glGenBuffers(1, &carWheelVBO);
    glBindVertexArray(carWheelVAO);
    glBindBuffer(GL_ARRAY_BUFFER, carWheelVBO);
    glBufferData(GL_ARRAY_BUFFER, carWheelVertices.size() * sizeof(float), &carWheelVertices[0], GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));
    glBindVertexArray(0);
    //road data

    glGenVertexArrays(1, &roadVAO);
    glGenBuffers(1, &roadVBO);
    glBindVertexArray(roadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, roadVBO);
    glBufferData(GL_ARRAY_BUFFER, roadVertices.size() * sizeof(float), &roadVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));
    glBindVertexArray(0);
    // steering wheel data

    glGenVertexArrays(1, &carSteeringVAO);
    glGenBuffers(1, &carSteeringVBO);
    glBindVertexArray(carSteeringVAO);
    glBindBuffer(GL_ARRAY_BUFFER, carSteeringVBO);
    glBufferData(GL_ARRAY_BUFFER, carSteeringVertices.size() * sizeof(float), &carSteeringVertices[0], GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));
    glBindVertexArray(0);
    // configure depth map
    const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return false;

    // skybox
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);

    // load texture
    std::vector<std::string> faces{
            "/Users/lyapunov/Documents/Lab02/mp_sandcastle/sandcastle_rt.jpg",
            "/Users/lyapunov/Documents/Lab02/mp_sandcastle/sandcastle_lf.jpg",
            "/Users/lyapunov/Documents/Lab02/mp_sandcastle/sandcastle_up.jpg",
            "/Users/lyapunov/Documents/Lab02/mp_sandcastle/sandcastle_dn.jpg",
            "/Users/lyapunov/Documents/Lab02/mp_sandcastle/sandcastle_ft.jpg",
            "/Users/lyapunov/Documents/Lab02/mp_sandcastle/sandcastle_bk.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    drawShader.use();
    drawShader.setInt("shadowMap", 0);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // timing process
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;
        if (!changeEnable) {
            --changeInterval;
            if (changeInterval <= 0) {
                changeInterval = intervalLimit;
                changeEnable = true;
            }
        }

        // input
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // 1. render depth of scene to texture
        mat4 lightProjection, lightView;
        mat4 lightSpaceMatrix;
        lightProjection = ortho(-1000.0f, 1000.0f, -1000.0f, 1000.0f, 1.0f, 5000.0f);
        lightView = lookAt(lightPos, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
        lightSpaceMatrix = lightProjection * lightView;
        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        if (showShadow)
            renderScene(depthShader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // reset view port
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2. render scene as normal using the generated depth/shadow map
        // same process for road
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        car.Change(status, deltaTime);

        drawShader.use();
        drawShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        drawShader.setMat4("projection", projection);
        drawShader.setMat4("view", camera.GetViewMatrix(followMode, car.GetCameraPosition(),
                                                        car.GetCameraDirection()));
        drawShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        drawShader.setVec3("viewPos", camera.Position);
        drawShader.setVec3("lightPos", lightPos);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        renderScene(drawShader);

        //draw skybox at last
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        mat4 view = mat4(mat3(camera.GetViewMatrix(followMode, car.GetCameraPosition(),
                                                   car.GetCameraDirection())));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

void renderScene(const Shader &shader) {
    // road
    shader.setVec3("objColor", 0.5f, 0.5f, 0.5f);
    shader.setMat4("model",
                   translate(
                           rotate(scale(mat4(1.0f), vec3(2.0f, 2.0f, 2.0f)), radians(90.0f), vec3(1.0f, 0.0f, 0.0f)),
                           vec3(-120.0f, -100.0f, 25.0f)));
    glBindVertexArray(roadVAO);
    glDrawArrays(GL_TRIANGLES, 0, roadVertices.size() / 6);
    glBindVertexArray(0);
    // car base
    shader.setVec3("objColor", 1.0f, 0.5f, 0.3f);
    shader.setMat4("model",
                   glm::rotate(car.GetBaseModelTransform(), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
    glBindVertexArray(carBaseVAO);
    glDrawArrays(GL_TRIANGLES, 0, carBaseVertices.size() / 6);
    glBindVertexArray(0);
    // steering wheel
    shader.setVec3("objColor", 0.2, 0.2f, 0.25f);
    shader.setMat4("model", car.GetSteeringModelTransform());
    glBindVertexArray(carSteeringVAO);
    glDrawArrays(GL_TRIANGLES, 0, carSteeringVertices.size() / 6);
    glBindVertexArray(0);
    // 4 wheels
    shader.setVec3("objColor", 0.2f, 0.2f, 0.25f);
    for (int i = 0; i < 4; ++i) {
        shader.setMat4("model", glm::rotate(car.GetWheelModelTransform(i), glm::radians(90.0f),
                                            glm::vec3(1.0f, 0.0f, 0.0f)));
        glBindVertexArray(carWheelVAO);
        glDrawArrays(GL_TRIANGLES, 0, carWheelVertices.size() / 6);
        glBindVertexArray(0);
    }
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

    if (changeEnable && glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        changeEnable = false;
        followMode = !followMode;
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
        showShadow = true;
    else showShadow = false;

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

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front)
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                         data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}