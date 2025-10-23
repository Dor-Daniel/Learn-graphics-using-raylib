#pragma once
#include "raylib.h"
#include "raymath.h"
#include <vector>
#include "../utils/QuadTree.hpp"
#include "../utils/dorMath.hpp"
#include "../utils/cameraSystem.hpp"

namespace FlockSimulation{
// Simulation params
// -------------------------------------------
static int sizeX = 8192;
static int sizeY = 8192;

// Movement scale 
static float ballSpeed = 75.0f;

// Circle/collision radius
static float ballRadius = 25.0f;

// Boids tuning 
static float perceptionRadius = 140.0f; 
static float separationRadius = 120.0f; 
static float maxSpeed = 1.8f;           
static float maxForce = 0.06f;          

// Behavior weights
static float wSeparation = 1.35f;
static float wAlignment = 0.75f;
static float wCohesion = 0.60f;

struct Ball
{
    float x, y;
    Vector2 vel;
    Vector2 acc;
    Rectangle bounds;

    inline void updateKinematics(float dt)
    {
        
        vel.x += acc.x * dt * ballSpeed;
        vel.y += acc.y * dt * ballSpeed;


        vel = vlimit(vel, maxSpeed);

        x += vel.x * dt * ballSpeed;
        y += vel.y * dt * ballSpeed;

        acc = Vector2{0, 0};

        bounds.x = x - ballRadius;
        bounds.y = y - ballRadius;
        bounds.width = 2.0f * ballRadius;
        bounds.height = 2.0f * ballRadius;
    }

    inline void addForce(Vector2 f) { acc = Vector2Add(acc, f); }


    inline void flock(const std::vector<const Ball *> &neighbors)
    {
        Vector2 sumVel = {0, 0};
        Vector2 sumPos = {0, 0};
        Vector2 sepAcc = {0, 0};

        int countPAC = 0; 
        int countSEP = 0; 

        const float pr2 = perceptionRadius * perceptionRadius;
        const float sr2 = separationRadius * separationRadius;

        for (const Ball *b : neighbors)
        {
            if (b == this)
                continue;

            const float dx = b->x - x;
            const float dy = b->y - y;
            const float d2 = dx * dx + dy * dy;
            if (d2 > 0.0001f && d2 <= pr2)
            {
                
                sumVel = Vector2Add(sumVel, b->vel);
                sumPos = Vector2Add(sumPos, Vector2{b->x, b->y});
                ++countPAC;

               
                if (d2 <= sr2)
                {
                    const float d = sqrtf(d2);
                    Vector2 away = Vector2{x - b->x, y - b->y};
                    away = Vector2Scale(away, 1.0f / (d + 1e-4f));
                    sepAcc = Vector2Add(sepAcc, away);
                    ++countSEP;
                }
            }
        }

        Vector2 steer = {0, 0};

        // Alignment
        if (countPAC > 0)
        {
            Vector2 avgVel = Vector2Scale(sumVel, 1.0f / (float)countPAC);
            Vector2 desiredA = vsafe_normalize(avgVel);
            desiredA = Vector2Scale(desiredA, maxSpeed);
            Vector2 steerA = Vector2Subtract(desiredA, vel);
            steerA = vlimit(steerA, maxForce);
            steer = Vector2Add(steer, Vector2Scale(steerA, wAlignment));

            // Cohesion
            Vector2 center = Vector2Scale(sumPos, 1.0f / (float)countPAC);
            Vector2 toCenter = Vector2{center.x - x, center.y - y};
            Vector2 desiredC = vsafe_normalize(toCenter);
            desiredC = Vector2Scale(desiredC, maxSpeed);
            Vector2 steerC = Vector2Subtract(desiredC, vel);
            steerC = vlimit(steerC, maxForce);
            steer = Vector2Add(steer, Vector2Scale(steerC, wCohesion));
        }

        // Separation
        if (countSEP > 0)
        {
            Vector2 desiredS = vsafe_normalize(sepAcc);
            desiredS = Vector2Scale(desiredS, maxSpeed);
            Vector2 steerS = Vector2Subtract(desiredS, vel);
            steerS = vlimit(steerS, maxForce);
            steer = Vector2Add(steer, Vector2Scale(steerS, wSeparation));
        }

        addForce(steer);
    }

    inline void wrapEdges()
    {
        if (x > sizeX)
            x = 0;
        else if (x < 0)
            x = (float)sizeX;
        if (y > sizeY)
            y = 0;
        else if (y < 0)
            y = (float)sizeY;
    }
};

static inline void resolveCollision(Ball &a, Ball &b)
{
    const float r = 2.0f * ballRadius;

    const float dx = b.x - a.x;
    const float dy = b.y - a.y;
    const float dist2 = dx * dx + dy * dy;

    if (dist2 <= 0.000001f || dist2 > r * r)
        return; 

    const float dist = sqrtf(dist2);
    const float nx = dx / dist;
    const float ny = dy / dist;

    const float rvx = b.vel.x - a.vel.x;
    const float rvy = b.vel.y - a.vel.y;
    const float velAlongNormal = rvx * nx + rvy * ny;

    if (velAlongNormal <= 0.0f)
    {
        const float j = -(1.0f + 1.0f) * velAlongNormal / 2.0f; 
        const float ix = j * nx;
        const float iy = j * ny;

        a.vel.x -= ix;
        a.vel.y -= iy;
        b.vel.x += ix;
        b.vel.y += iy;
    }

    const float penetration = (2.0f * ballRadius) - dist;
    if (penetration > 0.0f)
    {
        const float percent = 0.8f; 
        const float slop = 0.01f;   
        const float corrMag = percent * ((penetration - slop > 0.0f) ? (penetration - slop) : 0.0f) * 0.5f;

        a.x -= nx * corrMag;
        a.y -= ny * corrMag;
        b.x += nx * corrMag;
        b.y += ny * corrMag;

        
        a.bounds.x = a.x - ballRadius;
        a.bounds.y = a.y - ballRadius;
        b.bounds.x = b.x - ballRadius;
        b.bounds.y = b.y - ballRadius;
    }
}

static QuadTree<Ball> *qt = new QuadTree<Ball>(
    Vector2{(float)sizeX * 0.5f, (float)sizeY * 0.5f},
    (float)sizeX, (float)sizeY, 8, 0 
);

static std::vector<Ball *> balls;

// Setup
// -------------------------------------------
static void prepare(int initialCount)
{
    if (initialCount <= 0) 
        return;
    balls.reserve(initialCount);

    for (int i = 0; i < initialCount; ++i)
    {
        float x = random_ab(0.0f, (float)sizeX);
        float y = random_ab(0.0f, (float)sizeY);

        Ball *b = new Ball{
            x, y,
            Vector2{random_ab(-1.0f, 1.0f), random_ab(-1.0f, 1.0f)},
            Vector2{0.0f, 0.0f},
            Rectangle{x - ballRadius, y - ballRadius, 2.0f * ballRadius, 2.0f * ballRadius}};
        balls.emplace_back(b);
    }
    qt->rebuild(balls);
}
        
// Frame 
// -------------------------------------------
static void frame()
{
    // Input: spawn a ball at cursor
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        const Vector2 wp = GetScreenToWorld2D(GetMousePosition(), CameraSystem::camera);
        Ball *b = new Ball{
            wp.x, wp.y,
            Vector2{random_ab(-1.0f, 1.0f), random_ab(-1.0f, 1.0f)},
            Vector2{0.0f, 0.0f},
            Rectangle{wp.x - ballRadius, wp.y - ballRadius, 2.0f * ballRadius, 2.0f * ballRadius}};
        balls.emplace_back(b);
    }
    qt->rebuild(balls);

    const float dt = GetFrameTime();
    std::vector<const Ball *> neighbors;
    neighbors.reserve(64);

    for (Ball *a : balls)
    {
        neighbors.clear();

        const Rectangle queryBehavior{
            a->x - perceptionRadius,
            a->y - perceptionRadius,
            2.0f * perceptionRadius,
            2.0f * perceptionRadius};
        qt->rectQuery(queryBehavior, neighbors);

        a->flock(neighbors);
    }
    for (Ball *b : balls)
    {
        b->updateKinematics(dt);
        b->wrapEdges();
    }
    qt->rebuild(balls);

    std::vector<const Ball *> close;
    close.reserve(32);

    for (Ball *a : balls)
    {
        close.clear();
        const float inflate = 1.0f;
        const Rectangle queryCollision{
            a->bounds.x - inflate,
            a->bounds.y - inflate,
            a->bounds.width + 2 * inflate,
            a->bounds.height + 2 * inflate};
        qt->rectQuery(queryCollision, close);

        for (const Ball *bp : close)
        {
            if (bp == a)
                continue;
            if (bp < a)
                continue; 
            resolveCollision(*a, *const_cast<Ball *>(bp));
        }
    }

    for (Ball *b : balls)
    {
        Vector2 v = b->vel;
        if (Vector2Length(v) > 0.0001f)
        {
            v = Vector2Scale(vsafe_normalize(v), ballRadius + 12.0f);
            DrawLine((int)b->x, (int)b->y, (int)(b->x + v.x), (int)(b->y + v.y), YELLOW);
        }
        DrawCircleLines((int)b->x, (int)b->y, ballRadius, WHITE);
    }
    qt->setDebugMode(true);
    qt->drawDebug();
}
}