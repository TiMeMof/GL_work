#include <iostream>

#include "CommonGL.h"
#include "Unified_CubeClass.h"
#include "Unified_SphereClass.h"
#include <glm.hpp>
int main()
{
    GLFWwindow* window = Initialize_OpenGL();

    Unified_SphereClass sphere("material/Tshader.vs", "material/Tshader.fs", "material/sun.png");
    Unified_SphereClass sphere2("material/Tshader.vs", "material/Tshader.fs","material/earth.png");

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

        sphere.Draw();
        glm::mat4 model1 = sphere.GetModelMatrix();
        model1 = glm::translate(model1, glm::vec3(2.0f, 0.0f, 0.0f));
        model1 = glm::rotate(model1, (float)glfwGetTime(), glm::vec3(.0f, 1.0f, .0f)); 
        sphere2.Draw(model1);

        glfwSwapBuffers(window); // 双循环显像
        glfwPollEvents(); // 检查调用事件
    }

    glfwTerminate();
    return 0;
}