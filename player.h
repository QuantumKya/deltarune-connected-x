#include "raylib.h"
#include "raymath.h"

#include "builder_parts.h"

typedef struct {
    TexSource ts;
    Vector2 p;
} Entity;

const float sprDispMult = 4;

Rectangle getEntityRect(Entity e) {
    Rectangle d = e.ts.src;
    d.x = e.p.x; d.y = e.p.y - e.ts.src.height;
    d.width *= sprDispMult; d.height *= sprDispMult;
    return d;
}

void drawEntity(Entity e) {
    DrawTexturePro(e.ts.tex, e.ts.src, getEntityRect(e), Vector2Zero(), 0, WHITE);
}

const float playerMoveSpeed = 5.0f;
Vector2 moveCheck() {
    return (Vector2){
        playerMoveSpeed * (float)((IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) - (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))),
        playerMoveSpeed * (float)((IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) - (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)))
    };
}



bool collideWithHitbox(Rectangle moveHit, Hitbox hb, Vector2 moveV, Vector2* limitV) {

    float limitX, limitY;
    if (!(moveHit.y >= hb.y + hb.h) && !(moveHit.y + moveHit.height <= hb.y)) {

        if (moveV.x > 0 && moveHit.x + moveHit.width <= hb.x) limitX = hb.x - moveHit.x - moveHit.width;
        else if (moveV.x < 0 && moveHit.x >= hb.x + hb.w) limitX = hb.x + hb.w - moveHit.x;
        else limitX = moveV.x;

    }
    if (!(moveHit.x >= hb.x + hb.w) && !(moveHit.x + moveHit.width <= hb.x)) {

        if (moveV.y > 0 && moveHit.y + moveHit.height <= hb.y) limitY = hb.y - moveHit.y - moveHit.height;
        else if (moveV.y < 0 && moveHit.y >= hb.y + hb.h) limitY = hb.y + hb.h - moveHit.y;
        else limitY = moveV.y;

    }

    const bool xOver = fabsf(limitX) <= fabsf(moveV.x);
    const bool yOver = fabsf(limitY) <= fabsf(moveV.y);
    if (!xOver && !yOver) return false;
    if (limitV == NULL) return true;

    if (xOver) limitV->x = (moveV.x > 0) ? min(moveV.x, limitX) : max(moveV.x, limitX);
    if (yOver) limitV->y = (moveV.y > 0) ? min(moveV.y, limitY) : max(moveV.y, limitY);
}