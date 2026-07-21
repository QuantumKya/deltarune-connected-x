#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>

#include "building.h"
#include "file_business.h"


int main(void) {
    InitWindow(1280, 960, "DeltaConnPC Area Builder");
    SetTargetFPS(30);

    StringList fp;
    if (!initLoadMemory(&fp)) return 1;

    Font daFont = LoadFont("dep/determination-8-bit-oper-jve-mono.otf");

    init();
    bool movingCam = false;

    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_O)) exportLevelFile(&fp, hitboxes, hitboxesFull, imageboxes, imageboxesFull);
        if (IsKeyPressed(KEY_L)) {
            const char* fs[1] = { "*.txt" };
            const char* fn = tinyfd_openFileDialog(
                "Open Level File", "",
                1, fs,
                "Text Files (.txt)",
                0
            );
            if (fn != NULL) readLevelFile(fn, hitboxes, &hitboxesFull, imageboxes, &imageboxesFull, &fp);
        }

        // state-changing with the keyboard
        for (int i = 0; i < sizeof(toolModeKeys) / sizeof(KeyboardKey); i++) {
            if (IsKeyPressed(toolModeKeys[i])) {
                currentTool = (enum ToolMode)i;
                break;
            }
        }
        for (int i = 0; i < sizeof(subTypeKeys) / sizeof(KeyboardKey); i++) {
            if (IsKeyPressed(subTypeKeys[i])) {
                switch (currentTool) {
                    case EDIT:
                        currentEditMode = (enum EditingMode)i; break;
                    case MAKE_BOX:
                        tempBox.typ = (enum HitboxType)i; break;
                    case MAKE_IMAGE:
                        break;
                    default: break;
                }
                break;
            }
        }



        Vector2 mD = GetMouseDelta();
        Vector2 mP = GetMousePosition();


        // box making
        if (currentTool == MAKE_BOX) {
            if (IsMouseButtonDown(1)) hitboxBreakCheck(mP);
            
            hitboxMakeCheck(mP, mD, fp);
        }

        if (currentTool == MAKE_IMAGE) {
            if (IsMouseButtonDown(1)) imageBreakCheck(mP);

            imageMakeCheck(mP, &fp);
        }



        // all the editing
        if (currentTool == EDIT && currentEditMode == STRETCH) {
            stretchCheck(mP, mD);
        }

        if (currentTool == EDIT && currentEditMode == DRAG) {
            dragCheck(mP, mD);
        }

        if (currentTool == EDIT && currentEditMode == SELECT) {
            if (IsMouseButtonPressed(0)) emptyArrayUnique(selectedObjects, &selectCount);
            if (IsMouseButtonDown(0)) {
                for (int i = 0; i < hitboxesFull; i++) {
                    Hitbox hi = hitboxes[i];
                    if (hoveringEdgeH(hi, mP, 5)) {
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
        else if (Vector2LengthSqr(mD) > 0) if (movingCam) camPos = Vector2Subtract(camPos, mD);
        if (IsMouseButtonReleased(2)) movingCam = false;
        


        BeginDrawing();
            ClearBackground(DARKBLUE);

            for (int i = 0; i < imageboxesFull; i++)
                drawImageBox(imageboxes[i], WHITE);
            for (int i = 0; i < hitboxesFull; i++)
                drawHitbox(hitboxes[i], hitboxColors[hitboxes[i].typ], &fp, &daFont);

            
            if (currentTool == EDIT) {
                for (int i = 0; i < selectCount; i++) {
                    Hitbox* h = (Hitbox*)(selectedObjects[i]);
                    ImageBox* ib = (ImageBox*)(selectedObjects[i]);
                    if (h != NULL) drawHitbox(*h, *selectionOverColor, &fp, &daFont);
                    if (ib != NULL) drawImageBox(*ib, *selectionOverColor);
                }
            }
                
            if (currentTool == MAKE_BOX)
                drawHitbox(tempBox, hitboxColors[tempBox.typ], &fp, &daFont);



            DrawText(TextFormat("TOOL: %s", toolModeNames[currentTool]), 10, 10, 30, WHITE);

            int currentSubMode = 0;
            switch (currentTool) {
                case EDIT: currentSubMode = currentEditMode; break;
                case MAKE_BOX: currentSubMode = tempBox.typ; break;
                case MAKE_IMAGE: currentSubMode = 0; break;
                default: currentSubMode = 0; break;
            }
            DrawText(TextFormat("MODE: %s", subModeNames[currentTool][currentSubMode]), 10, 40, 30, WHITE);
            
        EndDrawing();
    }
    CloseWindow();

    clean();
    cleanLoadMemory(&fp);
    return 0;
}