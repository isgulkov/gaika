
#include <array>
#include <list>
#include <iostream>  //
#include <SDL2/SDL.h>

#include "geometry.hpp"

class wireframe_display
{
    SDL_Renderer* renderer;
    SDL_Window* window;

    std::vector<const phedron3f> contents;

public:
    wireframe_display()
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

    ~wireframe_display()
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
    }

    void add_solid(const phedron3f& solid)
    {
        contents.emplace_back(solid);
    }

    void redraw()
    {
        std::list<std::array<uint8_t, 3>> colors = {
                { 255, 0, 0 },
                { 0, 255, 0 },
                { 0, 0, 255 }
        };

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        for(const phedron3f& ph : contents) {
            SDL_SetRenderDrawColor(renderer, colors.front()[0], colors.front()[1], colors.front()[2], 127);

            colors.push_back(colors.front());
            colors.pop_front();

            for(const triangle<vec3f>& t : ph.faces) {
                SDL_RenderDrawLine(renderer,
                                   (int16_t)(t.a[0] * 250 + 75), (int16_t)(t.a[1] * 250 + 75),
                                   (int16_t)(t.b[0] * 250 + 75), (int16_t)(t.b[1] * 250 + 75)
                );

                SDL_RenderDrawLine(renderer,
                                   (int16_t)(t.b[0] * 250 + 75), (int16_t)(t.b[1] * 250 + 75),
                                   (int16_t)(t.c[0] * 250 + 75), (int16_t)(t.c[1] * 250 + 75)
                );

                SDL_RenderDrawLine(renderer,
                                   (int16_t)(t.a[0] * 250 + 75), (int16_t)(t.a[1] * 250 + 75),
                                   (int16_t)(t.c[0] * 250 + 75), (int16_t)(t.c[1] * 250 + 75)
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
    wireframe_display solids;

    SDL_Delay(1000);

    solids.add_solid(
            phedron3f::tetrahedron({ 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f })
    );

    solids.add_solid(
            phedron3f::tetrahedron({ 1.0f, 1.5f, 0.0f }, { 0.75f, 0.75f, 0.25f }, { 0.98f, -0.09f, 0.0f }, { 0.0f, 0.0f, 1.7f })
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
