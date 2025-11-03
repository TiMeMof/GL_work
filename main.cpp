#include <iostream>

#include "CommonGL.h"
#include "Unified_CubeClass.h"

int main()
{
    GLFWwindow* window = Initialize_OpenGL();

    Unified_CubeClass cube("material/Tshader.vs", "material/Tshader.fs");

    while (!glfwWindowShouldClose(window)) // 主渲染循环
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame; // deltaTime帧差用于相机
        camera.Update(deltaTime); // Camera平滑切换用到
        lastFrame = currentFrame;

        glClearColor(0.2f, 0.8f, 0.8f, 1.0f); // 清屏颜色RGBA
        //glClear(GL_COLOR_BUFFER_BIT); // 清空颜色缓冲
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 同时清空颜色和深度缓存

        processInput(window);
        StateSwitch(window);

        cube.Draw();

        glfwSwapBuffers(window); // 双循环显像
        glfwPollEvents(); // 检查调用事件
    }

    glfwTerminate();
    return 0;
}