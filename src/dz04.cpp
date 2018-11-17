
#include <vector>
#include <array>
#include <list>
#include <iostream>  // REMOVE: !
#include <iomanip>  // REMOVE: !
#include <queue>
#include <SDL2/SDL.h>

#include "matrices.hpp"
#include "geometry.hpp"

class wf_boy
{
    const int16_t WIDTH = 800, HEIGHT = 600; // TODO: make resizable

    SDL_Renderer* renderer;
    SDL_Window* window;

public:
    wf_boy()
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

    ~wf_boy()
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
    }

private:
    /**
     * State:
     *   - objects: string -> (model, position, orientation, color)
     *   - camera: (position, orientation)
     *   - projection: (theta_w, theta_h, z_near, z_far)
     *   - viewport: (width, height)
     */

public:
    struct wf_object
    {
        // TODO: represent objects (at least) as a collection of edges, not triangles
        const std::vector<vec3f> triangles_model;
        vec3f position, orientation;
        uint8_t c_r, c_g, c_b;
    };

private:
public: // REMOVE: <--
    // TODO: cache world coordinates of every object
    std::vector<wf_object> objects;

    // TODO: init with meaningful values and all in one place
    struct {
        vec3f position, orientation;
//    } camera = { { 0.0f, 0.0f, 15.0f }, { 0.0f, 0.0f, 0.0f } };
    } camera = { { 10.0f, 2.5f, 0.0f }, { 0.0f, 0.0f, 0.0f } };

    struct {
        float theta_w, wh_ratio, z_near, z_far;
    } perspective = { 30.0f / 180.f * (float)M_PI, 4.0f / 3.0f, 0.0f, -10.0f };

    float z_min = std::numeric_limits<float>::max(),
          z_max = std::numeric_limits<float>::lowest();

public:
    wf_object& add_object(const std::vector<vec3f>& triangles)
    {
        objects.push_back({ triangles, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, 255, 255, 255 });

        for(const vec3f& v : triangles) {
            if(v.z() < z_min) {
                z_min = v.z();
            }

            if(v.z() > z_max) {
                z_max = v.z();
            }
        }

        return objects.back();
    }

private:
    void draw_line(vec3f a, vec3f b)
    {
        const vec2s a_scr = a.onto_xy_screen(WIDTH, HEIGHT),
                b_scr = b.onto_xy_screen(WIDTH, HEIGHT);

        SDL_RenderDrawLine(renderer, a_scr.x(), a_scr.y(), b_scr.x(), b_scr.y());
    }

public:
    // TODO: move to separate module

//    int shit = 0;

    void redraw()
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

//        SDL_Rect fuck { 0, 0, shit++, 10 };
//        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
//        SDL_RenderFillRect(renderer, &fuck);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        std::list<std::array<uint8_t, 3>> colors = {
                { 255, 165, 0 },
                { 255, 165, 0 },
                { 255, 165, 0 },
                { 255, 165, 0 },
                { 255, 255, 255 },
                { 255, 0, 0 },
                { 0, 255, 0 },
                { 0, 0, 255 }
        };

        const mat_sq4f tx_camera = mx_tx::rot_x(camera.orientation.x())
                            * mx_tx::rot_y(camera.orientation.y())
                            * mx_tx::rot_z(camera.orientation.z())
                            * mx_tx::translate(-camera.position);

//        const mat_sq4f tx_camera = mx_tx::translate(-camera.position);

//        const mat_sq4f tx_perspective = mx_tx::perspective_z(perspective.theta_w,
//                                                             perspective.theta_h,
//                                                             z_max,
//                                                             z_min);

        const mat_sq4f tx_perspective = mx_tx::perspective_z(perspective.theta_w,
                                                             perspective.wh_ratio,
                                                             -1.0f,
                                                             1.0f);

//        const mat_sq4f tx_perspective = mat_sq4f(
//                1.0f, 0.0f, 0.0f, 0.0f,
//                0.0f, 1.0f, 0.0f, 0.0f,
//                0.0f, 0.0f, 1.0f, 0.0f,
//                0.0f, 0.0f, 0.0f, 1.0f
//        );

        const mat_sq4f tx_fucking_scale = mat_sq4f(
                0.1f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.1f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.1f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
        );

        const mat_sq4f tx_view_project = tx_perspective * tx_fucking_scale * tx_camera;

        for(const wf_object& object : objects) {
            const auto c = colors.front();

            SDL_SetRenderDrawColor(renderer, c[0], c[1], c[2], 127);

            colors.push_back(c);
            colors.pop_front();

            const std::vector<vec3f> solid_hui = tx_view_project * object.triangles_model;

            if(menstrukha) {
//                for(size_t i = 0; i < solid_hui.size(); i++) {
//                    std::cout << std::setprecision(2) << solid_hui[i] << " -> " << object.triangles_model[i]
//                              << std::endl;
//                }
//                std::cout << std::endl;


                for(size_t i = 0; i < solid_hui.size(); i++) {
                    std::cout << std::setprecision(2) << object.triangles_model[i].z() << " ";
                }
                std::cout << std::endl;
            }

            for(size_t i = 0; i < solid_hui.size(); i += 3) {
                draw_line(solid_hui[i], solid_hui[i + 1]);
                draw_line(solid_hui[i + 1], solid_hui[i + 2]);
                draw_line(solid_hui[i + 2], solid_hui[i]);
            }
        }

        if(menstrukha) {
            menstrukha = false;
        }
    }

    bool menstrukha = false;

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
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
    ), mx_b(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
    ), mx_c(
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f
    );

//    std::cout << (mx_a * mx_c) << std::endl;

    wf_boy boi;

    SDL_Delay(100);

    std::vector<vec3f> big_nw = suka_tetrahedron(
            { -8.0f, 4.0f, 0.0f },
            { -4.0f, 8.0f, 0.0f },
            { -9.0f, 9.0f, 0.0f },
            { -7.0f, 7.0f, -7.5f }
    );

    std::vector<vec3f> big_ne = suka_tetrahedron(
            { 8.0f, 4.0f, 0.0f },
            { 4.0f, 8.0f, 0.0f },
            { 9.0f, 9.0f, 0.0f },
            { 7.0f, 7.0f, 7.5f }
    );

    std::vector<vec3f> big_sw = suka_tetrahedron(
            { -6.0f, -9.0f, 0.0f },
            { -9.0f, -6.0f, 0.0f },
            { -4.0f, -4.0f, 0.0f },
            { -6.0f, -6.0f, 7.5f }
    );

    std::vector<vec3f> big_se = suka_tetrahedron(
            { 6.0f, -9.0f, 0.0f },
            { 9.0f, -6.0f, 0.0f },
            { 4.0f, -4.0f, 0.0f },
            { 6.0f, -6.0f, -7.5f }
    );

    std::vector<vec3f> center = suka_tetrahedron(
            { -1.0f, -1.0f, -1.0f },
            { 0.0f, 1.0f, -1.0f },
            { 1.0f, -1.0f, -1.0f },
            { 0.0f, 0.0f, 1.5f }
    );

    std::vector<vec3f> skinny_x = suka_tetrahedron(
            { 1.0f, -1.0f, -1.0f },
            { 1.0f, -1.0f, -1.0f },
            { 1.0f, 1.0f, 1.0f },
            { 15.0f, 0.0f, 0.0f }
    );

    std::vector<vec3f> skinny_y = suka_tetrahedron(
            { -1.0f, 1.0f, -1.0f },
            { 1.0f, 1.0f, -1.0f },
            { 1.0f, 1.0f, 1.0f },
            { 0.0f, 15.0f, 0.0f }
    );

    std::vector<vec3f> skinny_z = suka_tetrahedron(
            { -1.0f, -1.0f, 1.0f },
            { -1.0f, -1.0f, 1.0f },
            { 1.0f, 1.0f, 1.0f },
            { 0.0f, 0.0f, 15.0f }
    );

    boi.add_object(big_nw);
    boi.add_object(big_ne);
    boi.add_object(big_sw);
    boi.add_object(big_se);

    boi.add_object(center);

    boi.add_object(skinny_x);
    boi.add_object(skinny_y);
    boi.add_object(skinny_z);

    boi.redraw();
    boi.present();

    std::ios::sync_with_stdio(false);

    const uint32_t FPS_TARGET = 25;

    while(true) {
        // REVIEW: consider whether the time between loop iterations should be accounted for
        uint32_t t_start = SDL_GetTicks();

        SDL_Event event;

        while(SDL_PollEvent(&event)) {
            // mutate

            if(event.type == SDL_KEYDOWN) {
//                std::cout << event.key.keysym.sym << std::endl;

                switch(event.key.keysym.sym) {
                    case SDLK_w:
                        // TODO: actual movement
                        boi.camera.position -= { 0, 0, 2.5f };
                        break;
                    case SDLK_s:
                        boi.camera.position += { 0, 0, 2.5f };
                        break;
                    case SDLK_f:
                        boi.menstrukha = true;
                        break;
                    case SDLK_t:
                        boi.camera.orientation += { 0.05f, 0.0f, 0.0f };
                        break;
                    case SDLK_g:
                        boi.camera.orientation -= { 0.05f, 0.0f, 0.0f };
                        break;
                    case SDLK_y:
                        boi.camera.orientation += { 0.0f, 0.05f, 0.0f };
                        break;
                    case SDLK_h:
                        boi.camera.orientation -= { 0.0f, 0.05f, 0.0f };
                        break;
                    case SDLK_u:
                        boi.camera.orientation += { 0.0f, 0.0f, 0.05f };
                        break;
                    case SDLK_j:
                        boi.camera.orientation -= { 0.0f, 0.0f, 0.05f };
                        break;
                    default:
                        break;
                }
            }
            else if(event.type == SDL_MOUSEMOTION) {
                boi.camera.position += { event.motion.xrel / 100.0f, event.motion.yrel / 100.0f, 0.0f };

//                boi.camera.orientation.x += event.motion.xrel / 250.0f;
//                boi.camera.orientation.y += event.motion.yrel / 250.0f;
            }
            else if(event.type == SDL_QUIT) {
                SDL_Quit();
                return EXIT_SUCCESS;
            }
        }

        boi.redraw();
        boi.present();

        const int32_t ti_delay = 1000 / FPS_TARGET - (SDL_GetTicks() - t_start);

        if(ti_delay > 0) {
            SDL_Delay((uint32_t)ti_delay);
        }
    }
}
