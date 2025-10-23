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
#define WIDTH 2000
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
    CameraSystem::initCamera();
    
    SetTargetFPS(60);
}

static void Shutdown()
{
    ShaderSystem::cleanup();
    CloseWindow();
}

static void Update(){
    CameraSystem::updateCamera();        
    Time::update();
}




int main(void)
{
    Init();
    int initialCount = 1000;
    FlockSimulation::prepare(initialCount);

    while (!WindowShouldClose())
    {
        // updates 
        Update();
        
        BeginDrawing();
            ClearBackground(BACKGROUND_COLOR);

            BeginMode2D(CameraSystem::camera);
            FlockSimulation::frame();
            EndMode2D();

        EndDrawing();

    }

    Shutdown();

    return EXIT_SUCCESS;
}
