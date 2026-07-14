#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include "dep/tinyfiledialogs.h"

#include "builder_parts.h"


const int HITBOX_LIMIT = 256;
const enum EditingMode {
    SELECTION,
    HITBOX,
};
const KeyboardKey modeKeys[2] = { KEY_ONE, KEY_TWO };
enum EditingMode currentMode = HITBOX;


Hitbox* hitboxes = NULL;
int hitboxesFull = 0;

Hitbox tempBox;
bool drawingBox = false;

void init(void) {
    camPos = Vector2Zero();

    hitboxes = (Hitbox *)calloc(HITBOX_LIMIT, sizeof(Hitbox));
    tempBox.typ = (enum HitboxType)0;
}

void hitboxMakeCheck(Vector2 mP, Vector2 mD) {
    
    Vector2 mC = Vector2Add(mP, camPos);

    // press
    if (IsMouseButtonPressed(0)) {
        if (hitboxesFull >= HITBOX_LIMIT) {
            tinyfd_messageBox("Sorry... Too Many Hitboxes!!!", "Only a maximum of 256 hitboxes are allowed in one file. Please be more efficient!", "ok", "warning", 1);
            return;
        }

        tempBox.x = mC.x;
        tempBox.y = mC.y;
        drawingBox = true;
    }
    
    // dragging
    if (Vector2LengthSqr(mD) > 0) if (drawingBox) {
        tempBox.w = mC.x - tempBox.x;
        tempBox.h = mC.y - tempBox.y;
    }
    
    // release
    if (IsMouseButtonReleased(0)) if (drawingBox) {
        hitboxes[hitboxesFull++] = canonicalHitbox(tempBox);
        printf("\n%f, %f, %f, %f", tempBox.x, tempBox.y, tempBox.w, tempBox.h);
        tempBox.w = 0; tempBox.h = 0;
        drawingBox = false;
    }
}

void drawHitbox(Hitbox h, Color c) {
    DrawRectangleLinesEx(
        hitboxScreenRect(canonicalHitbox(h)),
        5, c
    );
}