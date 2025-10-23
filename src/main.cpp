// Includes
// -------------------------------------------
#include "raylib.h"
#include "rcamera.h"
#include "raymath.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include "utils/QuadTree.hpp"
#include "utils/dorMath.hpp"
#include "utils/cameraSystem.hpp"
#include "utils/inputSystem.hpp"
#include "utils/shaderSystem.hpp"
#include "sims/flockSim.hpp"
#include <string>
#include <list>
#include <memory>
#include <cmath>
#include <math.h>
#include <iostream>
#include <rlgl.h>
#include "miniGames/FallingCubes.hpp"

// Globals
// -------------------------------------------
#define WIDTH 1000
#define HEIGHT 1500
#define BACKGROUND_COLOR Color{ .r = 20, .g = 20, .b = 20, .a = 255}

// Init / Shutdown handlers
// -------------------------------------------
static void Init()
{
    
    SetConfigFlags(
        FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_UNDECORATED
    );
    InitWindow(WIDTH, HEIGHT, "raylib");
    SetTargetFPS(60);
    CameraSystem3D::initCamera();
    
    SetTargetFPS(60);
}

static void Shutdown()
{
    ShaderSystem::cleanup();
    CloseWindow();
}

static void Update(){
    CameraSystem3D::updateCamera();        
    Time::update();
}

enum class Interpolation{ LINEAR, CUBIC };

struct Object{
    Vec3 *vertices;
    size_t* triangles;
};

const Object Cube = Object{
    .vertices = (Vec3*)malloc(8 * sizeof(Vec3)),
    .triangles = (size_t*)malloc(36 * sizeof(size_t))
};


int main(void)
{
    Init();

    Cube.vertices[0] = Vec3{ 1,  1,  1};
    Cube.vertices[1] = Vec3{ 1,  1, -1};
    Cube.vertices[2] = Vec3{ 1, -1,  1};
    Cube.vertices[4] = Vec3{ 1, -1, -1};
    Cube.vertices[5] = Vec3{-1,  1,  1};
    Cube.vertices[6] = Vec3{-1,  1, -1};
    Cube.vertices[3] = Vec3{-1, -1,  1};
    Cube.vertices[7] = Vec3{-1, -1, -1};


    Cube.triangles[0]  = 0;
    Cube.triangles[1]  = 0;
    Cube.triangles[2]  = 0;
    Cube.triangles[3]  = 0;
    Cube.triangles[4]  = 0;
    Cube.triangles[5]  = 0;
 
    Cube.triangles[6]  = 0;
    Cube.triangles[7]  = 0;
    Cube.triangles[8]  = 0;
    Cube.triangles[9]  = 0;
    Cube.triangles[10] = 0;
    Cube.triangles[11] = 0;

    Cube.triangles[12] = 0;
    Cube.triangles[13] = 0;
    Cube.triangles[14] = 0;
    Cube.triangles[15] = 0;
    Cube.triangles[16] = 0;
    Cube.triangles[17] = 0;

    Cube.triangles[18] = 0;
    Cube.triangles[19] = 0;
    Cube.triangles[20] = 0;
    Cube.triangles[21] = 0;
    Cube.triangles[22] = 0;
    Cube.triangles[23] = 0;

    Cube.triangles[24] = 0;
    Cube.triangles[25] = 0;
    Cube.triangles[26] = 0;
    Cube.triangles[27] = 0;
    Cube.triangles[28] = 0;
    Cube.triangles[29] = 0;

    Cube.triangles[30] = 0;
    Cube.triangles[31] = 0;
    Cube.triangles[32] = 0;
    Cube.triangles[33] = 0;
    Cube.triangles[34] = 0;
    Cube.triangles[35] = 0;




    CameraSystem3D::camera.up = Vector3{0,0,1};
    CameraSystem3D::cameraOffset = Vector3{0, 100, 0};

    while (!WindowShouldClose())
    {
        // updates 
        Update();
        
        BeginDrawing();
            ClearBackground(BACKGROUND_COLOR);

            BeginMode3D(CameraSystem3D::camera);
                DrawCube(Vector3{0, 0, 0}, 10, 10, 10, DARKPURPLE);
                DrawCubeWires(Vector3{0, 0, 0}, 10, 10, 10, WHITE);
                DrawLine3D(Vector3{0, 25, 0}, Vector3{0, 0, 0}, RED);
            EndMode3D();

        EndDrawing();

    }

    Shutdown();

    return EXIT_SUCCESS;
}
