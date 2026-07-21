#include "raylib.h"
#include "raymath.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define sgn(x) ((x>0) - (x<0));
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

const int HITBOX_LIMIT = 128;
const int IMAGE_LIMIT = 64;

Vector2 camPos;

enum HitboxType {
    SOLID,
    TRANSITION,
    INTERACT
};
enum ImageBoxType {
    BACKGROUND,
    FOREGROUND
};
const Color hitboxColors[3] = { RED, BLUE, GREEN };

typedef struct {
    Texture tex;
    Rectangle src;
} TexSource;

typedef struct {
    int strIndex;
    enum HitboxType typ;
    float x; float y;
    float w; float h;
} Hitbox;
typedef struct {
    int fileIndex;
    TexSource ts;
    float x; float y;
    float w; float h;
} ImageBox;



// ALL IMAGES MUST HAVE FILE PATHS ACCESSIBLE IN THE PROJECT FOLDER OR SHIT WILL BREAK
const int MAX_STRINGS = 64;
typedef struct {
    char** paths;
    int pathCount;
} StringList;

bool initLoadMemory(StringList* pl);
bool cleanLoadMemory(StringList* pl);
int addString(StringList* pl, const char* newStr);

void deleteFromArray(const void* arr, int* counter, const size_t elem_size, const int index, const int howmany);
int addToArrayUnique(void** arr, int* counter, void* elem);
void emptyArrayUnique(void** arr, int* counter);

bool initLoadMemory(StringList* pl) {
    if (!pl) return false;

    memset(pl, 0, sizeof(*pl));
    pl->paths = calloc(MAX_STRINGS, sizeof(char*));
    if (pl->paths == NULL) return false;

    addString(pl, "");
    return true;
}
bool cleanLoadMemory(StringList* pl) {
    if (!pl) return false;

    for (int i = 0; i < pl->pathCount; i++) free(pl->paths[i]);
    free(pl->paths);
}
int addString(StringList* pl, const char* newStr) {
    if (pl->pathCount >= MAX_STRINGS) return -1;

    const int idx = addToArrayUnique((void**)pl->paths, &pl->pathCount, strdup(newStr));
    if (pl->paths[idx] == NULL) {
        deleteFromArray(pl->paths, &pl->pathCount, sizeof(char*), idx, 1);
        return -1;
    }

    return idx;
}



Hitbox hitboxFromRect(const Rectangle r) {
    Hitbox h;
    h.x = r.x; h.y = r.y;
    h.w = r.width; h.h = r.height;
    return h;
}
ImageBox imageBoxFromRect(const Rectangle r) {
    ImageBox ib;
    ib.x = r.x; ib.y = r.y;
    ib.w = r.width; ib.h = r.height;
    return ib;
}

Rectangle hitboxRect(const Hitbox h) {
    Rectangle r;
    r.x = h.x; r.y = h.y;
    r.width = h.w; r.height = h.h;
    return r;
}
Rectangle imageBoxRect(const ImageBox ib) {
    Rectangle r;
    r.x = ib.x; r.y = ib.y;
    r.width = ib.w; r.height = ib.h;
    return r;
}



Rectangle screenRect(Rectangle r) {
    r.x -= camPos.x; r.y -= camPos.y;
    return r;
}
Rectangle canonicalRect(const Rectangle r) {
    Rectangle nr;
    nr.width = fabsf(r.width); nr.height = fabsf(r.height);
    nr.x = (r.width < 0) ? r.x - nr.width : r.x;
    nr.y = (r.height < 0) ? r.y - nr.height : r.y;
    return nr;
}

Hitbox canonicalHitbox(const Hitbox h) {
    const Rectangle cr = canonicalRect(hitboxRect(h));
    Hitbox nh = hitboxFromRect(cr);
    nh.typ = h.typ; return nh;
}
ImageBox canonicalImageBox(const ImageBox ib) {
    const Rectangle cr = canonicalRect(imageBoxRect(ib));
    ImageBox nib = imageBoxFromRect(cr);
    nib.ts.tex = ib.ts.tex; nib.ts.src = ib.ts.src;
    return nib;
}

Rectangle screenHitboxRect(const Hitbox h) {
    Rectangle hr = hitboxRect(h);
    return screenRect(canonicalRect(hr));
}
Rectangle screenImageBoxRect(const ImageBox ib) {
    Rectangle ibr = imageBoxRect(ib);
    return screenRect(canonicalRect(ibr));
}

bool hoveringRect(const ImageBox imagebox, const Vector2 mousePos) {
    const ImageBox ib = canonicalImageBox(imagebox);
    const Vector2 mC = Vector2Add(mousePos, camPos);

    const bool insideRectangle = (mC.x > ib.x && mC.x < ib.x + ib.h && mC.y > ib.y && mC.y < ib.y + ib.h);
    return insideRectangle;
}
bool hoveringEdge(const Rectangle rect, const Vector2 mousePos, const float t) {
    const Rectangle r = canonicalRect(rect);
    const Vector2 mC = Vector2Add(mousePos, camPos);

    const bool nearLeft   = fabsf(mC.x - r.x) <= t && mC.y >= r.y - t && mC.y <= r.y + r.height + t;
    const bool nearRight  = fabsf(mC.x - (r.x + r.width)) <= t && mC.y >= r.y - t && mC.y <= r.y + r.height + t;
    const bool nearTop    = fabsf(mC.y - r.y) <= t && mC.x >= r.x - t && mC.x <= r.x + r.width + t;
    const bool nearBottom = fabsf(mC.y - (r.y + r.height)) <= t && mC.x >= r.x - t && mC.x <= r.x + r.width + t;

    return nearLeft || nearRight || nearTop || nearBottom;
}
bool hoveringEdgeH(const Hitbox h, const Vector2 mousePos, const float t) {
    return hoveringEdge(hitboxRect(h), mousePos, t);
}
bool hoveringEdgeIB(const ImageBox ib, const Vector2 mousePos, const float t) {
    return hoveringEdge(imageBoxRect(ib), mousePos, t);
}



void drawTextCentered(const Vector2 tPos, const Font fnt, const char* text, const float fs, const float spacing, const Color tint) {
    Vector2 textSize = MeasureTextEx(fnt, text, fs, spacing);
    Vector2 textPos = Vector2Subtract(
        tPos,
        Vector2Scale(textSize, 0.5f)
    );

    DrawTextEx(fnt, text, tPos, fs, spacing, tint);
}


void drawHitbox(const Hitbox h, const Color c, StringList* fp, Font* fnt) {
    DrawRectangleLinesEx(
        screenHitboxRect(h),
        5, c
    );
    if (h.typ == TRANSITION && fp != NULL) {
        const char* path = fp->paths[h.strIndex];
        const char tNum = path[strlen(path) - 1];
        drawTextCentered(
            (Vector2){ h.x + h.w/2.0f, h.y + h.h/2.0f },
            *fnt, &tNum, 40.0f, 2.0f, WHITE
        );
    }
}
void drawImageBox(const ImageBox ib, const Color tint) {
    DrawTexturePro(
        ib.ts.tex, ib.ts.src,
        screenImageBoxRect(ib),
        Vector2Zero(), 0,
        tint
    );
}



void deleteFromArray(const void* arr, int* counter, const size_t elem_size, const int index, const int howmany) {
    char* bytePtr = (char*)arr;
    
    for (int k = index; k < *counter - howmany; k++) memcpy(bytePtr + k*elem_size, bytePtr + (k+howmany)*elem_size, elem_size);
    *counter -= howmany;
}

int addToArrayUnique(void** arr, int* counter, void* elem) {
    for (int i = 0; i < *counter; i++) if (arr[i] == elem) return i;
    
    if (*counter >= sizeof(arr)/sizeof(elem)) return -1;
    arr[*counter] = elem; return (*counter)++;
}
void emptyArrayUnique(void** arr, int* counter) {
    for (int i = 0; i < *counter; i++) arr[i] = NULL;
    *counter = 0;
}






bool exportLevelFile(StringList* fp, Hitbox* hitboxes, int hitboxesFull, ImageBox* imageboxes, int imageboxesFull) {
    const int bytesPerHitbox = 4*8 + 2 + 3; // ",0" + ",00" + 4x ",0000.00"
    const int bytesPerImageBox = 4*8 + 4*4 + 3; // ",00" + 4x ",0000.00" + 4x ",000"



    int filePathsSize = 0;
    for (int i = 0; i < fp->pathCount; i++) filePathsSize += strlen(fp->paths[i]) + 1;
    //                                "fp" + filepaths
    char* filePathStrs = (char*)calloc(2 + filePathsSize, sizeof(char));
    if (filePathStrs == NULL) return false;
    
    strcpy(filePathStrs, "fp");
    filePathsSize = 0;
    for (int i = 0; i < fp->pathCount; i++) {
        strcat(filePathStrs, "\n");
        strcat(filePathStrs, fp->paths[i]);
    }


    
    //                              "hb"   +   hitbox encoding
    char* hitboxStrs = (char*)calloc(2 + hitboxesFull*bytesPerHitbox, sizeof(char));
    if (hitboxStrs == NULL) return false;
    strncpy(hitboxStrs, "hb", 2);

    for (int i = 0; i < hitboxesFull; i++) {
        const Hitbox h = hitboxes[i];

        const char* coordinates = TextFormat("\n%01i,%02i,%04.02f,%04.02f,%04.02f,%04.02f", (int)h.typ, h.strIndex, h.x, h.y, h.w, h.h);
        strncat(hitboxStrs, coordinates, bytesPerHitbox);
    }

    
    //                                "ib"    +    imagebox encoding
    char* imageBoxStrs = (char*)calloc(2 + imageboxesFull*bytesPerImageBox, sizeof(char));
    if (imageBoxStrs == NULL) return false;
    strncpy(imageBoxStrs, "ib", 2);

    for (int i = 0; i < imageboxesFull; i++) {
        const ImageBox ib = imageboxes[i];

        const char* coordinates = TextFormat("\n%02i,%04.02f,%04.02f,%04.02f,%04.02f,%03i,%03i,%03i,%03i",
            ib.fileIndex,
            ib.x, ib.y, ib.w, ib.h,
            ib.ts.src.x, ib.ts.src.y, ib.ts.src.width, ib.ts.src.height
        );
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


bool readLevelFile(const char* levelFile, Hitbox* hitboxes, int* hitboxCount, ImageBox* imageboxes, int* imageboxCount, StringList* fp) {
    if (fp == NULL || hitboxes == NULL || imageboxes == NULL) return false;

    FILE* readFile;
    readFile = fopen(levelFile, "r");
    if (readFile == NULL) return false;


    
    char line[256];

    enum ReadMode { FP, HB, IB };
    enum ReadMode cRM = FP;
    const char* buzzwords[3] = { "fp", "hb", "ib" };
    const char* fileSuffixes[3] = { ".png", ".jpg", ".bmp" };

    while (fgets(line, sizeof(line), readFile)) {
        // buzzword recognition
        bool indicatorLine = false;
        for (int i = 0; i < 3; i++) if (strncmp(line, buzzwords[i], 2) == 0) { cRM = (enum ReadMode)i; indicatorLine = true; }
        if (indicatorLine) continue;


        if (cRM == FP) {
            line[strcspn(line, "\r\n")] = '\0';

            char* filename = (char*)calloc(sizeof(line) - 1, sizeof(char));
            strncpy(filename, line, sizeof(line) - 1);
            addToArrayUnique((void**)fp->paths, &fp->pathCount, strdup(filename));
        }
        else if (cRM == HB || cRM == IB) {
            char* section = strtok(line, ",");
    
            ImageBox ib;
            Hitbox hb;

            int passes = 0;
            while (section != NULL) {
                if (cRM == HB) {
                    if (passes >= 6) break;
                    switch (passes) {
                        case 0: hb.typ = (enum HitboxType)strtol(section, NULL, 10); break;
                        case 1: hb.strIndex = (int)strtol(section, NULL, 10); break;
                        case 2: hb.x = (float)strtod(section, NULL); break;
                        case 3: hb.y = (float)strtod(section, NULL); break;
                        case 4: hb.w = (float)strtod(section, NULL); break;
                        case 5: hb.h = (float)strtod(section, NULL); break;
                        default: break;
                    }
                }
                else if (cRM == IB) {
                    if (passes >= 9) break;
                    switch (passes) {
                        case 0:
                            ib.fileIndex = (int)strtol(section, NULL, 10);
                            Image img = LoadImage(fp->paths[ib.fileIndex]);
                            ib.ts.tex = LoadTextureFromImage(img);
                            ib.ts.src = (Rectangle){
                                0, 0,
                                (float)ib.ts.tex.width,
                                (float)ib.ts.tex.height
                            };
                            break;
                        case 1: ib.x = (float)strtod(section, NULL); break;
                        case 2: ib.y = (float)strtod(section, NULL); break;
                        case 3: ib.w = (float)strtod(section, NULL); break;
                        case 4: ib.h = (float)strtod(section, NULL); break;
                        case 5: ib.ts.src.x = (float)strtol(section, NULL, 10); break;
                        case 6: ib.ts.src.y = (float)strtol(section, NULL, 10); break;
                        case 7: ib.ts.src.width = (float)strtol(section, NULL, 10); break;
                        case 8: ib.ts.src.height = (float)strtol(section, NULL, 10); break;
                        default: break;
                    }
                }

                section = strtok(NULL, ",");
                passes++;
            }

            if (cRM == HB) hitboxes[(*hitboxCount)++] = hb;
            if (cRM == IB) imageboxes[(*imageboxCount)++] = ib;
        }
    }

    return true;
}