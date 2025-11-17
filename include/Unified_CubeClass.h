#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "ShaderClass.h"
#include "LoadTexture.h"

class Unified_CubeClass // 对立方体绘图对象的集成化类 (坐标、内存分配、着色器、绘制变换等所有对象相关)
{
private:
    Shader shader;
    unsigned int VAO, VBO;
    unsigned textureId;
public:
    Unified_CubeClass(const char* vertexPath, const char* fragmentPath) : shader(vertexPath, fragmentPath) // 构造函数封装所有立方体 "图元配置" 代码，将在对象创建时自动执行
    {
        float vertices[] =
        {
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // 生成了一个VBO（顶点缓冲对象），并将顶点数据传输到GPU中
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0); // VAO点坐标生效

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1); // VAO纹理生效
        

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0); // 下班

        unsigned textureId = loadTexture("material/grassblock.png"); // 纹理也可以考虑传参加载
    }

    void Draw() // 增加绘图函数封装所有立方体 "图元绘制" 代码，集成在在While循环中
    {
        shader.use();
        shader.setFloat("rgb_b", sin(glfwGetTime()));
        glBindTexture(GL_TEXTURE_2D, textureId);
        
        glm::mat4 model = glm::mat4(1.0f); // 模型矩阵: 变换矩阵初始化，然后附加：缩放、旋转、位移。但变换顺序与阅读顺序相反
        // 自动旋转，glfwGetTime()获得启动后的时间，秒为单位,glm::vec3(xyz)是旋转轴
        // x左，y上，z前
        model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(.0f, .0f, 1.0f)); 

        // glm::mat4 view  = glm::mat4(1.0f);
        // view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 view = camera.GetViewMatrix();

        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        
        glBindVertexArray(VAO); // 上班
        //glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
        model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(.0f, 1.0f, .0f)); 
        shader.setMat4("model", model); // 重新设置模型再绘制
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    ~Unified_CubeClass()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
};