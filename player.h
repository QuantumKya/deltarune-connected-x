#include "raylib.h"
#include "raymath.h"

typedef struct {
    float x;
    float y;
    Texture sprite;
} Entity;

void drawEntity(Entity e) {
    DrawTexture(e.sprite, e.x, e.y - e.sprite.height, WHITE);
}