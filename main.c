#include <stdio.h>
#include "raylib.h"

#include "player.h"

int main() {
    InitWindow(1280, 960, "DeltaConnected PC");
    SetTargetFPS(60);

    camPos = Vector2Zero();


    Font daFont = LoadFont("dep/determination-8-bit-oper-jve-mono.otf");

    const Image floradinnImg = LoadImage("img/floradinn0.png");
    if (floradinnImg.data == NULL) return 1;

    Entity testMonster;
    testMonster.p.x = 30; testMonster.p.y = 30;
    testMonster.ts.tex = LoadTextureFromImage(floradinnImg);
    testMonster.ts.src = (Rectangle){
        0, 0,
        (float)testMonster.ts.tex.width,
        (float)testMonster.ts.tex.height
    };
    UnloadImage(floradinnImg);



    StringList fp; initLoadMemory(&fp);
    Hitbox* hitboxes = (Hitbox*)calloc(HITBOX_LIMIT, sizeof(Hitbox));
    int hitboxCount = 0;
    ImageBox* imageboxes = (ImageBox*)calloc(IMAGE_LIMIT, sizeof(ImageBox));
    int imageboxCount = 0;

    bool ok = readLevelFile("abc.txt", hitboxes, &hitboxCount, imageboxes, &imageboxCount, &fp);
    printf("loaded=%d hitboxes=%d images=%d\n", ok, hitboxCount, imageboxCount);

    while (!WindowShouldClose()) {

        Vector2 mv = moveCheck();
        for (int i = 0; i < hitboxCount; i++) if (hitboxes[i].typ == SOLID) collideWithHitbox(getEntityRect(testMonster), hitboxes[i], mv, &mv);

        testMonster.p = Vector2Add(testMonster.p, mv);

        BeginDrawing();
            ClearBackground(BLACK);

            for (int i = 0; i < hitboxCount; i++) drawHitbox(hitboxes[i], hitboxColors[hitboxes[i].typ], &fp, &daFont);
            for (int i = 0; i < imageboxCount; i++) drawImageBox(imageboxes[i], GRAY);

            drawEntity(testMonster);
        EndDrawing();
    }
    CloseWindow();

    free(hitboxes);
    free(imageboxes);
    cleanLoadMemory(&fp);
    return 0;
}