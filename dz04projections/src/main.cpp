
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2_gfxPrimitives.h>

#include "geometry.hpp"

class solid_window
{
    SDL_Renderer* renderer;
    SDL_Window* window;

    std::vector<const solid3d> contents;

public:
    solid_window()
    {
        window = SDL_CreateWindow(
                "Hello, world!",
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                640,
                480,
                SDL_WINDOW_SHOWN | SDL_WINDOW_ALWAYS_ON_TOP
        );

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    }

    ~solid_window()
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
    }

    void add_solid(const solid3d& solid)
    {
        contents.emplace_back(solid);
    }

    void redraw()
    {
        for(const solid3d& solid : contents) {
            for(const triangle3d& t : solid.faces) {
                filledTrigonRGBA(renderer,
                                 (int16_t)(t.a.x * 250 + 75), (int16_t)(t.a.y * 250 + 75),
                                 (int16_t)(t.b.x * 250 + 75), (int16_t)(t.b.y * 250 + 75),
                                 (int16_t)(t.c.x * 250 + 75), (int16_t)(t.c.y * 250 + 75),
                                 127, 127, 127, 127
                );
            }
        }
    }

    void present()
    {
        SDL_RenderPresent(renderer);
    }
};

int main()
{
    solid_window solids;

    SDL_Delay(1000);

    solids.add_solid(
            solid3d::tetrahedron({ 0, 1, 0 }, { 1, 1, 0 }, { 1, 0, 0 }, { 0, 0, 1 })
    );

    solids.redraw();
    solids.present();

    while(true) {
        SDL_Event event;

        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                SDL_Quit();
                return EXIT_SUCCESS;
            }
        }
    }
}
