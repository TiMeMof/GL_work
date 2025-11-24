#include <iostream>
#include <vector>
#include <algorithm>
#include "CommonGL.h"
#include "Unified_CubeClass.h"
#include "Unified_SphereClass.h"
#include <glm.hpp>
#include "RayTracer.h" // 引入 CPU 光线追踪器
#include "RayTracingData.h"

int main()
{   float scale_screen = 2/3.0f;
    float weidth = 1920.0f * scale_screen;
    float height = 1080.0f * scale_screen;
    GLFWwindow* window = Initialize_OpenGL(weidth, height); // 初始化OpenGL（创建窗口，设置上下文等）
    
    // 初始化 CPU 光线追踪器
    RayTracer rayTracer(weidth, height);

    Unified_SphereClass SUN("material/Tshader.vs", "material/Tshader.fs", "material/sun.jpg",8,8,2.0f);
    Unified_SphereClass EARTH("material/Tshader.vs", "material/Tshader.fs","material/earth.png",8,8,0.6f,0.6f);
    Unified_SphereClass MOON("material/Tshader.vs", "material/Tshader.fs","material/moon.jpg",8,8,0.2f, 0.3f);
    Unified_SphereClass sky("material/Tshader.vs", "material/Tshader.fs","material/milky_way.png",16,16, 50.0f);

    RTTexture skyTexture;
    sky.GetTextureData(skyTexture.width, skyTexture.height, skyTexture.channels, skyTexture.data);
    rayTracer.SetEnvironmentTexture(skyTexture);

    // 设置光追材质
    SUN.SetRTMaterial(glm::vec3(0.9f, 0.9f, 0.8f), glm::vec3(1.f,1.f,1.f), MaterialType::DIFFUSE); 
    EARTH.SetRTMaterial(glm::vec3(0.2f, 0.4f, 0.8f), glm::vec3(0.0f), MaterialType::DIFFUSE,0,1.9f);
    // 修改：颜色设为纯白(1.0)以减少光线吸收；折射率设为 1.1 (接近空气) 以减少透镜扭曲
    // EARTH.SetRTMaterial(glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f), MaterialType::REFRACTIVE, 0.0f, 1.45f); 
    MOON.SetRTMaterial(glm::vec3(0.7f, 0.7f, 0.7f), glm::vec3(0.0f), MaterialType::DIFFUSE);

    // TODO: 增加一个飞船class,表面是镜面反射
    while (!glfwWindowShouldClose(window)) // 主渲染循环
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame; // deltaTime帧差用于相机
        camera.Update(deltaTime); // Camera平滑切换用到
        lastFrame = currentFrame;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 清屏颜色黑色
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        processInput(window);
        StateSwitch(window);

        // SUN ROTATE
        glm::mat4 tmp0 = glm::mat4(1.0f);
        tmp0 = glm::rotate(tmp0, glm::radians(90.0f), glm::vec3(1.0f, .0f, 0.0f));
        tmp0 = glm::rotate(tmp0, (float)glfwGetTime()/15, glm::vec3(.0f, 1.0f, .0f)); // 先旋转

        // EARTH ROTATE
        glm::mat4 model1 = SUN.GetModelMatrix();
        glm::mat4 tmp1 = glm::mat4(1.0f);
        // 只取平移部分
        for (size_t i = 0; i < 3; i++)
        {
            tmp1[3][i] = model1[3][i];
        }
        tmp1 = glm::rotate(tmp1, (float)glfwGetTime()/10, glm::vec3(.0f, .0f, 1.0f));  // 先旋转
        tmp1 = glm::translate(tmp1, glm::vec3(8.0f, 0.0f, 0.0f));                     // 再平移
        tmp1 = glm::rotate(tmp1, glm::radians(45.0f), glm::vec3(1.0f, .0f, 0.0f));    // 调整轴向
        tmp1 = glm::rotate(tmp1, (float)glfwGetTime()/2, glm::vec3(.0f, 1.0f, .0f));  // 自转
        

        // MOON ROTATE
        glm::mat4 model2 = EARTH.GetModelMatrix();
        glm::mat4 tmp2 = glm::mat4(1.0f);
        // 只取平移部分
        for (size_t i = 0; i < 3; i++)
        {
            tmp2[3][i] = model2[3][i];
        }
        tmp2 = glm::rotate(tmp2, (float)glfwGetTime(), glm::vec3(.0f, .0f, 1.0f));  // 先旋转
        tmp2 = glm::translate(tmp2, glm::vec3(1.f, 0.0f, 0.0f));                     // 再平移
        tmp2 = glm::rotate(tmp2, (float)glfwGetTime()*2, glm::vec3(.0f, 1.0f, .0f));  // 自转
        
        // 更新对象内部矩阵
        SUN.SetModelMatrix(tmp0);
        EARTH.SetModelMatrix(tmp1);
        MOON.SetModelMatrix(tmp2);

        // 收集光追数据
        std::vector<RTSphereData> spheres;
        std::vector<RTMaterial> materials;
        std::vector<RTTexture> textures; // 新增纹理列表

        // 辅助 lambda：添加对象到列表
        auto AddObject = [&](Unified_SphereClass& obj, int matIndex) {
            RTSphereData data = obj.GetRTData();
            data.materialIndex = matIndex;
            spheres.push_back(data);
            materials.push_back(obj.GetRTMaterial());
            
            // 提取纹理数据
            RTTexture tex;
            obj.GetTextureData(tex.width, tex.height, tex.channels, tex.data);
            textures.push_back(tex);
        };

        // 太阳 (Index 0)
        AddObject(SUN, 0);
        // 地球 (Index 1)
        AddObject(EARTH, 1);
        // 月球 (Index 2)
        AddObject(MOON, 2);
        // // 天空球 (Index 3)

        // 执行 CPU 光线追踪渲染
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), weidth / height, 0.1f, 100.0f);
        
        rayTracer.Render(spheres, materials, textures, camera.Position, view, projection, 5);
        
        // 绘制结果到屏幕
        rayTracer.DrawResult();

        glfwSwapBuffers(window); // 双循环显像
        glfwPollEvents(); // 检查调用事件
    }

    glfwTerminate();
    return 0;
}