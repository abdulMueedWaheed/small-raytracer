#include <SDL2/SDL_stdinc.h>
#include <math.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <stdio.h>

#define WIDTH 900
#define HEIGHT 600
#define RAYS_NUMBER 200

#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define COLOR_YELLOW 0xFFFFD700
#define COLOR_GREY 0xFFA9A9A9

struct Circle
{
    double x;
    double y;
    double r;
};

struct Ray {
    double x1, y1;
    double x2, y2;
};

void SetPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
    if (x < 0 || y < 0 || x >= surface->w || y >= surface->h)
        return;
    
    Uint32* pixels = (Uint32*)surface->pixels;
    pixels[(y * surface->w) + x] = color;
}

void FillCircle(SDL_Surface* surface, struct Circle circle, Uint32 color) {
    double dx, dy;
    
    for (int y = (int)(circle.y - circle.r); y <= (int)(circle.y + circle.r); y++) {
        
        for (int x = (int)(circle.x - circle.r); x <= (int)(circle.x + circle.r); x++) {
            dx = x - circle.x;
            dy = y - circle.y;
            
            if (dx*dx + dy*dy <= circle.r * circle.r) {
                SetPixel(surface, x, y, color);
            }
        }
    }
}

_Bool WithinCircle(double x, double y, struct Circle circle) {
    double dx = x - circle.x;
    double dy = y - circle.y;

    return (dx*dx + dy*dy <= circle.r * circle.r);
}

struct Ray CalculateRay(struct Circle light, double angle, struct Circle block) {
    double cx = light.x, cy = light.y;
    
    while(true) {
        if (cx < 0 || cx >= WIDTH || cy < 0 || cy >= HEIGHT || WithinCircle(cx, cy, block)) {
            break;
        }

        cx = cx + 1*cos(angle);
        cy = cy + 1*sin(angle);
    }

    struct Ray newRay = {light.x, light.y, cx, cy};
    return newRay;
}

void DrawLine(SDL_Surface* surface, int x0, int y0, int x1, int y1, Uint32 color) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int e2;

    while (true) {
        SetPixel(surface, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}


void DrawRays(SDL_Surface* surface, struct Ray rays[RAYS_NUMBER], Uint32 color) {
    for (int i = 0 ; i < RAYS_NUMBER ; i++) {
        DrawLine(surface,
                 (int)rays[i].x1, (int)rays[i].y1,
                 (int)rays[i].x2, (int)rays[i].y2,
                 color);
    }
}


void GenerateRays(struct Circle light, struct Ray rays[RAYS_NUMBER], struct Circle block) {
    double angleStep = (2 * M_PI) / RAYS_NUMBER;
    double angle = 0;
    for (int i = 0 ; i < RAYS_NUMBER ; i++) {
        angle += angleStep;

        
        struct Ray ray = CalculateRay(light, angle, block);
        rays[i] = ray;
    }
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! Error: %s\n", SDL_GetError());
        return 1;
    }
    
    SDL_Window* window = SDL_CreateWindow("RayTracing",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WIDTH, HEIGHT,
                                          SDL_WINDOW_SHOWN);
    
    if (!window) {
        SDL_Log("Window could not be created: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Surface* surface = SDL_GetWindowSurface(window);
    struct Circle light_circle = {240, 440, 50};
    struct Circle shadow_circle = {520, 240, 100};

    struct Ray rays[RAYS_NUMBER];
    GenerateRays(light_circle, rays, shadow_circle);

    _Bool running = true;
    _Bool dragging_light = false;
    _Bool dragging_shadow = false;
    
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (WithinCircle(event.button.x, event.button.y, light_circle)) {
                    dragging_light = true;
                }

                else if (WithinCircle(event.button.x, event.button.y, shadow_circle)) {
                    dragging_shadow = true;
                }
            }

            else if (event.type == SDL_MOUSEBUTTONUP) {
                dragging_light = false;
                dragging_shadow = false;
            }

            else if (event.type == SDL_MOUSEMOTION) {
                if (dragging_light) {
                    light_circle.x = event.motion.x;
                    light_circle.y = event.motion.y;

                    GenerateRays(light_circle, rays, shadow_circle);
                }

                else if (dragging_shadow) {
                    shadow_circle.x = event.motion.x;
                    shadow_circle.y = event.motion.y;

                    GenerateRays(light_circle, rays, shadow_circle);
                }
            }
        }
        
        SDL_FillRect(surface, NULL, COLOR_BLACK);
        
        FillCircle(surface, light_circle, COLOR_YELLOW);
        DrawRays(surface, rays, COLOR_WHITE);
        FillCircle(surface, shadow_circle, COLOR_GREY);

        SDL_UpdateWindowSurface(window);
        
        SDL_Delay(16);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
};
