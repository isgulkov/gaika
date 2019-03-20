
#ifndef DZ_GAIKA_DISPLAY2D_HPP
#define DZ_GAIKA_DISPLAY2D_HPP

#include <utility>
#include <algorithm>
#include <iterator>
#include <cmath>

#include <QString>
#include <QTextStream>
#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>
#include <QBrush>
#include <QFont>
#include <QFontDatabase>
#include <QPen>
#include <QApplication>
#include <QSurfaceFormat>
#include <QDebug>

#include "matrices.hpp"
#include "geometry.hpp"
#include "viewer_state.hpp"
#include "model.hpp"

class render3d
{
    const wf_state& state;

    const int width, height;
//    const std::unique_ptr<uint8_t[]> buffer;

    int zb_width;
    std::vector<float> z_buffer;

public:
    const wf_state::th_object* hovered_object;
    bool hovered_multiple;

    float d_hovered = MAXFLOAT;

public:
    render3d(const wf_state& state) : state(state),
                                      width(state.viewport.width), height(state.viewport.height) { }

    static void draw_line(QPainter& painter, vec3f a, vec3f b)
    {
        painter.drawLine((int)a.x, (int)a.y, (int)b.x, (int)b.y);
    }

    static QColor clamped_rgb24(const vec3f& color)
    {
        const vec3f c = color.clamped(0, 256);

        return { uint8_t(c.x), uint8_t(c.y), uint8_t(c.z) };
    }

    inline void draw_pixel(QPainter& painter, int x, int y, const vec3f& color)
    {
        /**
         * Color values go out of range because both X and Y extremes can actually land outside the triangle during
         * interpolation. This is not the only time colors need to be clamped, so might as well.
         *
         * REVIEW: is this right:
         */
        painter.setPen(clamped_rgb24(color));
        painter.drawPoint(x, y);
    }

    inline void put_pixel(QPainter& painter, int x, int y, float z)
    {
        if(state.options.occlusion != wf_state::OCC_BFC_ZBUF) {
            painter.drawPoint(x, y);
            return;
        }

        float& z_value = z_buffer[x + y * width];

        if(z < z_value) {
            painter.drawPoint(x, y);

            z_value = z;
        }
    }

    inline void put_pixel(QPainter& painter, int x, int y, float z, const vec3f& color)
    {
        if(state.options.occlusion != wf_state::OCC_BFC_ZBUF) {
            draw_pixel(painter, x, y, color);
            return;
        }

        float& z_value = z_buffer[x + y * width];

        if(z < z_value) {
            draw_pixel(painter, x, y, color);

            z_value = z;
        }
    }

    void draw_triangle_flat_top(QPainter& painter, vec3f a, vec3f b, vec3f c)
    {
        const float alpha_left = (c.x - a.x) / (c.y - a.y);
        const float alpha_right = (c.x - b.x) / (c.y - b.y);

        const float dz_left = (c.z - a.z) / (c.y - a.y);
        const float dz_right = (c.z - b.z) / (c.y - b.y);

        const int y_start = std::max(0, (int)std::ceil(a.y - 0.5f));
        const int y_end = std::min(height - 1, (int)std::ceil(c.y - 0.5f));

        float z_left = a.z + dz_left * (y_start - a.y);
        float z_right = b.z + dz_right * (y_start - b.y);

        for(int y = y_start; y < y_end; y++) {
            const float x_left = a.x + alpha_left * (y - a.y + 0.5f);
            const float x_right = b.x + alpha_right * (y - b.y + 0.5f);

            const int x_start = std::max(0, (int)std::ceil(x_left - 0.5f));
            const int x_end = std::min(width - 1, (int)std::ceil(x_right - 0.5f));

            const float dz_line = (z_right - z_left) / (x_right - x_left);
            float z = z_left + dz_line * (x_start - x_left);

            for(int x = x_start; x < x_end; x++) {
                put_pixel(painter, x, y, z);

                z += dz_line;
            }

            z_left += dz_left;
            z_right += dz_right;
        }
    }

    void draw_triangle_flat_bottom(QPainter& painter, vec3f a, vec3f b, vec3f c)
    {
        // TODO: refactor commonalities out of ..._top and ..._bottom

        const float alpha_left = (b.x - a.x) / (b.y - a.y);
        const float alpha_right = (c.x - a.x) / (c.y - a.y);

        const float dz_left = (b.z - a.z) / (b.y - a.y);
        const float dz_right = (c.z - a.z) / (c.y - a.y);

        const int y_start = std::max(0, (int)std::ceil(a.y - 0.5f));
        const int y_end = std::min(height - 1, (int)std::ceil(c.y - 0.5f));

        float z_left = a.z + dz_left * (y_start - a.y);
        float z_right = a.z + dz_right * (y_start - a.y);

        for(int y = y_start; y < y_end; y++) {
            const float x_left = a.x + alpha_left * (y + 0.5f - a.y);
            const float x_right = a.x + alpha_right * (y + 0.5f - a.y);

            const int x_start = std::max(0, (int)std::ceil(x_left - 0.5f));
            const int x_end = std::min(width - 1, (int)std::ceil(x_right - 0.5f));

            const float dz_line = (z_right - z_left) / (x_right - x_left);
            float z = z_left + dz_line * (x_start - x_left);

            for(int x = x_start; x < x_end; x++) {
                put_pixel(painter, x, y, z);

                z += dz_line;
            }

            z_left += dz_left;
            z_right += dz_right;
        }
    }

protected:
    void draw_triangle(QPainter& painter, vec3f a, vec3f b, vec3f c, const vec3f& color)
    {
        painter.setPen(clamped_rgb24(color));

        // Sort: `a` at the top, `c` at the bottom
        if(a.y > b.y) std::swap(a, b);
        if(b.y > c.y) std::swap(b, c);
        if(a.y > b.y) std::swap(a, b);

        if(a.y == b.y) {
            if(a.x > b.x) std::swap(a, b);
            draw_triangle_flat_top(painter, a, b, c);
        }
        else if(b.y == c.y) {
            if(b.x > c.x) std::swap(b, c);
            draw_triangle_flat_bottom(painter, a, b, c);
        }
        else {
            const float alpha_split = (b.y - a.y) / (c.y - a.y);

            // TODO: rewrite using operators when this is vec3f
            const vec3f s = {
                    a.x + (c.x - a.x) * alpha_split,
                    a.y + (c.y - a.y) * alpha_split,
                    a.z + (c.z - a.z) * alpha_split
            };

            if(s.x < b.x) {
                draw_triangle_flat_bottom(painter, a, s, b);
                draw_triangle_flat_top(painter, s, b, c);
            }
            else {
                draw_triangle_flat_bottom(painter, a, b, s);
                draw_triangle_flat_top(painter, b, s, c);
            }
        }
    }

private:
    void draw_triangle_flat_top_gouraud(QPainter& painter, vec3f a, vec3f b, vec3f c, vec3f c_a, vec3f c_b, vec3f c_c)
    {
        const float alpha_left = (c.x - a.x) / (c.y - a.y);
        const float alpha_right = (c.x - b.x) / (c.y - b.y);

        const float dz_left = (c.z - a.z) / (c.y - a.y);
        const float dz_right = (c.z - b.z) / (c.y - b.y);

        const vec3f dc_left = (c_c - c_a) / (c.y - a.y);
        const vec3f dc_right = (c_c - c_b) / (c.y - b.y);

        const int y_start = std::max(0, (int)std::ceil(a.y - 0.5f));
        const int y_end = std::min(height - 1, (int)std::ceil(c.y - 0.5f));

        vec3f c_left = c_a + dc_left * (y_start - a.y);
        vec3f c_right = c_b + dc_right * (y_start - b.y);

        float z_left = a.z + dz_left * (y_start - a.y);
        float z_right = b.z + dz_right * (y_start - b.y);

        for(int y = y_start; y < y_end; y++) {
            const float x_left = a.x + alpha_left * (y - a.y + 0.5f);
            const float x_right = b.x + alpha_right * (y - b.y + 0.5f);

            const int x_start = std::max(0, (int)std::ceil(x_left - 0.5f));
            const int x_end = std::min(width - 1, (int)std::ceil(x_right - 0.5f));

            const float dz_line = (z_right - z_left) / (x_right - x_left);
            float z = z_left + dz_line * (x_start - x_left);

            const vec3f dc_line = (c_right - c_left) / (x_right - x_left);
            vec3f color = c_left + dc_line * (x_start - x_left);

            for(int x = x_start; x < x_end; x++) {
                put_pixel(painter, x, y, z, color);

                z += dz_line;
                color += dc_line;
            }

            z_left += dz_left;
            z_right += dz_right;

            c_left += dc_left;
            c_right += dc_right;
        }
    }

    void draw_triangle_flat_bottom_gouraud(QPainter& painter, vec3f a, vec3f b, vec3f c, vec3f c_a, vec3f c_b, vec3f c_c)
    {
        // TODO: refactor commonalities out of ..._top and ..._bottom

        const float alpha_left = (b.x - a.x) / (b.y - a.y);
        const float alpha_right = (c.x - a.x) / (c.y - a.y);

        const float dz_left = (b.z - a.z) / (b.y - a.y);
        const float dz_right = (c.z - a.z) / (c.y - a.y);

        const vec3f dc_left = (c_b - c_a) / (b.y - a.y);
        const vec3f dc_right = (c_c - c_a) / (c.y - a.y);

        const int y_start = std::max(0, (int)std::ceil(a.y - 0.5f));
        const int y_end = std::min(height - 1, (int)std::ceil(c.y - 0.5f));

        float z_left = a.z + dz_left * (y_start - a.y);
        float z_right = a.z + dz_right * (y_start - a.y);

        vec3f c_left = c_a + dc_left * (y_start - a.y);
        vec3f c_right = c_a + dc_right * (y_start - a.y);

        for(int y = y_start; y < y_end; y++) {
            const float x_left = a.x + alpha_left * (y + 0.5f - a.y);
            const float x_right = a.x + alpha_right * (y + 0.5f - a.y);

            const int x_start = std::max(0, (int)std::ceil(x_left - 0.5f));
            const int x_end = std::min(width - 1, (int)std::ceil(x_right - 0.5f));

            const float dz_line = (z_right - z_left) / (x_right - x_left);
            float z = z_left + dz_line * (x_start - x_left);

            const vec3f dc_line = (c_right - c_left) / (x_right - x_left);
            vec3f color = c_left + dc_line * (x_start - x_left);

            for(int x = x_start; x < x_end; x++) {
                put_pixel(painter, x, y, z, color);

                z += dz_line;
                color += dc_line;
            }

            z_left += dz_left;
            z_right += dz_right;

            c_left += dc_left;
            c_right += dc_right;
        }
    }

protected:
    void draw_triangle_gouraud(QPainter& painter, vec3f a, vec3f b, vec3f c, vec3f c_a, vec3f c_b, vec3f c_c)
    {
        // Sort: `a` at the top, `c` at the bottom
        if(a.y > b.y) {
            std::swap(a, b);
            std::swap(c_a, c_b);
        }

        if(b.y > c.y) {
            std::swap(b, c);
            std::swap(c_b, c_c);
        }

        if(a.y > b.y) {
            std::swap(a, b);
            std::swap(c_a, c_b);
        }

        if(a.y == b.y) {
            if(a.x > b.x) std::swap(a, b);
            draw_triangle_flat_top_gouraud(painter, a, b, c, c_a, c_b, c_c);
        }
        else if(b.y == c.y) {
            if(b.x > c.x) std::swap(b, c);
            draw_triangle_flat_bottom_gouraud(painter, a, b, c, c_a, c_b, c_c);
        }
        else {
            const float alpha_split = (b.y - a.y) / (c.y - a.y);

            const vec3f s = {
                    a.x + (c.x - a.x) * alpha_split,
                    a.y + (c.y - a.y) * alpha_split,
                    a.z + (c.z - a.z) * alpha_split
            };

            const vec3f c_s = c_a * (1 - alpha_split) + c_c * alpha_split;

            if(s.x < b.x) {
                draw_triangle_flat_bottom_gouraud(painter, a, s, b, c_a, c_s, c_b);
                draw_triangle_flat_top_gouraud(painter, s, b, c, c_s, c_b, c_c);
            }
            else {
                draw_triangle_flat_bottom_gouraud(painter, a, b, s, c_a, c_b, c_s);
                draw_triangle_flat_top_gouraud(painter, b, s, c, c_b, c_s, c_c);
            }
        }
    }

    vec3f to_screen(const vec3f& v)
    {
        return {
                (v.x / 2.0f + 0.5f) * width,
                (-v.y / 2.0f + 0.5f) * height,
                v.z
        };
    }

    static bool test_p_in_triangle(const QPoint& p, const vec3f& a, const vec3f& b, const vec3f& c)
    {
        // TODO: figure out how this works and document
        const int x_ap = p.x() - (int)a.x;
        const int y_ap = p.y() - (int)a.y;

        const bool dp_ab = (b.x - a.x) * y_ap - (b.y - a.y) * x_ap > 0;
        const bool dp_ac = (c.x - a.x) * y_ap - (c.y - a.y) * x_ap > 0;

        if(dp_ab == dp_ac) {
            return false;
        }

        const bool whatever = (c.x - b.x) * (p.y() - b.y) - (c.y - b.y) * (p.x() - b.x) > 0;

        return whatever == dp_ab;
    }

protected:
    mat_sq4f create_tx_camera()
    {
        mat_sq4f tx_camera = mx_tx::translate(-state.camera.pos);

        if(!state.projection.is_orthographic()) {
            tx_camera = mx_tx::rotate_xyz(state.camera.orient).transpose() * tx_camera;
        }
        else {
            // TODO: write row-swap matrices manually (or swap rows on existing ones)
            switch(state.projection.axis()) {
                case wf_projection::X:
                    tx_camera = mx_tx::rotate_z(-(float)M_PI_2) * mx_tx::rotate_y(-(float)M_PI_2) * tx_camera;
                    break;
                case wf_projection::Y:
                    tx_camera = mx_tx::rotate_x(-(float)M_PI_2) * tx_camera;
                    break;
                case wf_projection::Z:
                    break;
            }
        }

        return tx_camera;
    }

    mat_sq4f create_tx_projection()
    {
        if(state.projection.is_perspective()) {
            return mx_tx::project_perspective_z(
                    state.projection.theta_w(),
                    state.projection.theta_h(),
                    state.projection.z_near(),
                    state.projection.z_far()
            );
        }

        /**
         * - To avoid distortion on non-square viewport, the hor. axis (camera's X) is squeezed by its aspect ratio
         * - For orthographic projections, near-far clipping is prevented by fixing Z
         */
        const float hw_ratio = (float)height / width;

        if(state.projection.is_orthographic()) {
            return mx_tx::scale(state.projection.scale()) * mx_tx::scale(hw_ratio, 1, 0);
        }

        // TODO: reconcile the scale with the Z mapping (right now the far plane seems a bit close)
        return mx_tx::scale(1, 1, -1.0f / (state.projection.z_far() - state.projection.z_near()))
               * mx_tx::translate(0, 0, -state.projection.z_near())
               * mx_tx::scale(state.projection.scale(), state.projection.scale(), 1)
               * mx_tx::scale(hw_ratio, 1, 1);
    }

    std::vector<vec3f> vx_world;

    /**
     * TODO: for each vertex
     *  - world coords
     *  - with Phong shading:
     *    - world normal (default to face normal)
     *    - material reference
     *  - without Phong shading:
     *    - final color (per vertex or per face)
     */
    std::vector<vec3f> vns_world;
    std::vector<vec3f> v_colors;

    std::vector<const wf_state::th_object*> px_objects;
    std::vector<QPen> tri_pens;

    std::vector<char> tri_nonorms;

    vec3f calc_light(const isg::material& mtl, const vec3f& norm_world, const vec3f& pos_world)
    {
        const auto& lighting = state.lighting;

        vec3f color = (mtl.has_ambient ? mtl.c_ambient : mtl.c_diffuse) * lighting.amb_color;

        // TODO: Process state.options.lighting
        // If ...lighting == ...AMBIENT, return color

        for(size_t i = 0; i < lighting.dir_lights.size(); i++) {
            const auto& light = state.lighting.dir_lights[i];
            const vec3f& dir = vx_dir_lights[i];

            // Diffuse component
            color += mtl.c_diffuse * light.color * std::max(0.0f, dir.dot(norm_world));

            // Specular component
            const vec3f dir_reflected = 2 * (vx_dir_lights[i].dot(norm_world)) * norm_world - vx_dir_lights[i];
            const vec3f dir_camera = (state.camera.pos - pos_world).normalize();

            const vec3f light_specular = mtl.c_specular * std::powf(std::max(0.0f, dir_reflected.dot(dir_camera)), mtl.exp_specular) * light.color;

            color += light_specular;
        }

        // TODO: D R Y
        for(const auto& light : state.th_objects) {
            if(!light.is_point_light) {
                continue;
            }

            vec3f dir_light = light.pos - pos_world;
            const float d_light = dir_light.norm();

            dir_light /= d_light;

            // Point lights have to have a "radius" parameter, for which just half the height is used
            const float attenuation = 1.0f / std::powf(d_light / (light.scale.z / 2) + 1, 2);

            // Diffuse component
            color += mtl.c_diffuse * light.light_color * std::max(0.0f, dir_light.dot(norm_world)) * attenuation; // TODO: apply attenuation

            // Specular component
            const vec3f dir_reflected = 2 * (dir_light.dot(norm_world)) * norm_world - dir_light;
            const vec3f dir_camera = (state.camera.pos - pos_world).normalize();

            const vec3f light_specular = mtl.c_specular * std::powf(std::max(0.0f, dir_reflected.dot(dir_camera)), mtl.exp_specular) * light.light_color;

            color += light_specular * attenuation;
        }

        return color.clamp(0, 1);
    }

    void collect_triangles(const wf_state::th_object& object)
    {
        if(object.vertices_world.empty()) {
            const mat_sq4f rot_world = mx_tx::rotate_xyz(object.orient);
            const mat_sq4f tx_world = mx_tx::translate(object.pos) * rot_world * mx_tx::scale(object.scale);

            object.vertices_world = tx_world * object.model->vertices;
            object.normals_world = rot_world * object.model->normals;
        }

        vec3f dir_out;

        if(state.projection.is_parallel()) {
            dir_out = mx_tx::rotate_xyz(state.camera.orient) * vec3f(0, 0, -1);
        }
        else {
            switch(state.projection.axis()) {
                case wf_projection::X:
                    dir_out = vec3f(-1, 0, 0);
                    break;
                case wf_projection::Y:
                    dir_out = vec3f(0, -1, 0);
                    break;
                case wf_projection::Z:
                    dir_out = vec3f(0, 0, -1);
                    break;
            }
        }

        QPen face_pen;

        if(state.projection.is_orthographic()) {
            if(state.hovering.fixed) {
                if(&object == state.hovering.object) {
                    face_pen.setStyle(Qt::SolidLine);
                    face_pen.setWidth(2);
                }
            }
            else if(object.hovered) {
                if(state.hovering.limited) {
                    face_pen.setDashPattern({ 1, 5 });
                }
                else {
                    face_pen.setStyle(Qt::DashLine);
                    face_pen.setWidth(2);
                }
            }
        }

        if(!state.hovering.fixed) {
            object.hovered = false;
        }

        for(const auto& triangle : object.model->faces) {
            const vec3f& a = object.vertices_world[triangle.i_a],
                    b = object.vertices_world[triangle.i_b],
                    c = object.vertices_world[triangle.i_c];

            // TODO: make sure this doesn't break when a model is flipped (scaled by a negative factor)
            if(state.projection.is_perspective()) {
                dir_out = a - state.camera.pos;
            }

            /**
             * Triangles with COUNTERCLOCKWISE order of vertices are considered FRONT-FACING here. This is
             * apparently how OpenGL does it by default, so most meshes out there are like this as well.
             */
            const vec3f tri_norm = (b - a).cross(c - a).normalize();

            // REVIEW: See if this flag check really has no overhead
            if(state.options.occlusion != wf_state::OCC_NONE && dir_out.dot(tri_norm) >= 0) {
                continue;
            }

            vx_world.push_back(a);
            vx_world.push_back(b);
            vx_world.push_back(c);

            const auto& mtl = object.model->materials[triangle.i_mtl];

            if(state.projection.is_orthographic() || state.options.shading == wf_state::SHD_NONE) {
                // TODO: Display wireframes with SHD_NONE ()
                v_colors.push_back(255 * mtl.c_diffuse);
            }
            else if(state.options.shading == wf_state::SHD_FLAT) {
                v_colors.push_back(255 * calc_light(mtl, tri_norm, (a + b + c) / 3));
            }
            else if(state.options.shading == wf_state::SHD_GOURAUD) {
                bool has_norms = false;

                for(const auto& pv : triangle.ix_vectors()) {
                    const vec3f& norm_world = pv.second != SIZE_T_MAX ? object.normals_world[pv.second] : tri_norm;
                    const vec3f& pos_world = object.vertices_world[pv.first];

                    if(pv.second != SIZE_T_MAX) {
                        has_norms = true;
                    }

                    v_colors.push_back(255 * calc_light(mtl, norm_world, pos_world));
                }

                /**
                 * Short circuit models without normals to flat shading, which Gouraud is equivalent to when
                 * the face normal is used by default.
                 */
                tri_nonorms.push_back(!has_norms);
            }
            else if(state.options.shading == wf_state::SHD_PHONG) {
                bool has_norms = false;

                for(const auto& pv : triangle.ix_vectors()) {
                    if(pv.second != SIZE_T_MAX) {
                        vns_world.push_back(object.normals_world[pv.second]);
                        has_norms = true;
                    }
                    else {
                        vns_world.push_back(tri_norm);
                    }
                }

                tri_nonorms.push_back(!has_norms);
            }

            px_objects.push_back(&object);

            if(state.projection.is_orthographic()) {
                tri_pens.push_back(face_pen);
            }
        }
    }

    void collect_triangles()
    {
        const mat_sq4f tx_camera = create_tx_camera();

        hovered_object = nullptr;
        hovered_multiple = false;

        for(const wf_state::th_object& object : state.th_objects) {
            collect_triangles(object);
        }

        /**
         * TODO: draw light sources
         *  - point lights must be movable like other objects
         *  - directional lights must have constant angular size (s.t. you could zoom in on them)
         */
    }

    void render_triangles(QPainter& painter, QPoint p_cursor)
    {
        if(state.options.occlusion == wf_state::OCC_BFC_ZBUF) {
            z_buffer.resize(size_t(width) * size_t(height));
            std::fill(z_buffer.begin(), z_buffer.end(), MAXFLOAT);
        }

        const mat_sq4f tx_camera = create_tx_camera();
        const mat_sq4f tx_projection = create_tx_projection();

        /**
         * TODO: Implement proper z-clipping
         *  - Produce up to three triangles through interpolation (probably on collection stage)
         */
        const std::vector<vec4f> vertices_clipping = tx_projection.mul_homo(tx_camera * vx_world);

        std::vector<char> vertex_beyond_near;
        vertex_beyond_near.reserve(vertices_clipping.size());

        for(const vec4f& vertex : vertices_clipping) {
            vertex_beyond_near.push_back(vertex.z < 0);
        }

        const std::vector<vec3f> vertices_screen = mx_tx::scale(width / 2.0f, -height / 2.0f, 1) * mx_tx::translate(1, -1, 0) * vertices_clipping;

        for(size_t i = 0; i < vx_world.size(); i += 3) {
            if(vertex_beyond_near[i] || vertex_beyond_near[i + 1] || vertex_beyond_near[i + 2]) {
                continue;
            }

            const vec3f& a = vertices_screen[i], b = vertices_screen[i + 1], c = vertices_screen[i + 2];

            if(state.projection.is_orthographic() || state.options.shading == wf_state::SHD_NONE) {
                const vec3f color = v_colors[i / 3];

                QPen face_pen = tri_pens[i / 3];
                face_pen.setColor({ int(color.x), int(color.y), int(color.z) });
                painter.setPen(face_pen);

                draw_line(painter, a, b);
                draw_line(painter, a, c);
                draw_line(painter, b, c);
            }
            else if(state.options.shading == wf_state::SHD_FLAT) {
                draw_triangle(painter, a, b, c, v_colors[i / 3]);
            }
            else if(state.options.shading == wf_state::SHD_GOURAUD) {
                if(!tri_nonorms[i / 3]) {
                    draw_triangle_gouraud(painter, a, b, c, v_colors[i], v_colors[i + 1], v_colors[i + 2]);
                }
                else {
                    draw_triangle(painter, a, b, c, v_colors[i]);
                }
            }
            else if(state.options.shading == wf_state::SHD_PHONG) {

            }

            if(state.hovering.disabled || state.hovering.fixed) {
                continue;
            }

            const auto& object = *px_objects[i / 3];

            if(!object.hovered && object.hoverable && test_p_in_triangle(p_cursor, a, b, c)) {
                if(state.projection.is_orthographic()) {
                    object.hovered = true;

                    if(!hovered_multiple && hovered_object != nullptr) {
                        hovered_multiple = true;
                        hovered_object = nullptr;
                    }

                    hovered_object = &object;
                }
                else {
                    /**
                     * Only hover the nearest object
                     *
                     * TODO: Use Z at the intersection point, not the center.
                     */
                    const float d = (a.z + b.z + c.z) / 3;

                    if(hovered_object == nullptr || d < d_hovered) {
                        d_hovered = d;
                        hovered_object = &object;
                    }
                }
            }
        }
    }

    std::vector<vec3f> vx_dir_lights;

public:
    void render(QPainter& painter, QPoint p_cursor)
    {
        painter.fillRect(0, 0, width, height, Qt::black);

        hovered_object = nullptr;
        hovered_multiple = false;

        // TODO: eliminate repetitions of this shit
        const mat_sq4f tx_camera = create_tx_camera();
        const mat_sq4f tx_projection = create_tx_projection();

        vx_dir_lights.clear();

        for(const auto& light : state.lighting.dir_lights) {
            const vec3f dir = mx_tx::rotate_z(-light.azimuth) * mx_tx::rotate_y(-light.altitude) * vec3f(1, 0, 0);

            // TODO: draw as objects (s.t. apparent size depends on FOV)
            if(state.projection.is_perspective()) {
                const vec4f pos_clip = tx_projection.mul_homo(tx_camera * ((dir * 50.0f) + state.camera.pos));

                if(pos_clip.z > 0) {
                    const vec3f pos_screen = to_screen(pos_clip.to_cartesian());

                    const QColor color = {
                            int(255 * light.color.x),
                            int(255 * light.color.y),
                            int(255 * light.color.z)
                    };

                    painter.setPen(QColor::fromHsl(color.hue(), 255, 128));

                    const int d = 1 + int(20 * light.color.norm());

                    painter.drawEllipse((int)pos_screen.x - 10, (int)pos_screen.y - 10, d, d);
                }
            }

            vx_dir_lights.push_back(dir);
        }

        collect_triangles();

        /**
         * TODO: Test rasterization top-left rule:
         *  - Create an X formation of four different-colored triangles
         *  - Rotate camera while looking down onto it against black background
         */

        render_triangles(painter, p_cursor);
    }
};

class display2d_widget : public QWidget
{
//    Q_OBJECT

    const wf_state& state;

public:
//    display2d_widget(QWidget* parent, const wf_state& state) : state(state)
    display2d_widget(QWidget* parent, wf_state& state) : state(state)
    {
        // ...
    }

    QSize sizeHint() const override
    {
        return { state.viewport.width * state.viewport.scale, state.viewport.height * state.viewport.scale };
    }

//    void resizeEvent(QResizeEvent* event) override { }

    bool hud_crosshair = true,
            hud_camera = false,
            hud_projection = true,
            hud_viewport = false,
            hud_geometry = false,
            hud_render_options = true,
            hud_performance = false;

public:
    const wf_state::th_object* hovered_object;
    bool hovered_multiple;

    void paintEvent(QPaintEvent* event) override
    {
        // TODO: write directly into pixels of a QImage, bypassing QPainter?
        QPixmap back_buffer(state.viewport.width, state.viewport.height);
        QPainter painter;

        painter.begin(&back_buffer);

        const QPoint p_cursor = mapFromGlobal(QCursor::pos());

        render3d r(state);
        r.render(painter, p_cursor / state.viewport.scale);

        // TODO: solve this shit (at least eliminate the redundancy)
        hovered_object = r.hovered_object;
        hovered_multiple = r.hovered_multiple;

        painter.end();

        painter.begin(this);

        const int scale = state.viewport.scale;
        const int w_final = state.viewport.width * scale, h_final = state.viewport.height * scale;

        painter.drawPixmap(0, 0, w_final, h_final, back_buffer);

        // Fill in the gaps which may arise from dimensions not being divisible by canvas size
        painter.fillRect(0, h_final, width(), height() - h_final, Qt::black);
        painter.fillRect(w_final, 0, width() - w_final, height(), Qt::black);

        draw_hud(painter);

        painter.end();
    }

    void draw_hud(QPainter& painter)
    {
        if(hud_crosshair) {
            draw_crosshair(painter);
        }

        painter.setOpacity(1.0);
        int y_hud = 5;

        if(!state.hovering.disabled && !state.hovering.limited && state.hovering.object != nullptr) {
            draw_object_properties_hud(painter, *state.hovering.object);
        }

        if(hud_camera) {
            y_hud += draw_camera_hud(painter, 5, y_hud);
        }

        if(hud_projection) {
            y_hud += draw_projection_hud(painter, 5, y_hud);
        }

        if(hud_viewport) {
            y_hud += draw_viewport_hud(painter, 5, y_hud);
        }

        if(hud_geometry) {
            y_hud += draw_geometry_hud(painter, 5, y_hud);
        }

        if(hud_render_options) {
            y_hud += draw_render_options_hud(painter, 5, y_hud);
        }

        if(hud_performance) {
            y_hud += draw_perf_hud(painter, 5, y_hud);
        }
    }

    void draw_crosshair(QPainter& painter)
    {
        if(!state.hovering.free_look) {
            return;
        }

        painter.setOpacity(0.75);
        painter.setPen(QPen(Qt::white, 1, Qt::DotLine));

        painter.drawArc(width() / 2 - 2, height() / 2 - 2, 5, 5, 0, 16 * 360);
    }

    QFont f_hud{"Courier"};
    QFont f_hud_small{"Courier", 10};

    void draw_object_properties_hud(QPainter& painter, const wf_state::th_object& object)
    {
        painter.setFont(f_hud);

        if(state.hovering.fixed) {
            painter.setPen(Qt::yellow);
        }
        else {
            painter.setPen(Qt::white);
        }

        QRect rect(width() / 2 - 75, height() - 60, 150, 55);
        QRect rect_top(rect.x(), rect.y() - 24, 150, 55);

        painter.drawText(rect, Qt::AlignHCenter | Qt::AlignBottom, object.id.c_str());

        if(state.hovering.mode == wf_state::INT_CARRY) {
            painter.drawText(rect_top, Qt::AlignHCenter | Qt::AlignTop, "Release E to drop");
        }
        else if(state.hovering.mode == wf_state::INT_ROTATE) {
            painter.drawText(rect_top, Qt::AlignHCenter | Qt::AlignTop, "Release F to drop");
        }

        QString text;

        QTextStream s_text(&text);
        s_text.setRealNumberNotation(QTextStream::RealNumberNotation::FixedNotation);
        s_text.setRealNumberPrecision(2);

        s_text << object.pos.x << '\n'
               << object.pos.y << '\n'
               << object.pos.z;
        painter.drawText(rect, Qt::AlignLeft, text);

        text = "";
        s_text << object.orient.x / (float)M_PI * 180 << QString::fromUtf8("°") << '\n'
               << object.orient.y / (float)M_PI * 180 << QString::fromUtf8("°") << '\n'
               << object.orient.z / (float)M_PI * 180 << QString::fromUtf8("°");
        painter.drawText(rect, Qt::AlignHCenter, text);

        painter.drawText(rect, Qt::AlignRight, "1.00\n1.00\n1.00");
    }

    int draw_camera_hud(QPainter& painter, int x, int y)
    {
        painter.setFont(f_hud);
        painter.setPen(Qt::white);

        QString text;

        QTextStream s_text(&text);
        s_text.setRealNumberNotation(QTextStream::RealNumberNotation::FixedNotation);
        s_text.setRealNumberPrecision(2);

        if(state.hovering.free_look) {
            painter.setPen(QColor::fromRgb(255, 255, 0));
            painter.drawText(QRect(5, y, 150, 55), Qt::AlignHCenter, "FREE LOOK");
            painter.setPen(Qt::white);
        }
        else {
            painter.drawText(QRect(x, y, 150, 55), Qt::AlignHCenter, "Camera");
        }

        s_text << state.camera.pos.x << '\n'
               << state.camera.pos.y << '\n'
               << state.camera.pos.z;
        painter.drawText(QRect(x, y + 20, 150, 55), Qt::AlignLeft, text);

        text = "";
        s_text.setRealNumberPrecision(1);
        s_text << state.camera.orient.x / (float)M_PI * 180 << QString::fromUtf8("°") << '\n'
               << state.camera.orient.y / (float)M_PI * 180 << QString::fromUtf8("°") << '\n'
               << state.camera.orient.z / (float)M_PI * 180 << QString::fromUtf8("°");
        painter.drawText(QRect(x, y + 20, 150, 55), Qt::AlignRight, text);

        text = "";
        s_text.setRealNumberPrecision(2);
        s_text << state.v_camera.x << '\n'
               << state.v_camera.y << '\n'
               << state.v_camera.z;
        painter.drawText(QRect(x, y + 20, 150, 55), Qt::AlignHCenter, text);

        return 70;
    }

    int draw_projection_hud(QPainter& painter, int x, int y)
    {
        painter.setFont(f_hud);
        painter.setPen(Qt::white);

        if(state.projection.is_perspective()) {
            return draw_proj_perspective(painter, x, y);
        }
        else {
            return draw_proj_parallel(painter, x, y);
        }
    }

    int draw_proj_perspective(QPainter& painter, int x, int y)
    {
        QString text;

        QTextStream s_text(&text);
        s_text.setRealNumberNotation(QTextStream::RealNumberNotation::FixedNotation);

        painter.drawText(QRect(5, y, 150, 55), Qt::AlignHCenter, "Perspective");

        const float thw_deg = state.projection.theta_w() / (float)M_PI * 180;
        const float thh_deg = state.projection.theta_h() / (float)M_PI * 180;

        s_text.setRealNumberPrecision(1);
        s_text << thw_deg << QString::fromUtf8("°") << " "
               << thh_deg << QString::fromUtf8("°") << '\n';
        s_text.setRealNumberPrecision(2);
        s_text << "[" << state.projection.z_near() << ", " << state.projection.z_far() << "]";
        painter.drawText(QRect(5, y + 20, 150, 55), Qt::AlignHCenter, text);

        return 60;
    }

    int draw_proj_parallel(QPainter& painter, int x, int y)
    {
//        painter.setPen(QColor::fromRgb(255, 140, 0));

        QString text;

        QTextStream s_text(&text);
        s_text.setRealNumberNotation(QTextStream::RealNumberNotation::FixedNotation);

        painter.drawText(QRect(5, y, 150, 55), Qt::AlignHCenter, state.projection.is_parallel() ? "Parallel" : "Orthographic");

        if(state.projection.is_parallel()) {
            s_text << "- - -";
        }
        else {
            wf_projection::ortho_axis axis = state.projection.axis();

            s_text << (axis == wf_projection::ortho_axis::X ? "X" : "-") << " "
                   << (axis == wf_projection::ortho_axis::Y ? "Y" : "-") << " "
                   << (axis == wf_projection::ortho_axis::Z ? "Z" : "-");
        }
        s_text.setRealNumberPrecision(4);
        s_text << "  " << state.projection.scale() << "x";
        painter.drawText(QRect(5, y + 20, 150, 55), Qt::AlignHCenter, text);

        if(state.projection.is_orthographic()) {
            draw_orthographic_axes(painter, state.projection.axis());
        }

        return 40;
    }

    void draw_orthographic_axes(QPainter& painter, wf_projection::ortho_axis axis)
    {
        QString up_text, right_text;
        QColor up_color, right_color;

        if(axis != wf_projection::Z) {
            up_text = "z";
            up_color = Qt::blue;
        }
        else {
            up_text = "y";
            up_color = Qt::green;
        }

        if(axis != wf_projection::X) {
            right_text = "x";
            right_color = Qt::red;
        }
        else {
            right_text = "y";
            right_color = Qt::green;
        }

        painter.setPen(up_color);
        painter.drawLine(10, height() - 11, 10, height() - 50);

        painter.setPen(right_color);
        painter.drawLine(11, height() - 10, 50, height() - 10);

        painter.setPen(Qt::white);
        painter.drawText(QRect(10, height() - 65, 15, 15), Qt::AlignVCenter | Qt::AlignLeft, up_text);
        painter.drawText(QRect(50, height() - 25, 15, 15), Qt::AlignHCenter | Qt::AlignBottom, right_text);
    }

    int draw_viewport_hud(QPainter& painter, int x, int y)
    {
        painter.setFont(f_hud);
        painter.setPen(Qt::white);

        QString text;

        QTextStream s_text(&text);
        s_text.setRealNumberNotation(QTextStream::RealNumberNotation::FixedNotation);

        painter.drawText(QRect(5, y, 150, 55), Qt::AlignHCenter, "Viewport");

        s_text.setRealNumberPrecision(0);
        s_text << state.viewport.width << " x " << state.viewport.height;
        painter.drawText(QRect(5, y + 20, 150, 55), Qt::AlignHCenter, text);

        painter.drawText(QRect(5, y + 40, 150, 55), Qt::AlignLeft, "Downscale factor");

        text = "";
        s_text << state.viewport.scale;
        painter.drawText(QRect(5, y + 40, 150, 55), Qt::AlignRight, text);

        return 65;
    }

    int draw_geometry_hud(QPainter& painter, int x, int y)
    {
        painter.setFont(f_hud);
        painter.setPen(Qt::white);

        painter.drawText(QRect(x, y, 150, 55), Qt::AlignHCenter, "Geometry");

        QString text;

        QTextStream s_text(&text);

        painter.setPen(Qt::gray);
        text = "";
        s_text << "vertices" << '\n' << "segments" << '\n' << "triangles";
        painter.drawText(QRect(x, y + 20, 150, 55), Qt::AlignLeft, text);

        painter.setPen(Qt::white);
        text = "";
        s_text << 7777 << " / " << 9999 << '\n'
               << 7777 << " / " << 9999 << '\n'
               << 7777 << " / " << 9999;
        painter.drawText(QRect(x, y + 20, 150, 55), Qt::AlignRight, text);

        return 70;
    }

    int draw_render_options_hud(QPainter& painter, int x, int y)
    {
        painter.setFont(f_hud);
        painter.setPen(Qt::white);

        painter.drawText(QRect(x, y, 150, 55), Qt::AlignHCenter, "Render options");

        QString text;

        QTextStream s_text(&text);

        text = "";
        s_text << "Occlusion" << '\n' << "Shading" << '\n' << "Lighting";
        painter.drawText(QRect(x, y + 20, 150, 55), Qt::AlignLeft, text);

        painter.setPen(Qt::white);
        text = "";
        switch(state.options.occlusion) {
            case wf_state::OCC_NONE:
                s_text << "None";
                break;
            case wf_state::OCC_BFC:
                s_text << "BFC only";
                break;
            case wf_state::OCC_BFC_ZBUF:
                s_text << "Z-buf";
        }
        s_text << '\n';
        switch(state.options.shading) {
            case wf_state::SHD_NONE:
            default:
                s_text << "None";
                break;
            case wf_state::SHD_FLAT:
                s_text << "Flat";
                break;
            case wf_state::SHD_GOURAUD:
                s_text << "Gouraud";
                break;
            case wf_state::SHD_PHONG:
                s_text << "Phong";
                break;
        }
        s_text << '\n';
        switch(state.options.lighting) {
            case wf_state::LGH_AMBIENT:
            default:
                s_text << "None";
                break;
            case wf_state::LGH_DIFFUSE:
                s_text << "Lambert";
                break;
            case wf_state::LGH_SPECULAR:
                s_text << "Phong";
                break;
        }
        painter.drawText(QRect(x, y + 20, 150, 55), Qt::AlignRight, text);

        return 70;
    }

    int draw_perf_hud(QPainter& painter, int x, int y)
    {
        painter.setFont(f_hud);
        painter.setPen(Qt::white);

        painter.drawText(QRect(x, y, 150, 55), Qt::AlignHCenter, "Performance");

        QString text;

        QTextStream s_text(&text);
        s_text.setRealNumberNotation(QTextStream::RealNumberNotation::FixedNotation);
        s_text.setRealNumberPrecision(2);

        painter.drawText(QRect(x, y + 20, 150, 55), Qt::AlignLeft, "FRAME:");

        text = "";
        s_text << state.perf_stats.sum_frame / state.perf_stats.n << " ms";
        painter.drawText(QRect(x, y + 20, 150, 55), Qt::AlignRight, text);

        painter.setPen(QColor::fromRgb(180, 180, 180));

        text = "";
        s_text << "Update:" << '\n' << "Vertices:" << '\n' << "Lines:";
        painter.drawText(QRect(x, y + 35, 150, 55), Qt::AlignLeft, text);

        text = "";
        s_text << "0.00 ms" << '\n'
               << state.perf_stats.sum_transform / state.perf_stats.n << " ms" << '\n'
               << state.perf_stats.sum_lines / state.perf_stats.n << " ms";
        painter.drawText(QRect(x, y + 35, 150, 55), Qt::AlignRight, text);

        return y + 35;
    }
};

#endif //DZ_GAIKA_DISPLAY2D_HPP
