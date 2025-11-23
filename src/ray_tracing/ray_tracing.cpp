#include <iostream>
#include <vector>
#include <algorithm>
#include "CommonGL.h"
#include "Unified_CubeClass.h"
#include "Unified_SphereClass.h"
#include <glm.hpp>

int main()
{
    GLFWwindow *window = Initialize_OpenGL();

    while (!glfwWindowShouldClose(window))
    {
        // Render loop

        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // 线框模式查看
        glfwSwapBuffers(window); // 双循环显像
        glfwPollEvents();        // 检查调用事件
    }

    glfwTerminate();
    return 0;
}