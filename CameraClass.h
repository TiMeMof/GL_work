#pragma once

#include <glad.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

class Camera
{
public:
    enum Camera_Movement
    {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // 轨迹球建模相机参数
    bool  ModelMode = false;
    float Radius = 10.0f; // 默认轨迹球半径（滚轮切换）

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f), float yaw = 0.0f, float pitch = 0.0f) : Front(glm::vec3(0.0f, -1.0f, 0.0f)), MovementSpeed(5.0f), MouseSensitivity(0.1f), Zoom(45.0f)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(5.0f), MouseSensitivity(0.1f), Zoom(45.0f)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    void ChangeMode() // 切换轨迹球相机/fps运动相机
    {
        if (isTransitioning) return; // 如果已经在过渡中，则忽略本次请求

        isTransitioning = true;
        transitionProgress = 0.0f;

        if (ModelMode) // 当前是 Orbit → 切换到 FPS
        {
            transition.startEye = Position - Radius * Front;
            transition.startCenter = Position;
            transition.endEye = Position;
            transition.endCenter = Position + Front;
        }
        else // 当前是 FPS → 切换到 Orbit
        {
            transition.startEye = Position;
            transition.startCenter = Position + Front;
            transition.endEye = Position - Radius * Front;
            transition.endCenter = Position;
        }

        ModelMode = !ModelMode; // ==== 最后真正改变状态标识 ====
    }

    // 每帧调用一次，用于更新相机状态（包括过渡动画）
    void Update(float deltaTime)
    {
        if (!isTransitioning) return;

        transitionProgress += deltaTime / transitionDuration; // 更新进度条（归一化到 [0,1]）
        if (transitionProgress > 1.0f)
        {
            transitionProgress = 1.0f;
            isTransitioning = false;
            //ModelMode = !ModelMode; //
        }

        float easedT = easeOutQuad(transitionProgress); // 应用缓动函数（easeOutQuad）：先快后慢

        glm::vec3 currentEye = mixVectors(transition.startEye, transition.endEye, easedT); // 插值Eye
        glm::vec3 currentCenter = mixVectors(transition.startCenter, transition.endCenter, easedT); // 插值Center
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        if (isTransitioning)
        {
            float easedT = easeOutQuad(transitionProgress);
            glm::vec3 currentEye = mixVectors(transition.startEye, transition.endEye, easedT);
            glm::vec3 currentCenter = mixVectors(transition.startCenter, transition.endCenter, easedT);
            return glm::lookAt(currentEye, currentCenter, Up);
        }
        else
        {
            if (!ModelMode) return glm::lookAt(Position,                  Position + Front, Up); // fps运动相机视角
            else            return glm::lookAt(Position - Radius * Front, Position,         Up); // 轨迹球视角（仅仅是lookAt不同，相机的属性本身没变）
        }
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        // 控制只在水平面运动：Front向水平投影（pitch）
        glm::vec3 Front0 = glm::normalize(glm::vec3(Front.x, Front.y, 0.0f));
        glm::vec3 Right0 = glm::normalize(glm::vec3(Right.x, Right.y, 0.0f));
        if (direction == FORWARD)
            Position += Front0 * velocity;
        if (direction == BACKWARD)
            Position -= Front0 * velocity;
        if (direction == LEFT)
            Position -= Right0 * velocity;
        if (direction == RIGHT)
            Position += Right0 * velocity;
        if (direction == UP)
            Position += WorldUp * velocity;
        if (direction == DOWN)
            Position -= WorldUp * velocity;
        //增加上下移动
        // glm::vec3 up0 = glm::normalize(glm::vec3(Up.x, Up.y, 0.0f));
        // if (direction == UP)
        //     Position += up0 * velocity;
        // if (direction == DOWN)
        //     Position -= up0 * velocity;
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean right_move_camera = false) // 修改最后一个参数指示是否为右键，以平移相机
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        if (right_move_camera) // 为true状态下（在外面控制为右键按住时）
        {
            glm::vec3 cX = xoffset*glm::normalize(glm::cross(Front, Up));
            glm::vec3 cY = yoffset*glm::normalize(Up);
            Position -= 0.05f * (cX + cY);
        }
        else // 右键时视角不变
        {
            Yaw   += xoffset;
            Pitch += yoffset;
        }

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset) // 用来调整非fps模式的半径而不是视野
    {
        Radius -= (float)yoffset;
        if (Radius < 1.0f)
            Radius = 1.0f;
        if (Radius > 45.0f)
            Radius = 45.0f;
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.z = sin(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up    = glm::normalize(glm::cross(Right, Front));
    }


    // ===== 新增：用于平滑过渡的状态变量 =====
    bool isTransitioning = false;          // 是否正在过渡中
    float transitionProgress = 0.0f;       // 当前过渡进度 [0~1]
    float transitionDuration = 0.5f;       // 过渡总时长（秒）

    struct TransitionState {
        glm::vec3 startEye;      // 过渡开始时的相机位置
        glm::vec3 endEye;        // 过渡结束时的相机位置
        glm::vec3 startCenter;   // 过渡开始时的观察目标
        glm::vec3 endCenter;     // 过渡结束时的观察目标
    } transition;

    // 混合两个三维向量的工具函数
    glm::vec3 mixVectors(const glm::vec3& a, const glm::vec3& b, float t) { return a * (1.0f - t) + b * t; }

    // Ease Out Quad 缓动函数（先快后慢）
    float easeOutQuad(float x) { return 1.0f - (1.0f - x) * (1.0f - x); } // 等同于 x*(2-x)
};