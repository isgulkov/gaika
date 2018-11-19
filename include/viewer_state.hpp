
#ifndef DZ_GAIKA_VIEWER_STATE_HPP
#define DZ_GAIKA_VIEWER_STATE_HPP

#include <vector>
#include <QColor>

#include "matrices.hpp"

// TODO: implement triangle meshes as well
// TODO: auto-center each model's center of mass at its origin
struct wf_model {
    std::vector<vec3f> vertices;
    std::vector<std::pair<uint32_t, uint32_t>> segments;

    static wf_model tetrahedron(vec3f a, vec3f b, vec3f c, vec3f p)
    {
        // In order A, B, C, P where (A, B, C) is clockwise when looking in P's direction

        return {
                { a, b, c, p },
                { { 0, 1 }, { 1, 2 }, { 0, 2 }, { 0, 3 }, { 1, 3 }, { 2, 3 } }
        };
    }
};

struct wf_state
{
    // TODO: lift state-related code up here

    /**
     * A public data structure with the complete state, which is passed by reference to both the updater and
     * display2d. Inputs are to be processed right in this class, or something.
     */

    struct th_object {
        std::shared_ptr<const wf_model> model;

        // TODO: cache the object's world coords
        vec3f pos, orient;
        float scale;

        QColor color;

        // TODO: animation through passing time delta to an object's callback?
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
