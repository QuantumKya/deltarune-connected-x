#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>
#include <raymath.h>
#include "dep/tinyfiledialogs.h"

#include "builder_parts.h"

#define sgn(x) ((x>0) - (x<0));


const int HITBOX_LIMIT = 128;
const int IMAGE_LIMIT = 128;
enum EditingMode {
    SELECT,
    EDIT,
    MAKE_BOX,
    MAKE_IMAGE,
};
const KeyboardKey modeKeys[] = { KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR };
enum EditingMode currentMode = SELECT;


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
    else if (Vector2LengthSqr(mD) > 0) if (drawingBox) {
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
        if (hoveringEdgeH(hitboxes[i], mP, 5)) {
            deleteFromArray(hitboxes, &hitboxesFull, sizeof(Hitbox), i, 1);
            break;
        }
    }
}


void imageMakeCheck(const Vector2 mP, FilePaths* fp) {
    
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

        char* copiedPath = malloc(strlen(filePath) + 1);
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



Vector2 stretchCorner;
bool stretchWhich = false;
int stretchIndex = 0;

void stretchCheck(const Vector2 mP, const Vector2 mD) {

    const Vector2 mC = Vector2Add(mP, camPos);

    // press
    if (IsMouseButtonPressed(0)) {
        for (int i = 0; i < hitboxesFull; i++) {
            if (hoveringEdgeH(hitboxes[i], mP, 5)) {
                stretchWhich = false; stretchIndex = i;
                break;
            }
        }
        for (int i = 0; i < imageboxesFull; i++) {
            if (hoveringEdgeIB(imageboxes[i], mP, 5)) {
                stretchWhich = true; stretchIndex = i;
                break;
            }
        }

        const Rectangle objectBox = stretchWhich
            ? imageBoxRect(imageboxes[stretchIndex])
            : hitboxRect(hitboxes[stretchIndex]);

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

        switch ((int)stretchCorner.x) {
            case -1:
                if (stretchWhich) {
                    imageboxes[stretchIndex].x += mD.x;
                    imageboxes[stretchIndex].w -= mD.x;
                }
                else {
                    hitboxes[stretchIndex].x += mD.x;
                    hitboxes[stretchIndex].w -= mD.x;
                }
                break;
            case 1:
                if (stretchWhich) imageboxes[stretchIndex].w += mD.x; else hitboxes[stretchIndex].w += mD.x;
                break;
            case 0: break;
        }
        switch ((int)stretchCorner.y) {
            case -1:
                if (stretchWhich) {
                    imageboxes[stretchIndex].y += mD.y;
                    imageboxes[stretchIndex].h -= mD.y;
                }
                else {
                    hitboxes[stretchIndex].y += mD.y;
                    hitboxes[stretchIndex].h -= mD.y;
                }
                break;
            case 1:
                if (stretchWhich) imageboxes[stretchIndex].h += mD.y; else hitboxes[stretchIndex].h += mD.y;
                break;
            case 0: break;
        }

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



bool exportLevelFile(FilePaths* fp) {
    const int bytesPerHitbox = 4*8 + 2; // ",t" + 4x ",0000.00"
    const int bytesPerImageBox = 8*8 + 3; // ",00" + 4x ",0000.00"



    int filePathsSize = 0;
    for (int i = 0; i < fp->pathCount; i++) filePathsSize += strlen(fp->paths[i]) + 1;
    //                                            "fp" + filepaths
    char* filePathStrs = (char*)calloc(2 + filePathsSize, sizeof(char));
    if (filePathStrs == NULL) return false;
    
    strcpy(filePathStrs, "fp");
    filePathsSize = 0;
    for (int i = 0; i < fp->pathCount; i++) {
        strcat(filePathStrs, ",");
        strcat(filePathStrs, fp->paths[i]);
    }


    
    //                                          "hb"   +   hitbox encoding
    char* hitboxStrs = (char*)calloc(2 + hitboxesFull*bytesPerHitbox, sizeof(char));
    if (hitboxStrs == NULL) return false;
    strncpy(hitboxStrs, "hb", 2);

    for (int i = 0; i < hitboxesFull; i++) {
        const Hitbox h = hitboxes[i];

        char* type = "";
        if (h.typ == SOLID) type = "s"; if (h.typ == TRANSITION) type = "t"; if (h.typ == INTERACT) type = "i";

        const char* coordinates = TextFormat(",%s,%04.02f,%04.02f,%04.02f,%04.02f", type, h.x, h.y, h.w, h.h);
        strncat(hitboxStrs, coordinates, bytesPerHitbox);
    }

    
    //                                            "ib"    +    imagebox encoding
    char* imageBoxStrs = (char*)calloc(2 + imageboxesFull*bytesPerImageBox, sizeof(char));
    if (imageBoxStrs == NULL) return false;
    strncpy(imageBoxStrs, "ib", 2);

    for (int i = 0; i < imageboxesFull; i++) {
        const ImageBox ib = imageboxes[i];

        const char* coordinates = TextFormat(",%02i,%04.02f,%04.02f,%04.02f,%04.02f", ib.fileIndex, ib.x, ib.y, ib.w, ib.h);
        strncat(imageBoxStrs, coordinates, bytesPerImageBox);
    }



    // write everything to the file
    FILE* writeFile;
    writeFile = fopen("./abc.txt", "w");
    if (writeFile == NULL) return false;

    fprintf(writeFile, "%s\n%s\n%s\n\0", filePathStrs, hitboxStrs, imageBoxStrs);
    fclose(writeFile);



    // after combining and saving
    free(hitboxStrs);
    free(imageBoxStrs);
    free(filePathStrs);
    if (hitboxStrs != NULL || imageBoxStrs != NULL || filePathStrs != NULL) return false;
    return true;
}