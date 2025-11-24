#include <iostream>
#include <vector>
#include <algorithm>
#include "CommonGL.h"
#include "Unified_CubeClass.h"
#include "Unified_SphereClass.h"
#include <glm.hpp>
int main()
{
    GLFWwindow* window = Initialize_OpenGL();
    
    Unified_SphereClass SUN("material/Tshader.vs", "material/Tshader.fs", "material/sun.jpg",16,16,3.0f);
    Unified_SphereClass EARTH("material/Tshader.vs", "material/Tshader.fs","material/earth.png",16,16,0.6f,0.6f);
    Unified_SphereClass MOON("material/Tshader.vs", "material/Tshader.fs","material/moon.jpg",16,16,0.2f, 0.3f);
    Unified_SphereClass sky("material/Tshader.vs", "material/Tshader.fs","material/milky_way.png", 64, 64, 50.0f);


    while (!glfwWindowShouldClose(window)) // 主渲染循环
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame; // deltaTime帧差用于相机
        camera.Update(deltaTime); // Camera平滑切换用到
        lastFrame = currentFrame;

        glClearColor(0.2f, 0.8f, 0.8f, 1.0f); // 清屏颜色RGBA
        // glClear(GL_COLOR_BUFFER_BIT); // 清空颜色缓冲
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清空颜色+深度缓冲

        processInput(window);
        StateSwitch(window);

        // 先绘制 sky（禁写深度，防止覆盖其它物体）
        glDepthMask(GL_FALSE);
        sky.Draw();
        glDepthMask(GL_TRUE);

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
        
        // 原来直接绘制： MOON.Draw(tmp2); EARTH.Draw(tmp);
        // 改为按摄像机距离（远->近）排序绘制，防止透明相互遮挡问题

        struct DrawItem { Unified_SphereClass* obj; glm::mat4 model; float dist; };
        std::vector<DrawItem> items;

        // 计算物体世界位置（从 model 的平移分量）
        glm::vec3 posMoon = glm::vec3(tmp2[3]);
        glm::vec3 posEarth = glm::vec3(tmp1[3]);
        glm::vec3 posSun = glm::vec3(tmp0[3]);

        // 计算与摄像机的距离
        float distMoon = glm::length(camera.Position - posMoon);
        float distEarth = glm::length(camera.Position - posEarth);
        float distSun = glm::length(camera.Position - posSun);

        items.push_back({ &MOON, tmp2, distMoon });
        items.push_back({ &EARTH, tmp1, distEarth });
        items.push_back({ &SUN, tmp0, distSun });

        // 按距离降序排列（远的先绘制）
        std::sort(items.begin(), items.end(), [](const DrawItem& a, const DrawItem& b) {
            return a.dist > b.dist;
        });

        for (const auto& it : items) {
            it.obj->Draw(it.model, camera.Zoom);
        }
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // 线框模式查看
        glfwSwapBuffers(window); // 双循环显像
        glfwPollEvents(); // 检查调用事件
    }

    glfwTerminate();
    return 0;
}