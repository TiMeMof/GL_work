#include "RayTracer.h"
#include <glad.h>
#include <glfw3.h>
#include <iostream>
#include <algorithm>
#include <limits>
#include <cmath>


// 简单的顶点着色器：绘制全屏四边形
const char* screenVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
}
)";

// 简单的片段着色器：显示纹理
const char* screenFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTexture;
void main()
{
    FragColor = texture(screenTexture, TexCoords);
}
)";

RayTracer::RayTracer(int w, int h) : width(w), height(h), textureID(0) {
    pixelBuffer.resize(width * height * 3);
    InitGLResources();
}

RayTracer::~RayTracer() {
    if (textureID) glDeleteTextures(1, &textureID);
    if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
    if (quadVBO) glDeleteBuffers(1, &quadVBO);
    if (screenShaderProgram) glDeleteProgram(screenShaderProgram);
}

void RayTracer::Resize(int w, int h) {
    width = w;
    height = h;
    pixelBuffer.resize(width * height * 3);
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
}

void RayTracer::InitGLResources() {
    // 创建纹理
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    // 创建全屏四边形
    float quadVertices[] = { 
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    SetupScreenShader();
}

void RayTracer::SetupScreenShader() {
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &screenVertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Check errors...

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &screenFragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Check errors...

    screenShaderProgram = glCreateProgram();
    glAttachShader(screenShaderProgram, vertexShader);
    glAttachShader(screenShaderProgram, fragmentShader);
    glLinkProgram(screenShaderProgram);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

bool RayTracer::IntersectSphere(const glm::vec3& origin, const glm::vec3& dir, const RTSphereData& sphere, float& t) {
    glm::vec3 oc = origin - sphere.center;
    float a = glm::dot(dir, dir);
    float b = 2.0f * glm::dot(oc, dir);
    float c = glm::dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4 * a * c;
    
    if (discriminant < 0) {
        return false;
    } else {
        float t1 = (-b - sqrt(discriminant)) / (2.0f * a);
        if (t1 > 0.001f) {
            t = t1;
            return true;
        }
        float t2 = (-b + sqrt(discriminant)) / (2.0f * a);
        if (t2 > 0.001f) {
            t = t2;
            return true;
        }
    }
    return false;
}

glm::vec3 RayTracer::SampleTexture(const RTTexture& tex, float u, float v) {
    if (tex.data.empty()) return glm::vec3(1.0f, 0.0f, 1.0f); // 错误紫

    // 简单的重复模式 (Repeat)
    u = u - floor(u);
    v = v - floor(v);

    int x = static_cast<int>(u * tex.width);
    int y = static_cast<int>(v * tex.height);

    // 边界检查
    x = std::max(0, std::min(x, tex.width - 1));
    y = std::max(0, std::min(y, tex.height - 1));

    int index = (y * tex.width + x) * tex.channels;
    
    float r = tex.data[index] / 255.0f;
    float g = r;
    float b = r;
    if (tex.channels > 1) {
        g = tex.data[index + 1] / 255.0f;
    }
    if (tex.channels > 2) {
        b = tex.data[index + 2] / 255.0f;
    }

    return glm::vec3(r, g, b);
}

glm::vec3 RayTracer::SampleEnvironment(const glm::vec3& dir) {
    if (!hasEnvironmentTexture) {
        return glm::vec3(0.0f);
    }

    const float invTwoPi = 1.0f / (2.0f * M_PI);
    const float invPi = 1.0f / M_PI;
    float u = 0.5f + atan2(dir.z, dir.x) * invTwoPi;
    float v = 0.5f - asin(dir.y) * invPi;
    glm::vec3 color = SampleTexture(environmentTexture, u, v);
    return color * environmentIntensity;
}

glm::vec3 RayTracer::Trace(const glm::vec3& origin, const glm::vec3& dir, 
                          const std::vector<RTSphereData>& spheres, 
                          const std::vector<RTMaterial>& materials, 
                          const std::vector<RTTexture>& textures,
                          int depth) {
    // 1. 寻找最近交点
    float closestT = std::numeric_limits<float>::max();
    int closestSphereIdx = -1;

    for (size_t i = 0; i < spheres.size(); ++i) {
        float t;
        if (IntersectSphere(origin, dir, spheres[i], t)) {
            if (t < closestT) {
                closestT = t;
                closestSphereIdx = i;
            }
        }
    }

    // 2. 未击中处理：返回背景色
    if (closestSphereIdx == -1) {
        return SampleEnvironment(dir);
    }

    const RTSphereData& hitSphere = spheres[closestSphereIdx];
    const RTMaterial& hitMat = materials[hitSphere.materialIndex];
    
    // 计算纹理颜色
    glm::vec3 albedo = hitMat.color;
    
    // 如果有纹理数据，进行采样
    // 假设 materialIndex 对应 textureIndex
    if (hitSphere.materialIndex >= 0 && hitSphere.materialIndex < textures.size()) {
        const RTTexture& tex = textures[hitSphere.materialIndex];
        if (!tex.data.empty()) {
            glm::vec3 hitPoint = origin + dir * closestT;
            glm::vec3 localPoint = hitPoint - hitSphere.center;
            glm::vec3 normal = glm::normalize(localPoint);
            
            // 球面 UV 映射
            // u = 0.5 + atan2(z, x) / (2*pi)
            // v = 0.5 - asin(y) / pi
            const float invTwoPi = 1.0f / (2.0f * M_PI);
            const float invPi = 1.0f / M_PI;
            float u = 0.5f + atan2(normal.z, normal.x) * invTwoPi;
            float v = 0.5f - asin(normal.y) * invPi;
            
            albedo = SampleTexture(tex, u, v);
        }
    }

    // 如果是发光体，直接返回自发光颜色 (混合纹理颜色)
    if (glm::length(hitMat.emission) > 0.1f) {
        return hitMat.emission * albedo; // 简单的混合
    }

    glm::vec3 hitPoint = origin + dir * closestT;
    glm::vec3 normal = glm::normalize(hitPoint - hitSphere.center);
    
    // --- 递归光线追踪逻辑 (处理透明/折射和镜面反射) ---
    if (depth > 0) {
        if (hitMat.type == MaterialType::REFRACTIVE) {
            glm::vec3 n = normal;
            glm::vec3 viewDir = glm::normalize(dir);
            float eta = 1.0f / (hitMat.ior > 0.0f ? hitMat.ior : 1.0f); // 假设空气折射率为1.0

            // 判断是射入还是射出 (法线方向与光线方向点积)
            if (glm::dot(viewDir, n) > 0) {
                n = -n; // 内部射出，法线反转
                eta = hitMat.ior; // 恢复折射率比 (Material -> Air)
            }

            glm::vec3 refractDir = glm::refract(viewDir, n, eta);
            
            if (glm::length(refractDir) > 0.0001f) {
                // 发生折射
                // 偏移起点以防自相交 (向折射方向偏移)
                return albedo * Trace(hitPoint + refractDir * 0.001f, refractDir, spheres, materials, textures, depth - 1);
            } else {
                // 全内反射 (Total Internal Reflection) -> 视为镜面反射
                glm::vec3 reflectDir = glm::reflect(viewDir, n);
                return albedo * Trace(hitPoint + reflectDir * 0.001f, reflectDir, spheres, materials, textures, depth - 1);
            }
        }
        else if (hitMat.type == MaterialType::SPECULAR) {
            // 镜面反射
            glm::vec3 reflectDir = glm::reflect(dir, normal);
            // 偏移起点以防自相交 (向法线方向偏移)
            return albedo * Trace(hitPoint + normal * 0.001f, reflectDir, spheres, materials, textures, depth - 1);
        }
    }
    // -------------------------------------------------------

    glm::vec3 viewDir = glm::normalize(-dir);
    
    // 3. 光照计算 (Phong Model)
    glm::vec3 finalColor = glm::vec3(0.0f);

    // 环境光 (Ambient)
    float ambientStrength = 0.1f;
    glm::vec3 ambient = ambientStrength * albedo;
    finalColor += ambient;

    // 遍历所有球体寻找光源
    for(size_t i = 0; i < spheres.size(); ++i) {
        // 获取潜在光源的材质
        int matIdx = spheres[i].materialIndex;
        if (matIdx < 0 || matIdx >= materials.size()) continue;
        
        const RTMaterial& lightMat = materials[matIdx];

        // 如果该球体发光，则视为光源
        if (glm::length(lightMat.emission) > 0.1f) {
             const RTSphereData& lightSphere = spheres[i];
             
             // 排除自己照亮自己
             if (i == closestSphereIdx) continue;

             glm::vec3 lightDir = glm::normalize(lightSphere.center - hitPoint);
             float distToLight = glm::length(lightSphere.center - hitPoint);

             // 阴影检测 (Shadow Ray)
             bool inShadow = false;
             for (size_t j = 0; j < spheres.size(); ++j) {
                 if (j == closestSphereIdx || j == i) continue; // 忽略自己和光源
                 float t;
                 if (IntersectSphere(hitPoint + normal * 0.001f, lightDir, spheres[j], t)) {
                     if (t < distToLight) {
                         inShadow = true;
                         break;
                     }
                 }
             }

             if (!inShadow) {
                 // 漫反射 (Diffuse)
                 float diff = std::max(glm::dot(normal, lightDir), 0.0f);
                 glm::vec3 diffuse = diff * albedo * lightMat.emission * 0.5f; 

                 // 镜面反射 (Specular) - Phong
                 float specularStrength = 0.5f;
                 float shininess = 32.0f; 
                 glm::vec3 reflectDir = glm::reflect(-lightDir, normal);
                 float spec = pow(std::max(glm::dot(viewDir, reflectDir), 0.0f), shininess);
                 glm::vec3 specular = specularStrength * spec * glm::vec3(1.0f); 

                 finalColor += diffuse + specular;
             }
        }
    }

    return glm::clamp(finalColor, 0.0f, 1.0f);
}

void RayTracer::Render(const std::vector<RTSphereData>& spheres, 
                      const std::vector<RTMaterial>& materials,
                      const std::vector<RTTexture>& textures,
                      const glm::vec3& cameraPos, 
                      const glm::mat4& view, 
                      const glm::mat4& projection,
                      const float traceTimes) {
    
    // 获取逆矩阵用于从屏幕空间反推世界空间射线
    glm::mat4 invView = glm::inverse(view);
    glm::mat4 invProj = glm::inverse(projection);

    // 简单的单线程循环 (可以优化为多线程)
    #pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // 归一化设备坐标 (NDC)
            float ndcX = (2.0f * x) / width - 1.0f;
            float ndcY = 1.0f - (2.0f * y) / height; // 注意 Y 轴翻转

            // 裁剪空间 -> 观察空间 -> 世界空间
            glm::vec4 clipCoords(ndcX, ndcY, -1.0f, 1.0f);
            glm::vec4 eyeCoords = invProj * clipCoords;
            eyeCoords = glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f); 
            glm::vec4 worldCoords = invView * eyeCoords;
            glm::vec3 rayDir = glm::normalize(glm::vec3(worldCoords));

            glm::vec3 color = Trace(cameraPos, rayDir, spheres, materials, textures, traceTimes);

            // 写入像素缓冲
            int index = (y * width + x) * 3;
            pixelBuffer[index] = static_cast<unsigned char>(color.r * 255);
            pixelBuffer[index + 1] = static_cast<unsigned char>(color.g * 255);
            pixelBuffer[index + 2] = static_cast<unsigned char>(color.b * 255);
        }
    }

    // 更新纹理
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixelBuffer.data());
}

void RayTracer::SetEnvironmentTexture(const RTTexture& env) {
    environmentTexture = env;
    hasEnvironmentTexture = (env.width > 0 && env.height > 0 && !env.data.empty());
}

void RayTracer::DrawResult() {
    glUseProgram(screenShaderProgram);
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
