#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>

#include "building.h"


int main(void) {
    InitWindow(1280, 960, "DeltaConnPC Area Builder");
    SetTargetFPS(30);

    init();
    bool movingCam = false;

    while (!WindowShouldClose()) {

        for (int i = 0; i < sizeof(modeKeys) / sizeof(KeyboardKey); i++) {
            if (IsKeyPressed(modeKeys[i])) {
                currentMode = (enum EditingMode)i;
                break;
            }
        }

        Vector2 mD = GetMouseDelta();
        Vector2 mP = GetMousePosition();

        if (currentMode == HITBOX) hitboxMakeCheck(mP, mD);

        
        // camera moving
        if (IsMouseButtonPressed(1)) movingCam = true;
        if (Vector2LengthSqr(mD) > 0) if (movingCam) camPos = Vector2Subtract(camPos, mD);
        if (IsMouseButtonReleased(1)) movingCam = false;


        BeginDrawing();
            ClearBackground(DARKBLUE);

            for (int i = 0; i < hitboxesFull; i++)
                drawHitbox(hitboxes[i], hitboxColors[hitboxes[i].typ]);

            if (currentMode == SELECTION);
                
            if (currentMode == HITBOX)
                drawHitbox(tempBox, hitboxColors[tempBox.typ]);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}