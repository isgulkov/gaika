
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

class render3d
{
    const wf_state& state;

    int zb_width;
    std::vector<float> z_buffer;

public:
    const wf_state::th_object* hovered_object;
    bool hovered_multiple;

public:
    render3d(const wf_state& state) : state(state) { }

    static void draw_line(QPainter& painter, vec3f a, vec3f b)
    {
        painter.drawLine(a.x, a.y, b.x, b.y);
    }

    void draw_triangle_flat_top(QPainter& painter, vec3f a, vec3f b, vec3f c)
    {
        const float alpha_left = (c.x - a.x) / (c.y - a.y);
        const float alpha_right = (c.x - b.x) / (c.y - b.y);

        const float dz_left = (c.z - a.z) / (c.y - a.y);
        const float dz_right = (c.z - b.z) / (c.y - b.y);

        const int y_start = (int)std::ceil(a.y - 0.5f), y_end = (int)std::ceil(c.y - 0.5f);

        float z_left = a.z + dz_left * (y_start - a.y);
        float z_right = b.z + dz_right * (y_start - b.y);

        for(int y = y_start; y < y_end; y++) {
            const float x_left = a.x + alpha_left * (y - a.y + 0.5f);
            const float x_right = b.x + alpha_right * (y - b.y + 0.5f);

            const float x_start = std::ceil(x_left - 0.5f);
            const float x_end = std::ceil(x_right - 0.5f);

            const float dz_line = (z_right - z_left) / (x_right - x_left);
            float z = z_left + dz_line * (x_start - x_left);

            for(int x = (int)x_start; x < x_end; x++) {
                float& z_value = z_buffer[x + y * zb_width];

                if(z < z_value) {
                    painter.drawPoint(x, y);
                    z_value = z;
                }

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

        const int y_start = (int)std::ceil(a.y - 0.5f), y_end = (int)std::ceil(c.y - 0.5f);

        float z_left = a.z + dz_left * (y_start - a.y);
        float z_right = a.z + dz_right * (y_start - a.y);

        for(int y = y_start; y < y_end; y++) {
            const float x_left = a.x + alpha_left * (y + 0.5f - a.y);
            const float x_right = a.x + alpha_right * (y + 0.5f - a.y);

            const float x_start = std::ceil(x_left - 0.5f);
            const float x_end = std::ceil(x_right - 0.5f);

            const float dz_line = (z_right - z_left) / (x_right - x_left);
            float z = z_left + dz_line * (x_start - x_left);

            for(int x = (int)x_start; x < x_end; x++) {
                float& z_value = z_buffer[x + y * zb_width];

                if(z < z_value) {
                    painter.drawPoint(x, y);
                    z_value = z;
                }

                z += dz_line;
            }

            z_left += dz_left;
            z_right += dz_right;
        }
    }

protected:
    void draw_triangle(QPainter& painter, vec3f a, vec3f b, vec3f c)
    {
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

    vec3f to_screen(const vec3f& v)
    {
        return {
                (v.x / 2.0f + 0.5f) * state.viewport.width,
                (-v.y / 2.0f + 0.5f) * state.viewport.height,
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
        const float hw_ratio = (float)state.viewport.height / state.viewport.width;

        if(state.projection.is_orthographic()) {
            return mx_tx::scale(state.projection.scale()) * mx_tx::scale(hw_ratio, 1, 0);
        }

        // TODO: reconcile the scale with the Z mapping (right now the far plane seems a bit close)
        return mx_tx::scale(1, 1, -1.0f / (state.projection.z_far() - state.projection.z_near()))
               * mx_tx::translate(0, 0, -state.projection.z_near())
               * mx_tx::scale(state.projection.scale(), state.projection.scale(), 1)
               * mx_tx::scale(hw_ratio, 1, 1);
    }

public:
    void render(QPainter& painter, QPoint p_cursor)
    {
        painter.fillRect(0, 0, state.viewport.width, state.viewport.height, Qt::black);

        const mat_sq4f tx_camera = create_tx_camera();
        const mat_sq4f tx_projection = create_tx_projection();

        hovered_object = nullptr;
        hovered_multiple = false;

        /**
         * TODO: reorganize everything
         *
         * 1. Assemble world vertices and normals
         * 2. Assemble triangles
         *    - culling backfaces along the way
         *    - for each vertex:
         *      - world coords
         *      - with Phong shading:
         *        - world normal (default to face normal)
         *        - material reference
         *      - without Phong shading:
         *        - final color
         * 3. Transfer (just the vertices) to clipping space
         * 4. Clip faces
         * 5. Rasterize
         * 6. Display on the canvas, add HUD
         *
         * TODO: see if a uint16_t z-buffer is any faster
         */
        zb_width = state.viewport.width;
        z_buffer.resize((size_t)(zb_width * state.viewport.height));
        std::fill(z_buffer.begin(), z_buffer.end(), MAXFLOAT);

        // TODO: support multiple directional lights
        const wf_state::dir_light& sun = state.dir_lights.back();

        const vec3f dir_sun = mx_tx::rotate_z(-sun.azimuth) * mx_tx::rotate_y(-sun.altitude) * vec3f(1, 0, 0);

        // TODO: move where other light sources will be drawn
        if(state.projection.is_perspective()) {
            const vec3f sun_pos = (dir_sun * 50.0f) + state.camera.pos;

            const vec4f sun_clip = tx_projection.mul_homo(tx_camera * sun_pos);

            if(sun_clip.z > 0) {
                const vec3f hui = to_screen(sun_clip.to_cartesian());

                painter.setPen(Qt::yellow);
                painter.drawEllipse((int)hui.x - 10, (int)hui.y - 10, 20, 20);
            }
        }

        // TODO: benchmark, investigate possible CPU parallelism (both SIMD and threads)
        for(const wf_state::th_object& object : state.th_objects) {
            if(object.vertices_world.empty()) {
                const mat_sq4f tx_world = mx_tx::translate(object.pos) * mx_tx::rotate_xyz(object.orient) * mx_tx::scale(object.scale);

                object.vertices_world = tx_world * object.model->vertices;
            }

            // TODO: decide whether to copy stuff over instead of setting flags
            std::vector<char> tri_culled;
            std::vector<float> tri_sunlight;

            if(state.options.use_backface_cull != wf_state::BFC_DISABLE) {
                // TODO: extract all this mess
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

                tri_culled.resize(object.model->faces.size(), false);
                tri_sunlight.resize(object.model->faces.size());
                size_t i_triangle = tri_culled.size() - 1;

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

                    tri_culled[i_triangle] = dir_out.dot(tri_norm) >= 0;
                    tri_sunlight[i_triangle] = std::max(dir_sun.dot(tri_norm), 0.0f);

                    i_triangle -= 1;
                }
            }

            const std::vector<vec4f> vertices_clipping = tx_projection.mul_homo(tx_camera * object.vertices_world);

            std::vector<uint8_t> vertex_outcodes(vertices_clipping.size(), 0);
            std::vector<vec3f> vertices_screen;
            vertices_screen.reserve(vertices_clipping.size());

            for(const vec4f& vertex : vertices_clipping) {
                uint8_t outcode = 0;

                outcode |= (vertex.x > vertex.w);
                outcode |= (vertex.x < -vertex.w) << 1;
                outcode |= (vertex.y > vertex.w) << 2;
                outcode |= (vertex.y < -vertex.w) << 3;
                outcode |= (vertex.z > vertex.w) << 4;
                outcode |= (vertex.z < 0) << 5;

                vertex_outcodes[vertices_screen.size()] = outcode;

                // TODO: gut to_screen after removing segment rendering
                vertices_screen.push_back(to_screen(vertex.to_cartesian()));
            }

            QPen face_pen(Qt::white);

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

            if(!state.hovering.fixed) {
                object.hovered = false;
            }

            // Segment rendering code was here

            for(const auto& triangle : object.model->faces) {
                painter.setOpacity(state.projection.is_orthographic() ? 0.75 : 1);

                const float face_sunlight = state.projection.is_orthographic() ? 1 : tri_sunlight.back();
                tri_sunlight.pop_back();

                if(state.options.use_backface_cull != wf_state::BFC_DISABLE) {
                    const bool face_culled = tri_culled.back();
                    tri_culled.pop_back();

                    if(face_culled) {
                        if(state.options.use_backface_cull == wf_state::BFC_TRANSPARENT) {
                            painter.setOpacity(0.2);
                        }
                        else {
                            continue;
                        }
                    }
                }

                if(vertex_outcodes[triangle.i_a] != 0 || vertex_outcodes[triangle.i_b] != 0 || vertex_outcodes[triangle.i_c] != 0) {
                    continue;
                }

                const vec3f a = vertices_screen[triangle.i_a], b = vertices_screen[triangle.i_b], c = vertices_screen[triangle.i_c];

                // TODO: add ambient lighting and shit
                const vec3f c_final = 255.0f * object.model->materials[triangle.i_mtl].c_diffuse * (
                        (sun.color * sun.intensity * face_sunlight * 0.9f) + vec3f(0.1f, 0.1f, 0.1f)
                );

                QColor color { int(c_final.x), int(c_final.y), int(c_final.z) };

                face_pen.setColor(color);
                painter.setPen(face_pen);

                if(state.projection.is_orthographic()) {
                    draw_line(painter, a, b);
                    draw_line(painter, a, c);
                    draw_line(painter, b, c);
                }
                else {
                    draw_triangle(painter, a, b, c);
                }

                if(state.hovering.disabled || state.hovering.fixed) {
                    continue;
                }

                if(!object.hovered && object.hoverable && test_p_in_triangle(p_cursor, a, b, c)) {
                    object.hovered = true;

                    if(!hovered_multiple && hovered_object != nullptr) {
                        hovered_multiple = true;
                        hovered_object = nullptr;
                    }

                    hovered_object = &object;
                }
            }
        }
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
        painter.drawPixmap(0, 0, state.viewport.width * scale, state.viewport.height * scale, back_buffer);

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
            painter.drawText(rect_top, Qt::AlignHCenter | Qt::AlignTop, "Press E to drop");
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

        return 45;
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
        s_text << "Backface cull" << '\n' << "Hidden line rem.";
        painter.drawText(QRect(x, y + 20, 150, 55), Qt::AlignLeft, text);

        painter.setPen(Qt::white);
        text = "";
        switch(state.options.use_backface_cull) {
            case wf_state::BFC_DISABLE:
                s_text << "off";
                break;
            case wf_state::BFC_TRANSPARENT:
                s_text << "ON";
                break;
            case wf_state::BFC_CULL:
                s_text << "HIDE";
        }
        s_text << '\n' << "--";
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
