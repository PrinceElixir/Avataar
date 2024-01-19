#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>



bool isSphereSelected = false;
int selectedSphereIndex = -1;
float elapsedTime = 0.0f;
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    out vec3 FragPos;  
    out vec3 Normal;  
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    void main() {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aPos;
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    in vec3 FragPos;  
    in vec3 Normal;  
    out vec4 FragColor;

    struct Material {
        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
        float shininess;
}; 

    struct Light {
        vec3 position;
        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
    };

    uniform Material material;
    uniform Light light;

    uniform bool isSelected;
    uniform int selectedSphereIndex;

    void main() {
   
        vec3 ambient = light.ambient * material.ambient;

   
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(light.position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light.diffuse * diff * material.diffuse;

   
        vec3 viewDir = normalize(-FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = light.specular * spec * material.specular;

        vec3 result = ambient + diffuse + specular;

   
        vec3 backgroundColor = vec3(0.2, 0.2, 0.2);

        if (isSelected && int(FragPos.x * 6.0) == selectedSphereIndex % 6 && int(FragPos.y * 6.0) == selectedSphereIndex / 6) {
            result *= 2.5;
        }

        FragColor = vec4(result + backgroundColor, 1.0);
    }
)";

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        
        float x = static_cast<float>(xpos) / 800.0 * 2.0 - 1.0;
        float y = 1.0 - static_cast<float>(ypos) / 800.0 * 2.0;

        
        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < 6; ++j) {
                float sphereX = (i - 2) * 0.4f;
                float sphereY = (j - 2) * 0.4f;
                if (x >= sphereX - 0.1f && x <= sphereX + 0.1f && y >= sphereY - 0.1f && y <= sphereY + 0.1f) {
                    isSphereSelected = true;
                    selectedSphereIndex = i * 6 + j;
                    return;
                }
            }
        }

        
        isSphereSelected = false;
        selectedSphereIndex = -1;
    }
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

int main() {
    
    if (!glfwInit()) {
        return -1;
    }

    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(800, 800, "Grid of Spheres", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    
    std::vector<float> sphereVertices;
    const int stacks = 20;
    const int sectors = 20;
    const float radius = 0.1f;

    for (unsigned int i = 0; i <= stacks; ++i) {
        float stackAngle = glm::pi<float>() * static_cast<float>(i) / static_cast<float>(stacks);
        for (unsigned int j = 0; j <= sectors; ++j) {
            float sectorAngle = 2.0f * glm::pi<float>() * static_cast<float>(j) / static_cast<float>(sectors);
            float x = radius * sin(stackAngle) * cos(sectorAngle);
            float y = radius * sin(stackAngle) * sin(sectorAngle);
            float z = radius * cos(stackAngle);
            sphereVertices.push_back(x);
            sphereVertices.push_back(y);
            sphereVertices.push_back(z);
        }
    }

    
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);

    
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

   
    glUseProgram(shaderProgram);
    glUniform3f(glGetUniformLocation(shaderProgram, "material.ambient"), 0.2f, 0.2f, 0.2f);
    glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuse"), 0.8f, 0.8f, 0.8f);
    glUniform3f(glGetUniformLocation(shaderProgram, "material.specular"), 1.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), 32.0f);
    glUniform3f(glGetUniformLocation(shaderProgram, "light.position"), 1.0f, 1.0f, 2.0f);
    glUniform3f(glGetUniformLocation(shaderProgram, "light.ambient"), 0.2f, 0.2f, 0.2f);
    glUniform3f(glGetUniformLocation(shaderProgram, "light.diffuse"), 0.5f, 0.5f, 0.5f);
    glUniform3f(glGetUniformLocation(shaderProgram, "light.specular"), 1.0f, 1.0f, 1.0f);

  
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


    while (!glfwWindowShouldClose(window)) {
        elapsedTime += 0.016f; 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

        
        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < 6; ++j) {
                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3((i - 2) * 0.4f, (j - 2) * 0.4f, 0.2f * sin(elapsedTime)));

              
                if (isSphereSelected && selectedSphereIndex == i * 6 + j) {
                    float rotationAngle = elapsedTime * glm::pi<float>(); // Adjust the rotation speed
                    model = glm::rotate(model, rotationAngle, glm::vec3(0.0f, 0.0f, 1.0f));
                }

       
                if (isSphereSelected && selectedSphereIndex == i * 6 + j) {
                    glUniform3f(glGetUniformLocation(shaderProgram, "material.ambient"), 0.4f, 0.4f, 0.4f);
                    glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuse"), 1.0f, 1.0f, 1.0f);
                    glUniform3f(glGetUniformLocation(shaderProgram, "material.specular"), 1.0f, 1.0f, 1.0f);
                }
                else {
                    glUniform3f(glGetUniformLocation(shaderProgram, "material.ambient"), 0.2f, 0.2f, 0.2f);
                    glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuse"), 0.8f, 0.8f, 0.8f);
                    glUniform3f(glGetUniformLocation(shaderProgram, "material.specular"), 1.0f, 1.0f, 1.0f);
                }

                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

                glBindVertexArray(vao);
                glDrawArrays(GL_TRIANGLE_FAN, 0, sphereVertices.size() / 3);
            }
        }

        
        glUniform3f(glGetUniformLocation(shaderProgram, "material.ambient"), 0.2f, 0.2f, 0.2f);
        glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuse"), 0.8f, 0.8f, 0.8f);
        glUniform3f(glGetUniformLocation(shaderProgram, "material.specular"), 1.0f, 1.0f, 1.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();
        processInput(window);
    }


   
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}