
#ifndef DZ_GAIKA_VIEWER_STATE_HPP
#define DZ_GAIKA_VIEWER_STATE_HPP

#include <vector>
#include <string>
#include <cmath>

#include "matrices.hpp"
#include "geometry.hpp"
#include "model.hpp"

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

    float _scale = 0.025f;

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
        std::string id;
        std::shared_ptr<const isg::model> model;

        explicit th_object(std::shared_ptr<const isg::model> model) : id(model->name), model(std::move(model)) { }

        bool hoverable = false;
        mutable bool hovered = false;

        vec3f pos = { 0, 0, 0 };
        vec3f orient = { 0, 0, 0 };
        vec3f scale = { 1, 1, 1 };

        th_object& set_id(const std::string new_id) {
            id = new_id; return *this;
        }
        th_object& set_pos(vec3f new_pos) {
            pos = new_pos; return *this;
        }
        th_object& set_orient(vec3f new_orient) {
            orient = new_orient; return *this;
        }
        th_object& set_scale(vec3f new_scale) {
            scale = new_scale; return *this;
        }
        th_object& set_scale(float s) {
            scale = { s, s, s }; return *this;
        }
        th_object& set_hoverable(bool new_hoverable) {
            hoverable = new_hoverable; return *this;
        }

        // Cached world-transformed vectors
        mutable std::vector<vec3f> vertices_world;
        mutable std::vector<vec3f> normals_world;
    };

    // TODO: angle representation for camera (e.g. which is which)
    // TODO: represent object orientation with quaternions?

    std::vector<th_object> th_objects;

    struct dir_light {
        float azimuth, altitude;

        vec3f color;
        float intensity;
    };

    std::vector<dir_light> dir_lights;

    struct point_light {
        vec3f pos, color;
        float intensity;
    };

    std::vector<point_light> point_lights;

    struct {
        vec3f pos, orient;
    } camera;

    vec3f v_camera = { 0, 0, 0 };

    // TODO: orthographic perspectives
    wf_projection projection;

    struct {
        int width, height;
        int scale;
    } viewport;

    struct {
        bool forward = false,
                left = false,
                back = false,
                right = false,
                jump = false,
                duck = false;
    } controls;

    enum backface_cull {
        BFC_DISABLE, BFC_TRANSPARENT, BFC_CULL
    };

    enum shading {
        SHD_NONE = 0, SHD_FLAT, SHD_GOURAUD, SHD_PHONG
    };

    struct {
        backface_cull use_backface_cull = BFC_CULL;
        shading shading = SHD_FLAT;
    } options;

    enum interaction_mode {
        INT_NONE = 0, INT_DRAG, INT_CARRY, INT_ROTATE
    };

    struct {
        bool free_look = false;

        bool disabled = false;
        bool limited = false;
        bool fixed = false;

        interaction_mode mode = INT_NONE;
        th_object* object = nullptr;
    } hovering;

    struct {
        int n = 0;
        float sum_frame = 0, sum_transform = 0, sum_lines = 0;
    } perf_stats;
};

#endif //DZ_GAIKA_VIEWER_STATE_HPP
