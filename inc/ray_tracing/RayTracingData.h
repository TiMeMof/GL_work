// RayTracingData.h
#pragma once
#include <glm.hpp>

enum MaterialType { DIFFUSE, SPECULAR, REFRACTIVE };

struct RTMaterial {
    glm::vec3 color;       // TODO: 光的颜色与物体颜色的影响
    float padding1;        // 对齐
    glm::vec3 emission;    // 自发光，设置rgb三个分量的强度
    float padding2;        // 对齐
    int type;              // 材质类型 (DIFFUSE:漫反射, SPECULAR:镜面, REFRACTIVE:折射)
    float roughness;       // 粗糙度 (可选)
    float ior;             // 折射率 (可选)
    float padding3;        // 对齐
};

struct RTSphereData {
    glm::vec3 center;      // 球心
    float radius;          // 半径
    int materialIndex;     // 材质索引
    float padding[3];      // 对齐 GPU 内存 (std430 alignment)
};
