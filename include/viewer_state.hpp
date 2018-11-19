
#ifndef DZ_GAIKA_VIEWER_STATE_HPP
#define DZ_GAIKA_VIEWER_STATE_HPP

#include <vector>
#include <QColor>

#include "matrices.hpp"

// TODO: implement other models (triangle- and segment-based)
// TODO: auto-center each model's center of mass at its origin
struct th_model {
    // In order A, B, C, P where (A, B, C) is clockwise when looking in P's direction
    std::vector<vec3f> vertices;
};

struct wf_state
{
    // TODO: lift state-related code up here

    /**
     * A public data structure with the complete state, which is passed by reference to both the updater and
     * display2d. Inputs are to be processed right in this class, or something.
     */

    struct th_object {
        std::shared_ptr<const th_model> model;
        QColor color;
        vec3f pos, orient;
    };

    std::vector<th_object> th_objects;

    struct {
        vec3f pos, orient;
    } camera;

    vec3f v_camera = { 0, 0, 0 };

    // TODO: orthographic perspectives
    struct {
        // TODO: leave the ratio out of here and just use the viewport parameters (theta being the larger side's fov)
        float theta_w, wh_ratio, z_near, z_far;
    } perspective;

    struct {
        int width, height;
    } viewport;

    struct {
        bool forward = false,
                left = false,
                back = false,
                right = false;
    } controls;
};

#endif //DZ_GAIKA_VIEWER_STATE_HPP
