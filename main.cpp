#include <iostream>

#include "CommonGL.h"
#include "Unified_CubeClass.h"
#include "Unified_SphereClass.h"
#include <glm.hpp>
int main()
{
    GLFWwindow* window = Initialize_OpenGL();

    Unified_SphereClass SUN("material/Tshader.vs", "material/Tshader.fs", "material/sun.jpg",32,32,3.0f);
    Unified_SphereClass EARTH("material/Tshader.vs", "material/Tshader.fs","material/earth.png",32,32,0.6f);
    Unified_SphereClass MOON("material/Tshader.vs", "material/Tshader.fs","material/moon.jpg",32,32,0.2f);
    Unified_SphereClass sky("material/Tshader.vs", "material/Tshader.fs","material/milky_way.png", 256, 256, 50.0f);


    while (!glfwWindowShouldClose(window)) // 主渲染循环
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame; // deltaTime帧差用于相机
        camera.Update(deltaTime); // Camera平滑切换用到
        lastFrame = currentFrame;

        glClearColor(0.2f, 0.8f, 0.8f, 1.0f); // 清屏颜色RGBA
        // glClear(GL_COLOR_BUFFER_BIT); // 清空颜色缓冲
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // 清空颜色+深度+模板缓冲

        processInput(window);
        StateSwitch(window);

        // SUN ROTATE
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, .0f, 0.0f));
        model = glm::rotate(model, (float)glfwGetTime()/15, glm::vec3(.0f, 1.0f, .0f)); // 先旋转
        SUN.Draw(model);

        // EARTH ROTATE
        glm::mat4 model1 = SUN.GetModelMatrix();
        glm::mat4 tmp = glm::mat4(1.0f);
        // 只取平移部分
        for (size_t i = 0; i < 3; i++)
        {
            tmp[3][i] = model1[3][i];
        }
        tmp = glm::rotate(tmp, (float)glfwGetTime()/10, glm::vec3(.0f, .0f, 1.0f));  // 先旋转
        tmp = glm::translate(tmp, glm::vec3(8.0f, 0.0f, 0.0f));                     // 再平移
        tmp = glm::rotate(tmp, glm::radians(45.0f), glm::vec3(1.0f, .0f, 0.0f));    // 调整轴向
        tmp = glm::rotate(tmp, (float)glfwGetTime()/2, glm::vec3(.0f, 1.0f, .0f));  // 自转
        EARTH.Draw(tmp);

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
        MOON.Draw(tmp2);

        sky.Draw();
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // 线框模式查看
        glfwSwapBuffers(window); // 双循环显像
        glfwPollEvents(); // 检查调用事件
    }

    glfwTerminate();
    return 0;
}