#pragma once
#include "raylib.h"

namespace Time{
    float dt;
    void update(){
        dt = GetFrameTime();
    }
}