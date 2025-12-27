#pragma once
#include <vector>
#include <glm.hpp>
#include "RayTracingData.h"

struct RTTexture {
    int width;
    int height;
    int channels;
    std::vector<unsigned char> data;
};

class RayTracer {
public:
    RayTracer(int width, int height);
    ~RayTracer();

    void Resize(int width, int height);
    
    // 执行 CPU 光线追踪计算
    void Render(const std::vector<RTSphereData>& spheres, 
                const std::vector<RTMaterial>& materials,
                const std::vector<RTTexture>& textures, // 新增纹理数据
                const glm::vec3& cameraPos, 
                const glm::mat4& view, 
                const glm::mat4& projection,
                const float traceTimes);
    
    void SetEnvironmentTexture(const RTTexture& env);
    
    // 将计算结果绘制到屏幕上
    void DrawResult();

private:
    int width, height;
    std::vector<unsigned char> pixelBuffer; // RGB 数据 (width * height * 3)
    unsigned int textureID;
    unsigned int quadVAO = 0, quadVBO;
    unsigned int screenShaderProgram = 0;

    // 光线追踪核心函数
    glm::vec3 Trace(const glm::vec3& origin, const glm::vec3& dir, 
                   const std::vector<RTSphereData>& spheres, 
                   const std::vector<RTMaterial>& materials, 
                   const std::vector<RTTexture>& textures, // 新增
                   int depth);
    
    // 辅助函数：求交
    bool IntersectSphere(const glm::vec3& origin, const glm::vec3& dir, 
                        const RTSphereData& sphere, float& t);
    
    // 辅助函数：纹理采样
    glm::vec3 SampleTexture(const RTTexture& tex, float u, float v);
    
    glm::vec3 SampleEnvironment(const glm::vec3& dir);

    void InitGLResources();
    void SetupScreenShader();
    
    RTTexture environmentTexture;
    bool hasEnvironmentTexture = false;
    float environmentIntensity = 1.5;
};
