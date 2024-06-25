#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include "glad/glad.h"
#include <corecrt_math_defines.h>

// Camera parameters
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float cameraSpeed = 0.001f; // Adjust accordingly
// zoom
float zoomPerspective = 20.0f;

// Vertex shader source code
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

out vec3 ourColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    ourColor = aColor;
}
)";

// Fragment shader source code
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 ourColor;

void main()
{
    FragColor = vec4(ourColor, 1.0f);
}
)";

// Process input
void registerEventHandlers(GLFWwindow* window) {
    // Q means quit
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

   // if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
   //     cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

   // if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
   //     cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    // up arrow means zoom in
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        zoomPerspective -= 0.01f;
        if (zoomPerspective <= 1.0f)
            zoomPerspective = 1.0f;
    }

    // down means zoom out
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        zoomPerspective += 0.01f;
        if (zoomPerspective >= 70.0f)
            zoomPerspective = 70.0f;
    }
}

// Function to add a triangle to the vertices vector
void addTriangle(std::vector<float>& vertices, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 color) {
    vertices.push_back(v1.x); vertices.push_back(v1.y); vertices.push_back(v1.z);
    vertices.push_back(color.x); vertices.push_back(color.y); vertices.push_back(color.z);
    vertices.push_back(v2.x); vertices.push_back(v2.y); vertices.push_back(v2.z);
    vertices.push_back(color.x); vertices.push_back(color.y); vertices.push_back(color.z);
    vertices.push_back(v3.x); vertices.push_back(v3.y); vertices.push_back(v3.z);
    vertices.push_back(color.x); vertices.push_back(color.y); vertices.push_back(color.z);
}

// Recursive function to generate Sierpinski Tetrahedron vertices
void sierpinski(std::vector<float>& vertices, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4, int depth, glm::vec3 color) {
    if (depth == 0) {
        addTriangle(vertices, v1, v2, v3, color);
        addTriangle(vertices, v1, v3, v4, color);
        addTriangle(vertices, v1, v2, v4, color);
        addTriangle(vertices, v2, v3, v4, color);
    }
    else {
        glm::vec3 mid12 = (v1 + v2) / 2.0f;
        glm::vec3 mid13 = (v1 + v3) / 2.0f;
        glm::vec3 mid14 = (v1 + v4) / 2.0f;
        glm::vec3 mid23 = (v2 + v3) / 2.0f;
        glm::vec3 mid24 = (v2 + v4) / 2.0f;
        glm::vec3 mid34 = (v3 + v4) / 2.0f;

        sierpinski(vertices, v1, mid12, mid13, mid14, depth - 1, color);
        sierpinski(vertices, mid12, v2, mid23, mid24, depth - 1, color);
        sierpinski(vertices, mid13, mid23, v3, mid34, depth - 1, color);
        sierpinski(vertices, mid14, mid24, mid34, v4, depth - 1, color);
    }
}

// Function to generate torus vertices
std::vector<float> generateTorusVertices(float innerRadius, float outerRadius, int sides, int rings) {
    std::vector<float> vertices;

    float ringFactor = 2.0f * M_PI / rings;
    float sideFactor = 2.0f * M_PI / sides;
    float r1 = outerRadius;
    float r2 = innerRadius;

    for (int i = 0; i <= rings; i++) {
        float theta = i * ringFactor;
        float cosTheta = cos(theta);
        float sinTheta = sin(theta);

        for (int j = 0; j <= sides; j++) {
            float phi = j * sideFactor;
            float cosPhi = cos(phi);
            float sinPhi = sin(phi);
            float x = (r1 + r2 * cosPhi) * cosTheta;
            float y = (r1 + r2 * cosPhi) * sinTheta;
            float z = r2 * sinPhi;

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(1.0f); // Color R
            vertices.push_back(1.0f); // Color G
            vertices.push_back(1.0f); // Color B
        }
    }

    return vertices;
}

// Function to generate torus indices
std::vector<unsigned int> generateTorusIndices(int sides, int rings) {
    std::vector<unsigned int> indices;

    for (int i = 0; i < rings; i++) {
        for (int j = 0; j < sides; j++) {
            int nextI = (i + 1) % rings;
            int nextJ = (j + 1) % sides;

            indices.push_back(i * (sides + 1) + j);
            indices.push_back(nextI * (sides + 1) + j);
            indices.push_back(nextI * (sides + 1) + nextJ);

            indices.push_back(i * (sides + 1) + j);
            indices.push_back(nextI * (sides + 1) + nextJ);
            indices.push_back(i * (sides + 1) + nextJ);
        }
    }

    return indices;
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Sierpinski Triangle and Torus", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Shader program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Delete shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Sierpinski Tetrahedron vertices
    std::vector<float> sierpinskiVertices;
    sierpinski(sierpinskiVertices, glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -0.5f, -0.5f), 3, glm::vec3(1.0f, 0.843f, 0.0f));

    // Torus vertices and indices
    std::vector<float> torusVertices = generateTorusVertices(0.15f, 0.35f, 30, 30);
    std::vector<unsigned int> torusIndices = generateTorusIndices(30, 30);

    // VAO and VBO for Sierpinski Tetrahedron
    unsigned int VAO1, VBO1;
    glGenVertexArrays(1, &VAO1);
    glGenBuffers(1, &VBO1);
    glBindVertexArray(VAO1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO1);
    glBufferData(GL_ARRAY_BUFFER, sierpinskiVertices.size() * sizeof(float), &sierpinskiVertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // VAO, VBO, and EBO for Torus
    unsigned int VAO2, VBO2, EBO2;
    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO2);
    glGenBuffers(1, &EBO2);
    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, torusVertices.size() * sizeof(float), &torusVertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, torusIndices.size() * sizeof(unsigned int), &torusIndices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // AXIS
    unsigned int axisVAO, axisVBO;

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Input
        registerEventHandlers(window);

        // Render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use shader program
        glUseProgram(shaderProgram);

        // Camera and perspective transformations
        glm::mat4 model = glm::mat4(1.0f);

        const float radius = 5.0f;
        float camX = sin(glfwGetTime()) * radius;
        float camZ = cos(glfwGetTime()) * radius;
        glm::mat4 view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0f, 0.0f, 0.0f), cameraUp);

        //glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(zoomPerspective), 800.0f / 600.0f, 0.1f, 100.0f);

        // Retrieve the matrix uniform locations
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

        // Set view and projection matrices
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Render Sierpinski Tetrahedron (static)
        glBindVertexArray(VAO1);
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, sierpinskiVertices.size() / 6);

        // Render Torus (rotating)
        glBindVertexArray(VAO2);
        model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, torusIndices.size(), GL_UNSIGNED_INT, 0);
        
        /*
        glLineWidth(2.0f);

        float axisVertices[] = {
            // X axis (red)
            0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            // Y axis (green)
            0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            // Z axis (blue)
            0.0f, 0.0f, 0.0f,
            -1.0f, -0.5f, -1.0f
        };

        static const GLfloat g_color_buffer_data[] = {
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
        };

        unsigned int colorbuffer;
        glGenBuffers(1, &colorbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

        // Create buffers for the axis vertices
        glGenVertexArrays(1, &axisVAO);
        glGenBuffers(1, &axisVBO);
        glBindVertexArray(axisVAO);
        glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(axisVertices), axisVertices, GL_STATIC_DRAW);

        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // Render AXIS
        glBindVertexArray(axisVAO);
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_LINES, 0, sizeof(axisVertices) / 6);
         */      
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // De-allocate
    glDeleteVertexArrays(1, &VAO1);
    glDeleteBuffers(1, &VBO1);
    glDeleteVertexArrays(1, &VAO2);
    glDeleteBuffers(1, &VBO2);
    glDeleteBuffers(1, &EBO2);
    glDeleteVertexArrays(1, &axisVAO);
    glDeleteBuffers(1, &axisVBO);

    // Terminate GLFW
    glfwTerminate();
    return 0;
}
