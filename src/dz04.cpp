
#include <vector>
#include <array>
#include <list>
#include <iostream>  //
#include <SDL2/SDL.h>

#include "matrices.hpp"
#include "geometry.hpp"

class wf_renderer
{
    std::vector<const std::vector<vec3f>> contents;

    /**
     * State:
     *   - objects: string -> (model, v_up, v_pos)
     *   - camera: (v_pos, v_target, angle_roll)
     *   - projection: ?
     *   - viewport: (width, height)
     */

    struct wf_object
    {
        const std::vector<vec3f> triangles_model;
        vec3f v_up, v_pos;
        uint8_t c_r, c_g, c_b;
    };

    std::vector<wf_object> objects;
    std::vector<const std::vector<vec3f>> objects_world;

    // TODO: make resizable
    const int16_t WIDTH = 800, HEIGHT = 600;

    SDL_Renderer* renderer;
    SDL_Window* window;

public:
    wf_renderer()
    {
        window = SDL_CreateWindow(
                "Hello, world!",
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                WIDTH,
                HEIGHT,
                SDL_WINDOW_SHOWN | SDL_WINDOW_ALWAYS_ON_TOP
        );

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    }

    ~wf_renderer()
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
    }

    void add_solid(const std::vector<vec3f>& solid)
    {
        contents.emplace_back(solid);
    }



private:
    void draw_line(vec3f a, vec3f b)
    {
        const vec2s a_scr = a.onto_xy_screen(WIDTH, HEIGHT),
                b_scr = b.onto_xy_screen(WIDTH, HEIGHT);

        SDL_RenderDrawLine(renderer, a_scr.x(), a_scr.y(), b_scr.x(), b_scr.y());
    }

public:
    // TODO: move to geometry.hpp


    void redraw()
    {
        std::list<std::array<uint8_t, 3>> colors = {
                { 255, 0, 0 },
                { 0, 255, 0 },
                { 0, 0, 255 }
        };

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        const mat_sq4f mx_huirot = mx_transforms::rot_x((float)M_PI * 45.0f / 180.0f);

        for(const std::vector<vec3f>& solid : contents) {
            SDL_SetRenderDrawColor(renderer, colors.front()[0], colors.front()[1], colors.front()[2], 127);

            colors.push_back(colors.front());
            colors.pop_front();

            const std::vector<vec3f> solid_hui = mx_huirot * solid;

            for(size_t i = 0; i < solid_hui.size(); i += 3) {
                draw_line(solid_hui[i], solid_hui[i + 1]);
                draw_line(solid_hui[i + 1], solid_hui[i + 2]);
                draw_line(solid_hui[i + 2], solid_hui[i]);
            }
        }
    }

    void present()
    {
        SDL_RenderPresent(renderer);
    }
};

std::vector<vec3f> suka_tetrahedron(vec3f a, vec3f b, vec3f c, vec3f p)
{
    return {
            a, b, p,
            b, c, p,
            a, c, p,
            a, b, c
    };
}

int main()
{
    vec3f v(1.0f, 2.0f, 3.0f), w(7.0f, 8.0f, 9.0f);

    std::cout << v << std::endl;
    std::cout << w << std::endl;
    std::cout << (v * w) << std::endl;

    // TODO: fix the constructor
    mat_sq4f mx_a(
            {
                    std::array<float, 4> { 1.0f, 1.0f, 1.0f, 1.0f },
                    std::array<float, 4> { 1.0f, 1.0f, 1.0f, 1.0f },
                    std::array<float, 4> { 1.0f, 1.0f, 9.0f, 1.0f },
                    std::array<float, 4> { 1.0f, 1.0f, 1.0f, 1.0f }
            }
    ), mx_b(
            {
                    std::array<float, 4> { 1.0f, 0.0f, 0.0f, 0.0f },
                    std::array<float, 4> { 0.0f, -0.15f, 0.0f, 0.0f },
                    std::array<float, 4> { 0.0f, 0.0f, 0.0f, 0.0f },
                    std::array<float, 4> { 0.0f, 0.0f, 0.0f, 1.0f }
            }
    );

    std::cout << mx_a << std::endl;
    std::cout << mx_b << std::endl;
    std::cout << (mx_a * mx_b) << std::endl;

    wf_renderer solids;

    SDL_Delay(100);

    std::vector<vec3f> th_a = suka_tetrahedron({ 0.0f, 0.75f, 0.0f }, { 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    std::vector<vec3f> th_b = suka_tetrahedron({ 1.0f, 1.0f, 0.0f }, { 0.75f, 0.75f, 0.25f }, { 0.98f, 0.09f, 0.0f }, { 0.0f, 0.0f, 0.7f });

    solids.add_solid(th_a);

    solids.redraw();
    solids.present();

    SDL_Delay(500);

    solids.add_solid(th_b);

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
