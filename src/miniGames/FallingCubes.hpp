#pragma once

#include "../utils/dorMath.hpp"
#include <vector>
#include "raylib.h"
#include "raymath.h"
#include <rlgl.h>
#include "../utils/cameraSystem.hpp"
#include "../utils/timeSystem.hpp"
#include "../utils/inputSystem.hpp"

#define CAMERA_SPEED 0.05f

#define MAX_MOVEMENT 12

#define SCORE_ANIM_DURATION 0.2f
#define SCORE_ANIM_SCALE 1.5f

#define OVERLAY_ANIM_OFFSET -100.0f
#define OVERLAY_ANIM_FADE_SPEED 1.0f

#define COLLAPSE_ANIM_DURATION 0.7f
#define COLLAPSE_ANIM_MAX_BLOCK_TIME 0.99f
#define COLLAPSE_ANIM_ROTATION_SPEED 4.0f

enum class Direction   { FORWARD, BACKWARD };
enum class Axis        { X, Z };
enum class GameState   { READY, RUNING, OVER, RESET };
enum class OverlayType { GAME_START, GAME_OVER };
enum class FadeState   { IN, OUT, NONE };
enum class RenderMode { WIRES, SHADOWS };

struct Movement{
    float speed;
    Direction dir;
    Axis axis;
};

struct FallingBlock{
    bool active = true;
    Vec3 pos;
    Vec3 size;
    Vec3 rot;
    Vec3 rotSpeed;
    Vec3 velocity;
    Color baseCol;
    Color outlineCol;
};

struct Removal{
    float timer;
    float delay;
    float scale;
    float rotation;
};

struct Block{
    size_t index;
    size_t colorOffset;
    Vec3 pos;
    Vec3 size;
    Color baseCol;
    Color outlineCol;
    Movement movement;
    Removal removal;
};

struct ScoreAnimation{
    float scale;
    float duration;
};

struct OverlayAnimation{
    OverlayType overlayType;
    FadeState fadeState;
    float alpha;
    float offset;
};

struct Animations{
    ScoreAnimation scoreAnimation;
    OverlayAnimation overlayAnimation;
};

struct Game{
    Shader lightingShader;
    Model cubeModel;
    std::vector<Block> placedBlocks;
    std::vector<FallingBlock> fallingBlocks;
    Block* curr = nullptr;
    Block* prev = nullptr;
    GameState state;
    Animations animations;
    RenderMode renderMode;
};

const Block defaultBlock = (Block){
    .index = 0,
    .colorOffset = 0,
    .pos = Vec3::zero(),
    .size = Vec3{10.0f, 2.0f, 10.0f},
    .baseCol = Color{.r = 20, .g = 20, .b = 20, .a = 255 },
    .outlineCol = WHITE,
    .movement = Movement{.speed = 0.0f, .dir = Direction::FORWARD, .axis = Axis::X },
    .removal = Removal{.timer = 0.0f, .delay = 0.0f, .scale = 1.0f, .rotation = 0.0f }
};

// helpers
Block* createMovingBlock(Game* game){
    Block target = *(game->prev);
    Vec3 pos = target.pos + Vec3::unitY() * target.size.y;
    const Axis axis  = (target.movement.axis == Axis::X) ? Axis::Z : Axis::X;
    const Direction dir = GetRandomValue(0, 1) == 0 ? Direction::FORWARD : Direction::BACKWARD;

    if(axis == Axis::X){
        pos.x = dir == Direction::FORWARD ? -MAX_MOVEMENT : MAX_MOVEMENT;
    }else{
        pos.z = dir == Direction::FORWARD ? -MAX_MOVEMENT : MAX_MOVEMENT;
    }
    size_t index = target.index + 1;
    size_t offset = target.colorOffset + index;
    float r, g, b;
    r = sinf(0.05 * offset) * 75 + 75;
    g = sinf(0.05 * offset + 2) * 75 + 75;
    b = sinf(0.05 * offset + 4) * 75 + 75;

    return new (Block){
        .index = index,
        .colorOffset = offset,
        .pos  = pos, 
        .size = target.size, 
        .baseCol    = Color{(unsigned char)r, (unsigned char)g, (unsigned char)b, .a = 255}, 
        .outlineCol = target.outlineCol,
        .movement = (Movement){
            .speed = 12.0f + index * 0.5f,
            .dir = dir,
            .axis = axis
        },
        .removal = (Removal){ .timer = 0.0f, .delay = 0.0f, .scale = 1.0f, .rotation = 0.0f }
    };
}

FallingBlock CreateFallingBlock(Vec3 pos, Vec3 size,Vec3 vel, Color base, Color outline){
    return FallingBlock{
        .active = true,
        .pos = pos,
        .size = size,
        .rot = Vec3{0, 0, 0},
        .rotSpeed = Vec3{0.8, 2.1, 0.75},
        .velocity = vel,
        .baseCol = base,
        .outlineCol = outline
    };
}

void MoveCurrentBlock(Game* game){

    Block* curr = game->curr;
    float dir = curr->movement.dir == Direction::FORWARD ? 1 : -1;
    float& axisPos = curr->movement.axis == Axis::X ? curr->pos.x : curr->pos.z;
    axisPos += dir * Time::dt * curr->movement.speed;
    if(fabs(axisPos) > MAX_MOVEMENT) {
        curr->movement.dir = (curr->movement.dir == Direction::FORWARD) ? Direction::BACKWARD : Direction::FORWARD; 
        axisPos = fmax(fmin(MAX_MOVEMENT, axisPos), -MAX_MOVEMENT);
    }
}

bool PlaceCurrentBlock(Game* game){
    Block* curr = game->curr;
    Block* prev= game->prev;

    bool isXAxis = curr->movement.axis == Axis::X;
    float& currentPosition = isXAxis ? curr->pos.x : curr->pos.z;
    float& targetPosition = isXAxis ? prev->pos.x : prev->pos.z;
    float& currSize = isXAxis ? curr->size.x : curr->size.z;
    float& targetSize = isXAxis ? prev->size.x : prev->size.z;
    
    float delta = currentPosition - targetPosition;
    float overlap = targetSize - fabs(delta);
    
    if(overlap < 0.1){
        game->state = GameState::OVER;
        game->animations.overlayAnimation.overlayType = OverlayType::GAME_OVER;
        game->animations.overlayAnimation.fadeState = FadeState::IN;
        return false;
    }
    
    bool isPerfectOverlap = fabs(delta) < 0.25f;
    float choppedSize = isPerfectOverlap ? 0.0f : currSize - overlap;

    if(isPerfectOverlap){
        overlap = targetSize;
        currSize = targetSize;
        currentPosition = targetPosition;
    }else{
        currentPosition = targetPosition + delta * 0.5f;
        currSize = overlap;
    }

    
    if(choppedSize > 0.1f){
        Vec3 choppedPos = curr->pos;
        Vec3 choppedSizeV = curr->size;
        float& chopPosAxis = isXAxis ? choppedPos.x : choppedPos.z;
        float& chopSizeAxis = isXAxis ? choppedSizeV.x : choppedSizeV.z;
        chopSizeAxis = choppedSize;
        chopPosAxis = (delta > 0 ? 1 : -1) * (currSize + choppedSize) * 0.5f + currentPosition ;

        game->fallingBlocks.push_back(
            CreateFallingBlock(
                choppedPos,
                choppedSizeV, 
                Vec3{0.0f, -10.0f, 0.0f},
                curr->baseCol, 
                curr->outlineCol
            )
        );
    }

    game->placedBlocks.push_back(*curr);
    game->prev = &(game->placedBlocks[game->placedBlocks.size() - 1]);
    game->curr = createMovingBlock(game);
    game->animations.scoreAnimation.duration = SCORE_ANIM_DURATION;
    game->animations.scoreAnimation.scale    = SCORE_ANIM_SCALE;
    return true;
}

void UpdateAnimations(Game* game){
    if(game->animations.scoreAnimation.duration > 0.0f){
        ScoreAnimation& scoreAnim = game->animations.scoreAnimation;
        scoreAnim.duration -= Time::dt;
        
        float t = scoreAnim.duration / SCORE_ANIM_DURATION;
        scoreAnim.scale = Lerp(SCORE_ANIM_SCALE, 1.0f, t);
        
        if(scoreAnim.duration < 0.0f) {
            scoreAnim.duration = 0.0f;
            scoreAnim.scale = 1.0f;
        }
    }

    if(game->animations.overlayAnimation.fadeState != FadeState::NONE){
        OverlayAnimation& overlayAnim = game->animations.overlayAnimation;
        if(overlayAnim.fadeState == FadeState::IN){
            overlayAnim.alpha += Time::dt * OVERLAY_ANIM_FADE_SPEED;
            overlayAnim.offset  = Lerp(OVERLAY_ANIM_OFFSET, 0.0f, overlayAnim.alpha);

            if(overlayAnim.alpha >= 1.0f){
                overlayAnim.alpha = 1.0f;
                overlayAnim.offset = 0.0f;
                overlayAnim.fadeState = FadeState::NONE;
            }

        }else if(overlayAnim.fadeState == FadeState::OUT){
            overlayAnim.alpha -= Time::dt * OVERLAY_ANIM_FADE_SPEED;
            overlayAnim.offset  = Lerp(OVERLAY_ANIM_OFFSET, 0.0f, overlayAnim.alpha);

            if(overlayAnim.alpha <= 0.0f){
                overlayAnim.alpha = 0.0f;
                overlayAnim.offset = OVERLAY_ANIM_OFFSET;
                overlayAnim.fadeState = FadeState::NONE;
            }
        }
    }
}

void UpdateFallingBlocks(Game* game){
    size_t len = game->fallingBlocks.size();
    for (size_t i = 0; i < len; i++)
    {
        FallingBlock& b = game->fallingBlocks.at(i);
        if(b.active){
            b.rot += b.rotSpeed * Time::dt;
            b.pos += b.velocity * Time::dt;

            if(b.pos.y < -100){
                b.active = false;
            }
        }
    }
    
}

void UpdateTowerCollapse(Game* game){
    size_t len = game->placedBlocks.size();
    for(size_t i = len - 1; i > 0; i--){
        Block& b = game->placedBlocks.at(i);
        b.removal.timer += Time::dt;
        if(b.removal.timer >= b.removal.delay){
            float t = (b.removal.timer - b.removal.delay) / COLLAPSE_ANIM_DURATION;
            float scale = 1.0f - t;

            b.removal.scale = scale;
            b.removal.rotation = t * COLLAPSE_ANIM_ROTATION_SPEED;

            if(t > COLLAPSE_ANIM_MAX_BLOCK_TIME){
                game->placedBlocks.erase(game->placedBlocks.begin() + i);
            }

        }
        
    }
}

void StartTowerCollapse(Game* game){
    size_t len = game->placedBlocks.size();
    for (size_t i = 1; i < len; i++)
    {
        Block& b = game->placedBlocks.at(i);
        b.removal.delay = (len - i) * 0.05f;
    }
    
}

// Game logic
void InitGame(Game* game){
    Block b = defaultBlock;
    float colorOffset = GetRandomValue(0, 255);
    
    float offset = colorOffset - 1;
    float r, g, bl;
    g = sinf(0.05 * offset + 2) * 75 + 75;
    bl = sinf(0.05 * offset + 4) * 75 + 75;
    r = sinf(0.05 * offset) * 75 + 75;

    b.baseCol = Color{(unsigned char)r, (unsigned char)g, (unsigned char)bl, 255};

    game->lightingShader = LoadShader("shaders/lighting_vertex.glsl", "shaders/lighting_fragment.glsl");
    game->renderMode = RenderMode::SHADOWS;
    game->cubeModel = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    game->placedBlocks.push_back(b);
    game->prev = &(game->placedBlocks[0]);
    game->state = GameState::READY;
    game->prev->colorOffset = colorOffset;
    game->animations = (Animations){
        .scoreAnimation   = (ScoreAnimation){ 
            .scale = 1.0f, 
            .duration = 0.0f 
        },
        .overlayAnimation = (OverlayAnimation){ 
            .overlayType = OverlayType::GAME_START, 
            .fadeState = FadeState::IN, 
            .alpha = 0.0f, 
            .offset = OVERLAY_ANIM_OFFSET 
        }
    };

}

void ResetGame(Game* game){
    Block b = defaultBlock;
    float colorOffset = GetRandomValue(0, 255);
    
    float offset = colorOffset - 1;
    float r, g, bl;
    g = sinf(0.05 * offset + 2) * 75 + 75;
    bl = sinf(0.05 * offset + 4) * 75 + 75;
    r = sinf(0.05 * offset) * 75 + 75;
    
    size_t len = game->fallingBlocks.size();
    for(int i = len - 1; i >= 0; i--){
        FallingBlock& b = game->fallingBlocks.at(i);
        if(!b.active) game->fallingBlocks.erase(game->fallingBlocks.begin() + i);
    }


    
    b.baseCol = Color{(unsigned char)r, (unsigned char)g, (unsigned char)bl, 255};
    game->placedBlocks.push_back(b);
    game->prev = &(game->placedBlocks[0]);
    game->state = GameState::RUNING;
    game->prev->colorOffset = colorOffset;
    game->curr = createMovingBlock(game);
}

void UpdateGameState(Game* game){
    bool inputPressed = IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    switch (game->state)
    {
        case GameState::READY:
        {
            if(inputPressed){
                game->animations.overlayAnimation.fadeState = FadeState::OUT;
                Block* b = createMovingBlock(game);
                game->state = GameState::RUNING;
                game->curr = b;
            }

        } break;
        case GameState::RUNING:
        {
            bool success = true;
            if(inputPressed){
                success = PlaceCurrentBlock(game);
            }
            if(success)
                MoveCurrentBlock(game);
            else{
                game->state = GameState::OVER;
                game->animations.overlayAnimation.overlayType = OverlayType::GAME_OVER;
                game->animations.overlayAnimation.fadeState = FadeState::IN;
            }
        } break;
        case GameState::OVER:
        {
            if(inputPressed){
                game->state = GameState::RESET;
                game->animations.overlayAnimation.overlayType = OverlayType::GAME_OVER;
                game->animations.overlayAnimation.fadeState = FadeState::OUT;
                StartTowerCollapse(game);
                
            }
        } break;
        case GameState::RESET:
        {
            UpdateTowerCollapse(game);
            if(game->placedBlocks.size() == 1){
                ResetGame(game);
            }
        }break;
        default:
            break;
    }
}

void UpdateCamera(const Game* game){
    size_t len = game->placedBlocks.size();
    CameraSystem3D::camera.position.y = Lerp(CameraSystem3D::camera.position.y,50 + 2 * len , CAMERA_SPEED);
    CameraSystem3D::CameraTarget.y = Lerp(CameraSystem3D::CameraTarget.y, 2 * len, CAMERA_SPEED);
    SetShaderValue(game->lightingShader, GetShaderLocation(game->lightingShader, "cameraPosition"), &CameraSystem3D::camera.position,SHADER_UNIFORM_VEC3);
}

// drawers
void DrawBlock(Game* game, const Block& b){
    Matrix scale, rotate, translate;
    scale = MatrixScale(b.size.x * b.removal.scale, b.size.y, b.size.z * b.removal.scale);
    rotate = MatrixRotateXYZ(Vector3{0.0f, b.removal.rotation, 0.0f});
    translate = MatrixTranslate(b.pos.x, b.pos.y, b.pos.z);
    Matrix transform = MatrixMultiply(scale, MatrixMultiply(rotate, translate));
    game->cubeModel.transform = transform;
            
    if(game->renderMode == RenderMode::WIRES){
        rlPushMatrix();
        Vec3 r1 = b.pos + Vec3::unitX();
        Vec3 r2 = b.pos + Vec3::unitY();
        Vec3 r3 = b.pos + Vec3::unitZ();
        rlRotatef(RAD2DEG * b.removal.rotation, r1.x, r1.y, r1.z);
        rlRotatef(RAD2DEG * b.removal.rotation, r2.x, r2.y, r2.z);
        rlRotatef(RAD2DEG * b.removal.rotation, r3.x, r3.y, r3.z);

        DrawCube(b.pos, b.size.x, b.size.y, b.size.z, b.baseCol);
        DrawCubeWires(b.pos, b.size.x, b.size.y, b.size.z, b.outlineCol);
        rlPopMatrix();
    }else if(game->renderMode == RenderMode::SHADOWS && IsShaderValid(game->lightingShader)){
        Vector4 col = ColorNormalize(b.baseCol);
        Vector3* c = new Vector3();
        c->x = col.x; c->y = col.y; c->z = col.z;
        SetShaderValue(
            game->lightingShader, 
            GetShaderLocation(game->lightingShader, "blockColor"), 
            c, 
            SHADER_UNIFORM_VEC3
        );
        game->cubeModel.materials[0].shader = game->lightingShader;
        DrawModel(game->cubeModel, (Vector3){0}, 1.0f, b.baseCol);
        delete c;        
    }

}

void DrawPlacedBlocks(Game* game){
    for(const auto& b : game->placedBlocks){
        DrawBlock(game, b);
    }
}

void DrawCurrentBlock(Game* game){
    DrawBlock(game, *(game->curr));
}

void DrawFallingBlocks(Game* game){
    size_t len = game->fallingBlocks.size();
    for (size_t i = 0; i < len; i++)
    {
        FallingBlock& b = game->fallingBlocks.at(i);
        if(b.active){

            
            if(game->renderMode == RenderMode::WIRES){
                rlPushMatrix();
                Vec3 r1 = b.pos + Vec3::unitX();
                Vec3 r2 = b.pos + Vec3::unitY();
                Vec3 r3 = b.pos + Vec3::unitZ();
                rlRotatef(RAD2DEG * b.rot.x, r1.x, r1.y, r1.z);
                rlRotatef(RAD2DEG * b.rot.y, r2.x, r2.y, r2.z);
                rlRotatef(RAD2DEG * b.rot.z, r3.x, r3.y, r3.z);

                DrawCube(b.pos, b.size.x, b.size.y, b.size.z, b.baseCol);
                DrawCubeWires(b.pos, b.size.x, b.size.y, b.size.z, b.outlineCol);
                rlPopMatrix();
            }else if(game->renderMode == RenderMode::SHADOWS && IsShaderValid(game->lightingShader)){
                Matrix scale, rotate, translate;
                scale = MatrixScale(b.size.x, b.size.y, b.size.z);
                rotate = MatrixRotateXYZ((Vector3)b.rot);
                translate = MatrixTranslate(b.pos.x, b.pos.y, b.pos.z);
                Matrix transform = MatrixMultiply(scale, MatrixMultiply(rotate, translate));
                game->cubeModel.transform = transform;
                Vector4 col = ColorNormalize(b.baseCol);
                Vector3* c = new Vector3();
                c->x = col.x; c->y = col.y; c->z = col.z;
                SetShaderValue(
                    game->lightingShader, 
                    GetShaderLocation(game->lightingShader, "blockColor"), 
                    c, 
                    SHADER_UNIFORM_VEC3
                );
                game->cubeModel.materials[0].shader = game->lightingShader;
                DrawModel(game->cubeModel, (Vector3){0}, 1.0f, b.baseCol);
                delete c;
            }
        }
    }
    
}

void DrawGameOverlay(Game* game){


    if(game->state == GameState::READY || 
        (game->animations.overlayAnimation.overlayType == OverlayType::GAME_START && game->animations.overlayAnimation.fadeState != FadeState::NONE))
    {
        const char* title = "START GAME";
        int fontSize = 60;
        int textSize    = MeasureText(title, fontSize);
        int x = (GetScreenWidth() - textSize) * 0.5f , y = 100 + game->animations.overlayAnimation.offset;
        Color textColor = Fade(DARKGRAY, game->animations.overlayAnimation.alpha);  
        DrawText(title, x, y, fontSize, textColor);

        const char* subtitle = "Click or press space to start";
        const int subtitleFontSize = 30;
        const int subtitleTextSize = MeasureText(subtitle, subtitleFontSize);
        const int subtitleX = (GetScreenWidth() - subtitleTextSize) * 0.5f;
        const int subtitleY = 220 + game->animations.overlayAnimation.offset;
        const Color subtitleColor = Fade(GRAY, game->animations.overlayAnimation.alpha);
        DrawText(subtitle, subtitleX, subtitleY, subtitleFontSize, subtitleColor);

    } 
    if(game->state == GameState::RUNING || game->state == GameState::OVER)
    {
        const char* title = TextFormat("%d" , game->placedBlocks.size() - 1);
        int fontSize     = 120 * game->animations.scoreAnimation.scale;
        int textSize     = MeasureText(title, fontSize);
        int x = (GetScreenWidth() - textSize) * 0.5f , y  = 280;
        Color textColor  = DARKGRAY; 

        DrawText(title, x, y, fontSize, textColor);
    } 
    if(game->state == GameState::OVER || 
        (game->animations.overlayAnimation.overlayType == OverlayType::GAME_OVER 
            && game->animations.overlayAnimation.fadeState != FadeState::NONE)
      )
    {
        const char* title= "Game Over";
        int fontSize     = 60;
        int textSize     = MeasureText(title, fontSize);
        int x            = (GetScreenWidth() - textSize) * 0.5f ;
        int y            = 100 + game->animations.overlayAnimation.offset;
        Color textColor  = Fade(RED, game->animations.overlayAnimation.alpha);       

        DrawText(title, x, y, fontSize, textColor);

        const char* subtitle = "Click or press space to restart";
        const int subtitleFontSize = 30;
        const int subtitleTextSize = MeasureText(subtitle, subtitleFontSize);
        const int subtitleX = (GetScreenWidth() - subtitleTextSize) * 0.5f;
        const int subtitleY = 220 + game->animations.overlayAnimation.offset;
        const Color subtitleColor = Fade(Color{150, 70, 70, 255}, game->animations.overlayAnimation.alpha);
        DrawText(subtitle, subtitleX, subtitleY, subtitleFontSize, subtitleColor);

    }

    
    DrawFPS(10, 10);
}

void DrawDebag(Game* game){    Vec3 currPos = game->curr->pos;
    DrawLine3D(Vector3(currPos - Vec3::unitX() * MAX_MOVEMENT), Vector3(currPos + Vec3::unitX() * MAX_MOVEMENT), RED);
    DrawLine3D(Vector3(currPos - Vec3::unitZ() * MAX_MOVEMENT), Vector3(currPos + Vec3::unitZ() * MAX_MOVEMENT), GREEN);
}


// ##########################################
//               API
// ##########################################

Game* Setup(){
    Game* game = new Game();
    InitGame(game);
    return game;
}

void UpdateGame(Game* game){

    UpdateGameState(game);

    UpdateAnimations(game);

    UpdateCamera(game);

    UpdateFallingBlocks(game);

    if(IsKeyPressed(KEY_W)){
        game->renderMode = RenderMode::WIRES;
    }else if (IsKeyPressed(KEY_S)){
        game->renderMode = RenderMode::SHADOWS;
    }
}

void DrawGame(Game* game){
    DrawPlacedBlocks(game);
    if(game->state == GameState::RUNING){
        DrawCurrentBlock(game);
    }
    DrawFallingBlocks(game);
    // DrawDebag(*game);
}

void TerminateGame(Game* game){
    game->fallingBlocks.clear();
    game->placedBlocks.clear();
    delete game;
}


/*
Suggested settings:
---------------------------------------
#define WIDTH 1000
#define HEIGHT 1500
namespace CameraSystem3D{
    static Camera camera = {0};
    Vector3 CameraTarget = {0,0,0};
    Vector3 cameraOffset{50, 50, 50};
    float cameraSpeed = 10.0f;

    static void initCamera()
    {
        camera.fovy = 60.0f;
        camera.position = Vector3{0, 3.0f, 4};
        camera.projection = CAMERA_ORTHOGRAPHIC;
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
 your entry point should look like:
 int main(void)
{
    Init(); // init raylib engine and stuff not related to game

    Game* game = Setup(); // setup the game and get a game handler

    while (!WindowShouldClose())
    {
        // updates 
        Update(); // not related to game (camera, time etc)

        UpdateGame(game); // game related updates
        
        BeginDrawing(); 
            ClearBackground(BACKGROUND_COLOR ); // suggested background color #define BACKGROUND_COLOR Color{ .r = 210, .g = 200, .b = 190, .a = 255}

            BeginMode3D(CameraSystem3D::camera);

            DrawGame(game); // game render in 3D 
                
            EndMode3D();

            DrawGameOverlay(game); // game overlay render like text etc
            
        EndDrawing();

    }

    TerminateGame(game); // free resources of game (also delete game itself)

    Shutdown();

    return EXIT_SUCCESS;
}
*/