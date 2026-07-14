#include <stdio.h>
#include "raylib.h"

#include "player.h"

int main() {
    InitWindow(1280, 960, "DeltaConnected PC");
    SetTargetFPS(60);

    const Image floradinnTexture = LoadImage("C:/Users/rohan/OneDrive/Pictures/Screenshots/Screenshot 2026-07-13 155539.png");

    Entity testMonster;
    testMonster.x = 30; testMonster.y = 30;
    testMonster.sprite = LoadTextureFromImage(floradinnTexture);

    while (!WindowShouldClose()) {

        BeginDrawing();
            drawEntity(testMonster);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}