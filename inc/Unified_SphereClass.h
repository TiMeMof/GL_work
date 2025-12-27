#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <vector>
#include "ShaderClass.h"
#include "LoadTexture.h"
#include "RayTracingData.h"

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
    glm::mat4 model = glm::mat4(1.0f); // 模型pose
    float alpha = 1.0f; // 透明度
    RTMaterial rtMaterial; // 新增：光追材质属性
    
    // CPU 纹理数据
    std::vector<unsigned char> cpuTextureData;
    int texWidth = 0, texHeight = 0, texChannels = 0;

public:
    Unified_SphereClass(const char* vertexPath, const char* fragmentPath, 
                        char const * texture_path = "material/grassblock.png",
                       int slices = 32, int stacks = 32, float radius = 1.0f, float alpha_in = 1.0f) 
        : shader(vertexPath, fragmentPath), slices(slices), stacks(stacks), radius(radius), texture_path(texture_path), alpha(alpha_in)
    {
        // ... (顶点生成代码保持不变) ...
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

        // 加载纹理数据
        unsigned char *data = stbi_load(texture_path, &texWidth, &texHeight, &texChannels, 0);
        if (data) {
            cpuTextureData.assign(data, data + texWidth * texHeight * texChannels);
            stbi_image_free(data);
        } else {
            std::cout << "Failed to load texture for CPU Ray Tracing: " << texture_path << std::endl;
        }
    }
    
    // 获取纹理数据
    void GetTextureData(int& w, int& h, int& c, std::vector<unsigned char>& data) {
        w = texWidth;
        h = texHeight;
        c = texChannels;
        data = cpuTextureData;
    }
    
    //绘画原点处的球体
    void Draw()
    {
        shader.use();
        shader.setFloat("rgb_b", sin(glfwGetTime()));
        shader.setFloat("alpha", alpha); // 设置透明度uniform
        glBindTexture(GL_TEXTURE_2D, textureId);
        
        glm::mat4 model = glm::mat4(1.0f);
        // model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(.0f, .0f, 1.0f));
        
        glm::mat4 view = camera.GetViewMatrix(); // 使用相同的摄像机设置
        
        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);
        
        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        // 对于半透明物体：在绘制时禁用深度写入，以便正确混合
        bool transparent = (alpha < 1.0f - 1e-6f);
        if (transparent) {
            glDepthMask(GL_FALSE);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        if (transparent) {
            glDepthMask(GL_TRUE);
        }

    }

    void Draw(glm::mat4 model_in, float fov=45.0f)
    {
        model = model_in;
        shader.use();
        shader.setFloat("rgb_b", sin(glfwGetTime()));
        shader.setFloat("alpha", alpha); // 设置透明度uniform
        glBindTexture(GL_TEXTURE_2D, textureId);
        
        glm::mat4 view = camera.GetViewMatrix(); // 使用相同的摄像机设置
        
        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(fov), 1920.0f / 1080.0f, 0.1f, 100.0f);

        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        bool transparent = (alpha < 1.0f - 1e-6f);
        if (transparent) {
            glDepthMask(GL_FALSE);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        if (transparent) {
            glDepthMask(GL_TRUE);
        }
    }

    glm::mat4 GetModelMatrix()
    {
        return model;
    }
    
    // 设置光追材质
    // color: 基础颜色
    // emission: 自发光颜色
    // type: 材质类型（漫反射、镜面、折射等）
    // roughness: 粗糙度（仅对某些材质类型有效）
    // ior: 折射率（仅对折射材质有效）
    void SetRTMaterial(glm::vec3 color, glm::vec3 emission, int type, float roughness = 0.0f, float ior = 1.45f) {
        rtMaterial.color = color;
        rtMaterial.emission = emission;
        rtMaterial.type = type;
        rtMaterial.roughness = roughness;
        rtMaterial.ior = ior;
        rtMaterial.padding1 = 0.0f;
        rtMaterial.padding2 = 0.0f;
        rtMaterial.padding3 = 0.0f;
    }

    // 获取材质数据
    RTMaterial GetRTMaterial() {
        return rtMaterial;
    }
    // 手动设置模型矩阵（用于更新位置而不绘制）
    void SetModelMatrix(glm::mat4 model_in) {
        model = model_in;
    }
    // 获取用于上传 GPU 的数据
    RTSphereData GetRTData() {
        // 从 model 矩阵提取世界坐标位置
        glm::vec3 worldPos = glm::vec3(model[3]); 
        // 假设统一缩放，从 model 提取缩放后的半径
        float scale = glm::length(glm::vec3(model[0])); 
        RTSphereData data;
        data.center = worldPos;
        data.radius = radius * scale;
        data.materialIndex = 0; // 索引需由管理器分配
        data.padding[0] = 0.0f;
        data.padding[1] = 0.0f;
        data.padding[2] = 0.0f;
        return data;
    }
    ~Unified_SphereClass()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO); // 正确释放EBO资源
    }
};