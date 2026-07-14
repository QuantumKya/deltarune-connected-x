#include "raylib.h"
#include "raymath.h"

Vector2 camPos;

enum HitboxType {
    SOLID = 0,
    TRANSITION = 1,
    INTERACT = 2
};
const Color hitboxColors[3] = { RED, BLUE, GREEN };

typedef struct {
    enum HitboxType typ;
    float x;
    float y;
    float w;
    float h;
} Hitbox;

Rectangle hitboxRect(Hitbox h) {
    Rectangle r;
    r.x = h.x; r.y = h.y;
    r.width = h.w;
    r.height = h.h;
    return r;
}
Rectangle hitboxScreenRect(Hitbox h) {
    Rectangle r = hitboxRect(h);
    r.x -= camPos.x; r.y -= camPos.y;
    return r;
}
Hitbox canonicalHitbox(Hitbox h) {
    Hitbox nh; nh.typ = h.typ;
    nh.w = fabsf(h.w); nh.h = fabsf(h.h);
    nh.x = (h.w < 0) ? h.x - nh.w : h.x;
    nh.y = (h.h < 0) ? h.y - nh.h : h.y;
    return nh;
}