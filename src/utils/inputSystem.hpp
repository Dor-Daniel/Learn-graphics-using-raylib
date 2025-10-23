#pragma once
#include "raylib.h"
#include "raymath.h"

namespace Input{
    Vector3 moveDelta { 0 };
    void update(){
        moveDelta = { 0 };
        if(IsKeyDown(KEY_A)){
            moveDelta = Vector3Add(moveDelta, Vector3{1, 0, 0});
        }
        if(IsKeyDown(KEY_D)){
            moveDelta = Vector3Add(moveDelta, Vector3{-1, 0, 0});
        }
        if(IsKeyDown(KEY_W)){
            moveDelta = Vector3Add(moveDelta, Vector3{0, 0, 1});
        }
        if(IsKeyDown(KEY_A)){
            moveDelta = Vector3Add(moveDelta, Vector3{0, 0, -1});
        }
    }
} 