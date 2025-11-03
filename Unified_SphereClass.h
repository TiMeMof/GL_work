#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <vector>
#include "ShaderClass.h"
#include "LoadTexture.h"

class Unified_SphereClass
{
private:
    Shader shader;
    unsigned int VAO, VBO, EBO;
    unsigned textureId;
    int slices; // 经度分割数（水平方向）
    int stacks; // 纬度分割数（垂直方向）
    float radius; // 球体半径
    int indexCount; // 索引总数
    char const * texture_path;

public:
    Unified_SphereClass(const char* vertexPath, const char* fragmentPath, 
                        char const * texture_path = "material/grassblock.png",
                       int slices = 32, int stacks = 32, float radius = 1.0f) 
        : shader(vertexPath, fragmentPath), slices(slices), stacks(stacks), radius(radius), texture_path(texture_path)
    {
        // 生成球体顶点数据（使用参数方程）
        std::vector<float> vertices;
        
        for (int i = 0; i <= stacks; i++) {
            float phi = glm::pi<float>() * i / stacks; // 天顶角 [0, π]
            float cosPhi = cos(phi);
            float sinPhi = sin(phi);
            
            for (int j = 0; j <= slices; j++) {
                float theta = 2.0f * glm::pi<float>() * j / slices; // 方位角 [0, 2π]
                float cosTheta = cos(theta);
                float sinTheta = sin(theta);
                
                // 球体坐标计算
                float x = cosTheta * sinPhi;
                float y = cosPhi;
                float z = sinTheta * sinPhi;
                
                // 纹理坐标（u,v）
                float u = (float)j / slices;
                float v = (float)i / stacks;
                
                // 添加顶点数据
                vertices.push_back(radius * x);
                vertices.push_back(radius * y);
                vertices.push_back(radius * z);
                vertices.push_back(u);
                vertices.push_back(v);
            }
        }
        
        // 生成索引数据（用于三角形绘制）
        std::vector<unsigned int> indices;
        for (int i = 0; i < stacks; i++) {
            for (int j = 0; j < slices; j++) {
                int first = (i * (slices + 1)) + j;
                int second = first + slices + 1;
                
                // 两个三角形组成一个四边形
                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);
                
                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }
        
        indexCount = indices.size();
        
        // 配置OpenGL对象
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        
        glBindVertexArray(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        
        // 位置属性
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // 纹理坐标属性
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        textureId = loadTexture(texture_path); // 使用相同的贴图
    }
    
    void Draw()
    {
        shader.use();
        shader.setFloat("rgb_b", sin(glfwGetTime()));
        glBindTexture(GL_TEXTURE_2D, textureId);
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(.0f, .0f, 1.0f));
        
        glm::mat4 view = camera.GetViewMatrix(); // 使用相同的摄像机设置
        
        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);
        
        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

        // // 绘制第二个球体（与原立方体类保持一致）
        // model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
        // model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(.0f, 1.0f, .0f)); 
        // shader.setMat4("model", model);
        // glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }

    void Draw(glm::mat4 model)
    {
        shader.use();
        shader.setFloat("rgb_b", sin(glfwGetTime()));
        glBindTexture(GL_TEXTURE_2D, textureId);
        
        glm::mat4 view = camera.GetViewMatrix(); // 使用相同的摄像机设置
        
        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }

    glm::mat4 GetModelMatrix()
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(.0f, .0f, 1.0f));
        return model;
    }
    
    ~Unified_SphereClass()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO); // 正确释放EBO资源
    }
};