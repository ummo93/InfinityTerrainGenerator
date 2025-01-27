#include "world.h"
#include "rcamera.h"
#include <iostream>

#define log(...) std::cout << std::format(__VA_ARGS__) << '\n'

void UpdateCameraCustom(Camera& camera)
{
    // int cameraMode = CAMERA_FIRST_PERSON;
    // UpdateCamera(&camera, cameraMode); // Simple default update camera
    // 
    // Camera PRO usage example (EXPERIMENTAL)
    // This new camera function allows custom movement/rotation values to be directly provided
    // as input parameters, with this approach, rcamera module is internally independent of raylib inputs
    UpdateCameraPro(
        &camera,
        {
            (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) * 5.1f -        // Move forward-backward
            (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) * 5.1f,
            (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) * 5.1f -     // Move right-left
            (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) * 5.1f,
            0.0f                                                    // Move up-down
        },
    {
        GetMouseDelta().x * 0.05f,                              // Rotation: yaw
        GetMouseDelta().y * 0.05f,                              // Rotation: pitch
        0.0f                                                    // Rotation: roll
    },
        0 // zoom, f.ex: GetMouseWheelMove() * 2.0f
    );
}



//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 800;
    
    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera first person");
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    // Define the camera to look into our 3d world (position, target, up vector)
    Camera camera = { 0 };
    camera.position = { 0.1f, 10.f, 0.1f };     // Camera position
    camera.target = { 0, 10.f, 0 };             // Camera looking at point
    camera.up = { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                       // Camera field-of-view Y   
    camera.projection = CAMERA_PERSPECTIVE;    // Camera projection type

    DisableCursor();                    // Limit cursor to relative movement inside the window
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second

    //--------------------------------------------------------------------------------------

    InfinityWorld world = { 12456 };

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        auto localArea = world.GetArea(camera.position);
        world.LoadNeighbours(*localArea);

        // Update
        //----------------------------------------------------------------------------------
        // Update camera computes movement internally depending on the camera mode
        // Some default standard keyboard/mouse inputs are hardcoded to simplify use
        // For advanced camera controls, it's recommended to compute camera movement manually

        UpdateCameraCustom(camera); // Update camera
        //----------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(SKYBLUE);

        BeginMode3D(camera);

        world.EachArea([](auto area) {
            DrawModel(*area->model, { area->location.x * Area::scaleOffset, -150, area->location.y * Area::scaleOffset }, CHUNK_SIZE, YELLOW);
        });

        EndMode3D();

        DrawFPS(80, 20);
        EndDrawing();

        world.UnloadFarAreas(camera.position);
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

