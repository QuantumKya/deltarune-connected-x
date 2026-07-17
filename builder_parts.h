#include "raylib.h"
#include "raymath.h"

#include <string.h>

Vector2 camPos;

enum HitboxType {
    SOLID,
    TRANSITION,
    INTERACT
};
const KeyboardKey hitboxTypeKeys[3] = { KEY_Q, KEY_W, KEY_E };
const Color hitboxColors[3] = { RED, BLUE, GREEN };

typedef struct {
    enum HitboxType typ;
    float x; float y;
    float w; float h;
} Hitbox;
typedef struct {
    Texture tex;
    int fileIndex;
    Rectangle src;
    float x; float y;
    float w; float h;
} ImageBox;



// ALL IMAGES MUST HAVE FILE PATHS ACCESSIBLE IN THE PROJECT FOLDER OR SHIT WILL BREAK
const int MAX_IMAGES = 32;
typedef struct {
    char** paths;
    int pathCount;
} FilePaths;

bool initLoadMemory(FilePaths* pl) {
    if (!pl) return false;

    memset(pl, 0, sizeof(*pl));
    pl->paths = calloc(MAX_IMAGES, sizeof(char*));
    if (pl->paths == NULL) return false;
    return true;
}
bool cleanLoadMemory(FilePaths* pl) {
    if (!pl) return false;

    for (int i = 0; i < pl->pathCount; i++) free(pl->paths[i]);
    free(pl->paths);
}
bool addFilePath(FilePaths* pl, const char* newPath) {
    if (pl->pathCount >= 32) return false;

    pl->paths[pl->pathCount] = strdup(newPath);
    if (pl->paths[pl->pathCount] == NULL) return false;

    pl->pathCount++;
    return true;
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
    nib.tex = ib.tex; nib.src = ib.src;
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