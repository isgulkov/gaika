
#include <iostream>
#include <SDL2/SDL.h>

#include "geometry.hpp"

int main()
{
    SDL_Window* const window = SDL_CreateWindow(
            "Hello, world!",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            640,
            480,
            SDL_WINDOW_SHOWN | SDL_WINDOW_ALWAYS_ON_TOP
    );

    shape3d suk = shape3d::tetrahedron({ 0, 1, 0 }, { 1, 1, 0 }, { 1, 0, 0 }, { 0, 0, 1 });

    std::cout << suk << std::endl;

    for(triangle3d triangle : suk.faces) {
        std::cout << "  - " << triangle << std::endl;
    }

    SDL_Renderer* const renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Rect filling_rect { 0, 0, 640, 480 / 3 };

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &filling_rect);

    filling_rect.y += 480 / 3;
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(renderer, &filling_rect);

    filling_rect.y += 480 / 3;
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &filling_rect);

    filling_rect.y = 0;
    filling_rect.h = 480;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &filling_rect);

    SDL_RenderPresent(renderer);

    while(true) {
        SDL_Event event;

        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();

                return EXIT_SUCCESS;
            }
        }
    }
}
