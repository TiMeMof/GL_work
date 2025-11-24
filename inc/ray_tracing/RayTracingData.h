// RayTracingData.h
enum MaterialType { DIFFUSE, SPECULAR, REFRACTIVE };

struct RTMaterial {
    glm::vec3 color;
    glm::vec3 emission;
    int type;
    // 可以在这里添加更多参数，如粗糙度
};

struct RTSphereData {
    glm::vec3 center;
    float radius;
    int materialIndex;
    float padding[3]; // 对齐 GPU 内存
};