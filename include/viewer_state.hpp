
#ifndef DZ_GAIKA_VIEWER_STATE_HPP
#define DZ_GAIKA_VIEWER_STATE_HPP

#include <vector>
#include <cmath>
#include <QColor>

#include "matrices.hpp"
#include "geometry.hpp"

struct wf_model {
    std::vector<vec3f> vertices;
    std::vector<std::pair<uint32_t, uint32_t>> segments;
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> triangles;

    static wf_model tetrahedron(vec3f a, vec3f b, vec3f c, vec3f p)
    {
        // In order A, B, C, P where (A, B, C) is clockwise when looking in P's direction

        return {
                { a, b, c, p },
                { },
                { { 0, 1, 2 }, { 1, 0, 3 }, { 2, 1, 3 }, { 0, 2, 3 } }
        };
    }
};

class wf_projection {
    enum proj_type {
        PERSPECTIVE, PARALLEL, ORTHOGRAPHIC
    };

    proj_type type = PERSPECTIVE;

public:
    enum ortho_axis { X = 0, Y = 1, Z = 2 };

private:
    ortho_axis _axis = X;

    struct {
        float theta, wh_ratio, z_near, z_far;
    } perspective = { (float)M_PI * 2 / 3, 1, 0, 1 };

    float _scale = 0.1f;

public:
    wf_projection() = default;
    wf_projection(float theta, float wh_ratio, float z_near, float z_far) : perspective({ theta, wh_ratio, z_near, z_far }) { }

    bool is_perspective() const { return type == PERSPECTIVE; }

    float theta() const { return perspective.theta; }
    float z_near() const { return perspective.z_near; }
    float z_far() const { return perspective.z_far; }

private:
    static float calculate_theta_b(float theta_a, float ba_ratio)
    {
        return 2 * std::atan(std::tan(theta_a / 2.0f) * ba_ratio);
    }

public:
    float theta_w() const
    {
        if(perspective.wh_ratio >= 1) {
            return theta();
        }
        else {
            return calculate_theta_b(theta(), perspective.wh_ratio);
        }
    }

    float theta_h() const
    {
        if(perspective.wh_ratio <= 1) {
            return theta();
        }
        else {
            return calculate_theta_b(theta(), 1.0f / perspective.wh_ratio);
        }
    }

    void set_theta(float theta) { perspective.theta = theta; }
    void set_wh_ratio(float wh_ratio) { perspective.wh_ratio = wh_ratio; }
    void set_z_near(float z_near) { perspective.z_near = z_near; }
    void set_z_far(float z_far) { perspective.z_far = z_far; }

    bool is_parallel() const { return type == PARALLEL; }
    bool is_orthographic() const { return type == ORTHOGRAPHIC; }
    ortho_axis axis() const { return _axis; };

    float scale() const { return _scale; }

    void set_scale(float scale) { _scale = scale; }

    mat_sq4f tx_project() const
    {
        if(is_perspective()) {
            return mx_tx::project_perspective_z(theta_w(), theta_h(), z_near(), z_far());
        }

        return mx_tx::project_ortho_z() * mx_tx::scale(scale());
    }

    void set_perspective() { type = PERSPECTIVE; }
    void set_parallel() { type = PARALLEL; }
    void set_orthographic(ortho_axis axis) { type = ORTHOGRAPHIC; _axis = axis; }
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

        vec3f pos, orient;
        float scale;

        bool hoverable;
        mutable bool hovered;

        QColor color;
    };

    std::vector<th_object> th_objects;

    struct {
        vec3f pos, orient;
    } camera;

    vec3f v_camera = { 0, 0, 0 };

    // TODO: orthographic perspectives
    wf_projection projection;

    struct {
        int width, height;
    } viewport;

    struct {
        bool forward = false,
                left = false,
                back = false,
                right = false,
                jump = false,
                duck = false;
    } controls;

    struct {
        bool free_look = false;
        bool hovering_disabled = false;
        bool hovering_limited = false;
    } options;

    struct {
        int n = 0;
        float sum_frame = 0, sum_transform = 0, sum_lines = 0;
    } perf_stats;
};

#endif //DZ_GAIKA_VIEWER_STATE_HPP
