#ifndef RAYLIB_H
#define RAYLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#if defined(_WIN32)
#define RLAPI __declspec(dllimport)
#else
#define RLAPI
#endif

typedef struct Vector2 {
    float x;
    float y;
} Vector2;

typedef struct Rectangle {
    float x;
    float y;
    float width;
    float height;
} Rectangle;

typedef struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} Color;

#define KEY_R 82
#define MOUSE_LEFT_BUTTON 0

RLAPI void InitWindow(int width, int height, const char *title);
RLAPI void CloseWindow(void);
RLAPI bool WindowShouldClose(void);
RLAPI void SetTargetFPS(int fps);
RLAPI bool IsKeyPressed(int key);
RLAPI bool IsMouseButtonPressed(int button);
RLAPI Vector2 GetMousePosition(void);
RLAPI bool CheckCollisionPointRec(Vector2 point, Rectangle rec);
RLAPI void BeginDrawing(void);
RLAPI void EndDrawing(void);
RLAPI void ClearBackground(Color color);
RLAPI void DrawText(const char *text, int posX, int posY, int fontSize, Color color);
RLAPI void DrawRectangleRec(Rectangle rec, Color color);
RLAPI void DrawRectangleLinesEx(Rectangle rec, float lineThick, Color color);
RLAPI void DrawRectangleLines(int posX, int posY, int width, int height, Color color);
RLAPI void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color);
RLAPI void DrawCircle(int centerX, int centerY, float radius, Color color);
RLAPI void DrawCircleLines(int centerX, int centerY, float radius, Color color);

extern const Color DARKBROWN;
extern const Color DARKGRAY;
extern const Color MAROON;
extern const Color DARKGREEN;
extern const Color RED;

#ifdef __cplusplus
}
#endif

#endif
