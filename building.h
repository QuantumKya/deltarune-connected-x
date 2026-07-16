#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include "dep/tinyfiledialogs.h"

#include "builder_parts.h"


const int HITBOX_LIMIT = 128;
const int IMAGE_LIMIT = 128;
enum EditingMode {
    SELECTION,
    MAKE_BOX,
    MAKE_IMAGE
};
const KeyboardKey modeKeys[3] = { KEY_ONE, KEY_TWO, KEY_THREE };
enum EditingMode currentMode = SELECTION;


Color* selectionOverColor = NULL;
void** selectedObjects = NULL;
int selectCount = 0;


Hitbox* hitboxes = NULL;
int hitboxesFull = 0;
ImageBox* imageboxes = NULL;
int imageboxesFull = 0;

Hitbox tempBox;
ImageBox tempIB;
bool drawingBox = false;

void init(void) {
    hitboxes = (Hitbox*)calloc(HITBOX_LIMIT, sizeof(Hitbox));
    imageboxes = (ImageBox*)calloc(IMAGE_LIMIT, sizeof(ImageBox));
    selectedObjects = (void**)calloc(HITBOX_LIMIT + IMAGE_LIMIT, sizeof(void*));
    
    selectionOverColor = (Color*)calloc(1, sizeof(Color));
    *selectionOverColor = ColorAlpha(BLUE, 0.5);
    tempBox.typ = (enum HitboxType)0;
    
    camPos = Vector2Zero();
}
void clean(void) {
    free(hitboxes);
    free(imageboxes);
    free(selectedObjects);
    free(selectionOverColor);
}

void hitboxMakeCheck(const Vector2 mP, const Vector2 mD) {
    
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
void hitboxBreakCheck(const Vector2 mP) {
    for (int i = 0; i < hitboxesFull; i++) {
        if (hoveringEdge(hitboxes[i], mP, 5)) {
            deleteFromArray(hitboxes, &hitboxesFull, sizeof(Hitbox), i, 1);
            break;
        }
    }
}


void imageMakeCheck(const Vector2 mP) {
    
    Vector2 mC = Vector2Add(mP, camPos);

    // press
    if (IsMouseButtonPressed(0)) {
        if (imageboxesFull >= IMAGE_LIMIT) {
            tinyfd_messageBox("Sorry... Too Many Images!!!", "Only a maximum of 256 images are allowed in one file. Please be more efficient!", "ok", "warning", 1);
            return;
        }

        const char* fileSuffixes[3] = { "*.png", "*.jpg", "*.bmp" };
        const char* filePath = tinyfd_openFileDialog(
            "Select an image!", "",
            3, fileSuffixes,
            "Image Files (.png, .jpg, .bmp)",
            0
        );
        if (!filePath) return;
        SetMousePosition(mP.x, mP.y);
        

        Image img = LoadImage(filePath);
        if (img.data == NULL) return;
        
        ImageBox nib;
        nib.tex = LoadTextureFromImage(img);
        nib.x = mC.x; nib.y = mC.y;
        nib.w = img.width; nib.h = img.height;
        
        Rectangle srcR; srcR.x = 0; srcR.y = 0;
        srcR.width = img.width; srcR.height = img.height;
        nib.src = srcR;

        imageboxes[imageboxesFull++] = nib;
    }
}
void imageBreakCheck(const Vector2 mP) {
    for (int i = 0; i < imageboxesFull; i++) if (hoveringRect(imageboxes[i], mP)) {
        deleteFromArray(imageboxes, &imageboxesFull, sizeof(ImageBox), i, 1);
        break;
    }
}



void drawHitbox(const Hitbox h, const Color c) {
    DrawRectangleLinesEx(
        screenHitboxRect(h),
        5, c
    );
}
void drawImageBox(const ImageBox ib, const Color tint) {
    DrawTexturePro(
        ib.tex, ib.src,
        screenImageBoxRect(ib),
        Vector2Zero(), 0,
        tint
    );
}



/*
const char* getLevelFile(void) {
    const int bytesPerHitbox = 4*8 + 2; // ",s" + 4x ",0000.00"

    //                                          "hb"   +    hitbox encoding   +  "\n"
    const char* hitboxStrs = (const char*)calloc(2 + hitboxesFull*bytesPerHitbox + 1, sizeof(char));
    strncpy(hitboxStrs, "hb", 2);

    for (int i = 0; i < hitboxesFull; i++) {
        const Hitbox h = hitboxes[i];
        char type = "";
        if (h.typ == SOLID) type = "s"; if (h.typ == TRANSITION) type = "t"; if (h.typ == INTERACT) type = "i";

        const char* coordinates = TextFormat(",%s,%04.02f,%04.02f,%04.02f,%04.02f", type, h.x, h.y, h.w, h.h);
        strncpy(hitboxStrs + i*bytesPerHitbox, coordinates, bytesPerHitbox);

        free(coordinates);
    }

    

    

    for (int i = 0; i < imageboxesFull; i++) {
        const ImageBox ib = imageboxes[i];
        // this is unfinished
    }



    // after combining and saving
    free(hitboxStrs);
}
*/