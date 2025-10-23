#pragma once
#include "raylib.h"
#include "inputSystem.hpp"
#include "timeSystem.hpp"
// namespace CameraSystem{
//     static Camera2D camera = {0};
//     Vector2 CameraTarget = {0, 0};

//     static void initCamera()
//     {
//         CameraSystem::camera.rotation = 0;
//         CameraSystem::camera.zoom = 1.0f;
//         CameraSystem::camera.target = CameraSystem::CameraTarget;
//     }

//     static void updateCamera()
//     {
//         if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
//         {
//             Vector2 mosDel = GetMouseDelta();
//             CameraTarget = Vector2Subtract(CameraTarget, Vector2Scale(mosDel, 1.0f / camera.zoom));
//         }

//         const float wheel = GetMouseWheelMove();
//         if (wheel != 0.0f)
//         {
//             // Zoom toward the mouse position (correct function)
//             Vector2 worldPos = GetScreenToWorld2D(GetMousePosition(), camera);
//             camera.target = worldPos;

//             camera.zoom += wheel * 0.125f;
//             if (camera.zoom < 0.125f)
//                 camera.zoom = 0.125f;
//         }
//         camera.target = CameraTarget;
//     }

// }


namespace CameraSystem3D{
    static Camera camera = {0};
    Vector3 CameraTarget = {0,0,0};
    Vector3 cameraOffset{50, 50, 50};
    float cameraSpeed = 10.0f;

    static void initCamera()
    {
        camera.fovy = 60.0f;
        camera.position = Vector3{0, 3.0f, 4};
        camera.projection = CAMERA_PERSPECTIVE;
        camera.target = CameraTarget;
        camera.up = Vector3{0, 1, 0};
    }

    static void updateCamera()
    {
        camera.target = CameraTarget;
        camera.position = Vector3Add(CameraTarget, cameraOffset);
        UpdateCamera(&camera, CAMERA_CUSTOM);
    }

}