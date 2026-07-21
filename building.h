#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>
#include <raymath.h>
#include "dep/tinyfiledialogs.h"

#include "builder_parts.h"


enum ToolMode {
    EDIT,
    MAKE_BOX,
    MAKE_IMAGE,
};
enum ToolMode currentTool = EDIT;

enum EditingMode {
    SELECT,
    DRAG,
    STRETCH
};
enum EditingMode currentEditMode = SELECT;
const KeyboardKey toolModeKeys[] = { KEY_ONE, KEY_TWO, KEY_THREE };
const char* toolModeNames[] = { "EDITING", "HITBOXES", "IMAGEBOXES" };
const KeyboardKey subTypeKeys[] = { KEY_Q, KEY_W, KEY_E };
const char* subModeNames[][3] = {
    { "SELECT", "DRAG", "STRETCH" },
    { "SOLID", "TRANSITION", "INTERACT" },
    { "BACKGROUND", "FOREGROUND" }
};


Color* selectionOverColor = NULL;
void** selectedObjects = NULL;
int selectCount = 0;


Hitbox* hitboxes = NULL;
int hitboxesFull = 0;
ImageBox* imageboxes = NULL;
int imageboxesFull = 0;

int transitionBoxes = 0;

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

void hitboxMakeCheck(const Vector2 mP, const Vector2 mD, StringList fp) {
    
    Vector2 mC = Vector2Add(mP, camPos);

    // press
    if (IsMouseButtonPressed(0)) {
        if (hitboxesFull >= HITBOX_LIMIT) {
            tinyfd_messageBox("Sorry... Too Many Hitboxes!!!", "Only a maximum of 128 hitboxes are allowed in one file. Please be more efficient!", "ok", "warning", 1);
            return;
        }

        tempBox.x = mC.x;
        tempBox.y = mC.y;
        drawingBox = true;
    }
    
    // dragging
    else if (Vector2LengthSqr(mD) > 0) if (drawingBox) {
        tempBox.w = mC.x - tempBox.x;
        tempBox.h = mC.y - tempBox.y;
    }
    
    // release
    if (IsMouseButtonReleased(0)) if (drawingBox) {
        if (tempBox.typ == INTERACT) {
            const char* msg = tinyfd_inputBox("Enter Interact Message", "Enter Interact Message: ", "An empty box. It looks like it could use some words.");
            if (msg != NULL) tempBox.strIndex = max(0, addString(&fp, msg));
        }
        if (tempBox.typ == TRANSITION) {
            const char* fileSuffix[1] = { "*.txt" };
            char levelpath[256];
            strcat(levelpath, tinyfd_openFileDialog("Choose Level File", "", 1, fileSuffix, "Text Files (.txt)", 0));
            strcat(levelpath, TextFormat("%01i", transitionBoxes++));
            addString(&fp, levelpath);
        }

        hitboxes[hitboxesFull++] = canonicalHitbox(tempBox);
        printf("\n%f, %f, %f, %f", tempBox.x, tempBox.y, tempBox.w, tempBox.h);
        tempBox.w = 0; tempBox.h = 0;
        drawingBox = false;
    }
}
void hitboxBreakCheck(const Vector2 mP) {
    for (int i = 0; i < hitboxesFull; i++) {
        if (hoveringEdgeH(hitboxes[i], mP, 5)) {
            deleteFromArray(hitboxes, &hitboxesFull, sizeof(Hitbox), i, 1);
            break;
        }
    }
}


void imageMakeCheck(const Vector2 mP, StringList* fp) {
    
    Vector2 mC = Vector2Add(mP, camPos);

    // press
    if (IsMouseButtonPressed(0)) {
        if (imageboxesFull >= IMAGE_LIMIT) {
            tinyfd_messageBox("Sorry... Too Many Images!!!", "Only a maximum of 64 images are allowed in one file. Please be more efficient!", "ok", "warning", 1);
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

        char* copiedPath = (char*)malloc(strlen(filePath) + 1);
        if (!copiedPath) return;
        strcpy(copiedPath, filePath);
        int fileIndex = addToArrayUnique(
            (void**)fp->paths,
            &fp->pathCount,
            copiedPath
        );
        if (fileIndex < 0) {
            free(copiedPath);
            return;
        }
        

        ImageBox nib;
        nib.fileIndex = fileIndex;
        nib.ts.tex = LoadTextureFromImage(img);
        nib.x = mC.x; nib.y = mC.y;
        nib.w = img.width; nib.h = img.height;
        
        Rectangle srcR; srcR.x = 0; srcR.y = 0;
        srcR.width = img.width; srcR.height = img.height;
        nib.ts.src = srcR;

        imageboxes[imageboxesFull++] = nib;
    }
}
void imageBreakCheck(const Vector2 mP) {
    for (int i = 0; i < imageboxesFull; i++) if (hoveringRect(imageboxes[i], mP)) {
        deleteFromArray(imageboxes, &imageboxesFull, sizeof(ImageBox), i, 1);
        break;
    }
}



void* editObj = NULL;
bool editWhich = false;

void dragCheck(const Vector2 mP, const Vector2 mD) {
    
    const Vector2 mC = Vector2Add(mP, camPos);

    // press
    if (IsMouseButtonPressed(0)) {
        for (int i = 0; i < hitboxesFull; i++) {
            if (hoveringEdgeH(hitboxes[i], mP, 5)) {
                editObj = &hitboxes[i];
                editWhich = false;
                break;
            }
        }
        for (int i = 0; i < imageboxesFull; i++) {
            if (hoveringRect(imageboxes[i], mP)) {
                editObj = &imageboxes[i];
                editWhich = true;
                break;
            }
        }
        if (editObj == NULL) return;
    }

    // dragging
    if (IsMouseButtonDown(0)) {

        bool autoCase = false;
        if (selectCount <= 0) { autoCase = true;
            selectedObjects[0] = editObj;
            selectCount = 1;
        }

        for (int i = 0; i < selectCount; i++) {
            void* objP = selectedObjects[i];
            if (objP == NULL) continue;
            
            if (editWhich) {
                ImageBox* ib = (ImageBox*)objP;
                ib->x += mD.x;
                ib->y += mD.y;
            }
            else {
                Hitbox* hb = (Hitbox*)objP;
                hb->x += mD.x;
                hb->y += mD.y;
            }

        }

        if (autoCase) emptyArrayUnique(selectedObjects, &selectCount);
    }

    // release
    if (IsMouseButtonReleased(0)) {
        editObj = NULL;
    }

}

Vector2 stretchCorner;

void stretchCheck(const Vector2 mP, const Vector2 mD) {

    const Vector2 mC = Vector2Add(mP, camPos);

    // press
    if (IsMouseButtonPressed(0)) {
        Rectangle objectBox;
        for (int i = 0; i < hitboxesFull; i++) {
            if (hoveringEdgeH(hitboxes[i], mP, 5)) {
                editObj = &hitboxes[i]; editWhich = false;
                objectBox = hitboxRect(hitboxes[i]);
                break;
            }
        }
        for (int i = 0; i < imageboxesFull; i++) {
            if (hoveringEdgeIB(imageboxes[i], mP, 5)) {
                editObj = &imageboxes[i]; editWhich = true;
                objectBox = imageBoxRect(imageboxes[i]);
                break;
            }
        }
        if (editObj == NULL) return;


        Vector2 boxCenter;
        boxCenter.x = objectBox.x + objectBox.width/2;
        boxCenter.y = objectBox.y + objectBox.height/2;

        const float amountX = (mC.x - boxCenter.x)*2/objectBox.width;
        const float amountY = (mC.y - boxCenter.y)*2/objectBox.height;
        stretchCorner.x = roundf(amountX);
        stretchCorner.y = roundf(amountY);
    }

    // dragging
    if (IsMouseButtonDown(0)) {

        bool autoCase = false;
        if (selectCount <= 0) { autoCase = true;
            selectedObjects[0] = editObj;
            selectCount = 1;
        }

        for (int i = 0; i < selectCount; i++) {
            void* objP = selectedObjects[i];
            if (objP == NULL) continue;
            
            if (editWhich) {
                ImageBox* ib = (ImageBox*)objP;
                if (stretchCorner.x < 0) ib->x += mD.x;
                if (stretchCorner.y < 0) ib->y += mD.y;
                ib->w += stretchCorner.x * mD.x;
                ib->h += stretchCorner.y * mD.y;
            }
            else {
                Hitbox* hb = (Hitbox*)objP;
                if (stretchCorner.x < 0) hb->x += mD.x;
                if (stretchCorner.y < 0) hb->y += mD.y;
                hb->w += stretchCorner.x * mD.x;
                hb->h += stretchCorner.y * mD.y;
            }

        }

        if (autoCase) emptyArrayUnique(selectedObjects, &selectCount);
    }

    // release
    if (IsMouseButtonReleased(0)) {
        editObj = NULL;
    }
}