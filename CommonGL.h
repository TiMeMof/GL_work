#pragma once

#include <glad.h>
#include <glfw3.h>
#include <glm.hpp>
#include "CameraClass.h"

Camera camera;

bool firstMouse = true;
float lastX = 1920.0f / 2.0f; // initial position of mouse
float lastY = 1080.0f / 2.0f; 
float deltaTime = 0.0f; // 当前帧与上一帧的时间差（平衡不同设备帧时间）
float lastFrame = 0.0f; // 上一帧的时间

std::pair<bool, bool> fpsMode = {false, true};    // 上一帧/本帧是否为fps运动相机状态（ESC切换出鼠标及切回）
std::pair<bool, bool> Key_Enter   = {false, false}; // 键盘Enter切换视角模式


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;
    

    if (fpsMode.second == false && camera.ModelMode == false) return; // 在设置中不会改变视角

    if (fpsMode.second || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        camera.ProcessMouseMovement(xoffset, yoffset, false); // 更新视角：1.fps下任意移动鼠标 2.非fps下按住鼠标左键
    }

    if(!fpsMode.second && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        camera.ProcessMouseMovement(xoffset, yoffset, true); // 右键平移: 仅当非fps模式下按住鼠标右键
    }
}

// 鼠标滚轮回调函数
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (!fpsMode.second) { camera.ProcessMouseScroll(yoffset); } // 非fps模式下滚轮切换的是Radio
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
    {
        Key_Enter.second = true;
        if (Key_Enter.first == false)
        {
            fpsMode.second = !fpsMode.second; // 单次锁！
            camera.ChangeMode(); // 同时改变camera状态(非fps下自动为轨迹球相机)
        }
    }
    else { Key_Enter.second = false; }
    Key_Enter.first = Key_Enter.second;

}

void StateSwitch(GLFWwindow *window)
{
    if (fpsMode.first != fpsMode.second)
    {
        fpsMode.second ?
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED) : // fps模式无指针
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);    // 非fps状态调出鼠标指针
        fpsMode.first = fpsMode.second;     // 随后本帧状态变为上一帧~
    }
}


GLFWwindow* Initialize_OpenGL() // 把OpenGL通用的初始化配置封装
{
    glfwInit(); // glfw初始化，版本配置为3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // MacOS系统兼容
#endif

    GLFWwindow* window = glfwCreateWindow(1920.0f, 1080.0f, "OpenGL World", NULL, NULL); // 创建glfw窗口
    if (window == NULL) { throw std::runtime_error("Failed to create GLFW window"); glfwTerminate(); }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // 注册窗口尺寸回调函数
    glfwSetCursorPosCallback(window, mouse_callback);                  // 注册鼠标移动回调函数
    glfwSetScrollCallback(window, scroll_callback);                    // 注册鼠标滚轮回调函数
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // 默认使用fps无指针鼠标

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { throw std::runtime_error("Failed to initialize GLAD"); } // glad测试
    
    glEnable(GL_DEPTH_TEST); // 开启深度测试
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 针对透明物体：提前设置好混合参数（但初始化时不启用）

    return window;
}