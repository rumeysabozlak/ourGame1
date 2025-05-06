#include "raylib.h"
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define BALL_SPEED 5

typedef struct {
    Vector2 position;
    Vector2 speed;
    Color color;
    bool active;
} Ball;

Ball CreateBall(Vector2 startPos, Vector2 direction) {
    Ball ball;
    ball.position = startPos;
    ball.speed = (Vector2){ direction.x * BALL_SPEED, direction.y * BALL_SPEED };
    ball.active = true;

    Color colors[] = { RED, GREEN, BLUE, YELLOW };
    ball.color = colors[rand() % 4]; // Rastgele renk seç

    return ball;
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Marble Puzzle Shoot");
    SetTargetFPS(60);

    srand(time(NULL));

    Vector2 shooterPos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT - 50 };
    Ball ball = { 0 };
    ball.active = false;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE) && !ball.active) {
            ball = CreateBall(shooterPos, (Vector2) { 0, -1 });
        }

        if (ball.active) {
            ball.position.y += ball.speed.y;
            if (ball.position.y < 0) {
                ball.active = false;
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawCircleV(shooterPos, 20, DARKGRAY);
        if (ball.active) {
            DrawCircleV(ball.position, 10, ball.color);
        }

        DrawText("Space ile top fýrlat", 10, 10, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
