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

        // state-changing with the keyboard
        for (int i = 0; i < sizeof(modeKeys) / sizeof(KeyboardKey); i++) {
            if (IsKeyPressed(modeKeys[i])) {
                currentMode = (enum EditingMode)i;
                break;
            }
        }
        for (int i = 0; i < sizeof(hitboxTypeKeys) / sizeof(KeyboardKey); i++) {
            if (IsKeyPressed(hitboxTypeKeys[i])) {
                tempBox.typ = (enum HitboxType)i;
                break;
            }
        }

        Vector2 mD = GetMouseDelta();
        Vector2 mP = GetMousePosition();

        if (currentMode == MAKE_BOX) {
            if (IsMouseButtonDown(1)) hitboxBreakCheck(mP);
            
            hitboxMakeCheck(mP, mD);
        }

        if (currentMode == MAKE_IMAGE) {
            if (IsMouseButtonDown(1)) imageBreakCheck(mP);

            imageMakeCheck(mP);
        }

        if (currentMode == SELECTION) {
            if (IsMouseButtonPressed(0)) {
                for (int i = 0; i < selectCount; i++) selectedObjects[i] = NULL;
                selectCount = 0;
            }
            if (IsMouseButtonDown(0)) {
                for (int i = 0; i < hitboxesFull; i++) {
                    Hitbox hi = hitboxes[i];
                    if (hoveringEdge(hi, mP, 5)) {
                        addToArrayUnique(selectedObjects, &selectCount, &hi);
                        break;
                    }
                }
                for (int i = 0; i < imageboxesFull; i++) {
                    ImageBox ib = imageboxes[i];
                    if (hoveringRect(ib, mP)) {
                        addToArrayUnique(selectedObjects, &selectCount, &ib);
                        break;
                    }
                }
            }
        }

        
        // camera moving
        if (IsMouseButtonPressed(2)) movingCam = true;
        if (Vector2LengthSqr(mD) > 0) if (movingCam) camPos = Vector2Subtract(camPos, mD);
        if (IsMouseButtonReleased(2)) movingCam = false;


        BeginDrawing();
            ClearBackground(DARKBLUE);

            for (int i = 0; i < imageboxesFull; i++)
                drawImageBox(imageboxes[i], WHITE);
            for (int i = 0; i < hitboxesFull; i++)
                drawHitbox(hitboxes[i], hitboxColors[hitboxes[i].typ]);

            
            if (currentMode == SELECTION) {
                for (int i = 0; i < selectCount; i++) {
                    Hitbox* h = (Hitbox*)(selectedObjects[i]);
                    ImageBox* ib = (ImageBox*)(selectedObjects[i]);
                    if (h != NULL) drawHitbox(*h, *selectionOverColor);
                    if (ib != NULL) drawImageBox(*ib, *selectionOverColor);
                }

                DrawText("SELECTION MODE", 10, 10, 30, WHITE);
            }
                
            if (currentMode == MAKE_BOX) {
                drawHitbox(tempBox, hitboxColors[tempBox.typ]);

                DrawText("HITBOX MODE", 10, 10, 30, WHITE);
            }

            if (currentMode == MAKE_IMAGE) {
                DrawText("IMAGEBOX MODE", 10, 10, 30, WHITE);
            }
            
        EndDrawing();
    }
    CloseWindow();

    clean();
    return 0;
}