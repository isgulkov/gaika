
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
        return { state.viewport.width, state.viewport.height };
    }

//    void resizeEvent(QResizeEvent* event) override { }

    bool hud_crosshair = true,
            hud_camera = false,
            hud_projection = true,
            hud_viewport = false,
            hud_geometry = false,
            hud_render_options = true,
            hud_performance = false;

protected:
    static void draw_line(QPainter& painter, vec2i a, vec2i b)
    {
        painter.drawLine(a.x, a.y, b.x, b.y);
    }

    /**
     * Map @param x from [-1; 1] to [0; @param x_size], where 0 becomes @code{x_size / 2}
     */
    static int scale_screen(float x, int x_size)
    {
        return (int16_t)((x / 2.0f + 0.5f) * x_size);
    }

    vec2i to_screen(const vec3f& v)
    {
        return { scale_screen(v.x, state.viewport.width), scale_screen(-v.y, state.viewport.height) };
    }

    vec3f from_screen(const vec2i& v)
    {
        // TODO: write a vec2f for this?
        return { 2.0f * v.x / state.viewport.width - 1.0f, -2.0f * y() / state.viewport.height + 1.0f, 0 };
    }

    static bool test_p_in_triangle(const vec2i& p, const vec2i& a, const vec2i& b, const vec2i& c)
    {
        // TODO: figure out how this works and document
        const int x_ap = p.x - a.x;
        const int y_ap = p.y - a.y;

        const bool dp_ab = (b.x - a.x) * y_ap - (b.y - a.y) * x_ap > 0;
        const bool dp_ac = (c.x - a.x) * y_ap - (c.y - a.y) * x_ap > 0;

        if(dp_ab == dp_ac) {
            return false;
        }

        const bool gandon = (c.x - b.x) * (p.y - b.y) - (c.y - b.y) * (p.x - b.x) > 0;

        return gandon == dp_ab;
    }

public:
    const wf_state::th_object* hovered_object;
    bool hovered_multiple;

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
         * Squeeze along the horizontal axis (camera's X) to compensate for distortion from displaying the square
         * visibility box on a non-square viewport
         *
         * This doesn't make sense for perspective projection, where the the image plane's aspect ratio translates
         * into a non-linear relationship between horizontal and vertical angles of view
         */
        mat_sq4f tx_project = mx_tx::scale(state.projection.scale()) * mx_tx::scale((float)state.viewport.height / state.viewport.width, 1, 1);

        if(state.projection.is_parallel()) {
            /**
             * Rescale the Z axis from [near; far] like in perspective (for clipping)
             */
            tx_project = mx_tx::scale(1, 1, -1.0f / (state.projection.z_far() - state.projection.z_near()))
                    * mx_tx::translate(0, 0, state.projection.z_near()) * tx_project;
        }

        return tx_project;
    }

    void paintEvent(QPaintEvent* event) override
    {
        QPainter painter;

        painter.begin(this);
        painter.setRenderHint(QPainter::Antialiasing);

//        painter.setBrush(QBrush(Qt::white));

//        painter.eraseRect(0, 0, width, height);
        painter.fillRect(0, 0, state.viewport.width, state.viewport.height, Qt::black);

        /**
         * Camera:
         *   - cam. rotation | special rotations (orthos, each)
         *   - none (perspective) | scale for aspect ratio
         * Projection:
         *   - for perspective -- mx into (x, y, z, w), just Z=0 for everything else
         *
         * Plane clip:
         *   - z_near | 0 | none
         */

        const mat_sq4f tx_camera = create_tx_camera();
        const mat_sq4f tx_projection = create_tx_projection();

        hovered_object = nullptr;
        hovered_multiple = false;

        const QPoint p_cursor = mapFromGlobal(QCursor::pos());
        const vec2i p{ p_cursor.x(), p_cursor.y() };

        /**
         * TODO: benchmark, investigate possible CPU parallelism
         *
         * 1. Split by object
         * 2. Merge all vertices into one large vector (transforming index collections), then split in chunks
         */
        for(const wf_state::th_object& object : state.th_objects) {
            if(object.vertices_world.empty()) {
                const mat_sq4f tx_world = mx_tx::translate(object.pos) * mx_tx::rotate_xyz(object.orient) * mx_tx::scale(object.scale);

                object.vertices_world = tx_world * object.model->vertices;
            }

            // TODO: decide whether to copy stuff over instead of setting flags
            std::vector<char> tri_culled;

            if(state.options.use_backface_cull) {
                tri_culled.resize(object.model->triangles.size(), false);
                size_t i_triangle = tri_culled.size() - 1;

                for(const auto& triangle : object.model->triangles) {
                    const vec3f& a = object.vertices_world[std::get<0>(triangle)],
                            b = object.vertices_world[std::get<1>(triangle)],
                            c = object.vertices_world[std::get<2>(triangle)];

                    tri_culled[i_triangle--] = (state.camera.pos - a) * (b - a).cross(c - a) >= 0;
                }
            }

            const std::vector<vec4f> vertices_clipping = tx_projection.mul_homo(tx_camera * object.vertices_world);

            std::vector<uint8_t> vertex_outcodes(vertices_clipping.size(), 0);
            std::vector<vec2i> vertices_screen;
            vertices_screen.reserve(vertices_clipping.size());

            for(const vec4f& vertex : vertices_clipping) {
                uint8_t outcode = 0;

                outcode |= (vertex.y > vertex.w);
                outcode |= (vertex.y < -vertex.w) << 1;
                outcode |= (vertex.x > vertex.w) << 2;
                outcode |= (vertex.x < -vertex.w) << 3;
                outcode |= (vertex.z > vertex.w) << 4;
                outcode |= (vertex.z < -vertex.w) << 5;

                vertex_outcodes[vertices_screen.size()] = outcode;

                vertices_screen.push_back(to_screen(vertex.to_cartesian()));
            }

            if(!object.hovered || state.options.hovering_disabled) {
                painter.setPen(object.color);
            }
            else {
                if(state.options.hovering_limited) {
                    QPen pen(object.color);
                    pen.setDashPattern({ 1, 5 });

                    painter.setPen(pen);
                }
                else {
                    painter.setPen(QPen(object.color, 3, Qt::DashLine));
                }
            }

            object.hovered = false;

            painter.setOpacity(0.75);

            for(const auto& segment : object.model->segments) {
                if(vertex_outcodes[segment.first] != 0 || vertex_outcodes[segment.second] != 0) {
                    continue;
                }

                draw_line(painter, vertices_screen[segment.first], vertices_screen[segment.second]);

//                uint32_t i_a = segment.first, i_b = segment.second;
//
//                if(is_behind_camera[i_a]) {
//                    std::swap(i_a, i_b);
//                }
//
//                const vec3f& a = vertices_camera[i_a], b = vertices_camera[i_b];
//
//                const float t = (z_clip - a.z) / (b.z - a.z);
//                const vec3f c = { a.x + t * (b.x - a.x), a.y + t * (b.y - a.y), z_clip };
//
//                draw_line(painter, vertices_screen[i_a], to_screen(tx_projection * c));
            }

            for(const auto& triangle : object.model->triangles) {
                if(state.options.use_backface_cull) {
                    painter.setOpacity(tri_culled.back() ? 0.25 : 0.75);
                    tri_culled.pop_back();
                }

                if(vertex_outcodes[std::get<0>(triangle)] != 0 || vertex_outcodes[std::get<1>(triangle)] != 0 || vertex_outcodes[std::get<2>(triangle)] != 0) {
                    continue;
                }

                const vec2i a = vertices_screen[std::get<0>(triangle)],
                        b = vertices_screen[std::get<1>(triangle)],
                        c = vertices_screen[std::get<2>(triangle)];

                draw_line(painter, a, b);
                draw_line(painter, a, c);
                draw_line(painter, b, c);

                if(state.options.hovering_disabled) {
                    continue;
                }

                if(!object.hovered && object.hoverable && test_p_in_triangle(p, a, b, c)) {
                    object.hovered = true;

                    if(!hovered_multiple && hovered_object != nullptr) {
                        hovered_multiple = true;
                        hovered_object = nullptr;
                    }

                    hovered_object = &object;
                }
            }
        }

        if(hud_crosshair) {
            draw_crosshair(painter);
        }

        painter.setOpacity(1.0);
        int y_hud = 5;

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

        painter.end();
    }

    void draw_crosshair(QPainter& painter)
    {
        if(!state.options.free_look) {
            return;
        }

        painter.setOpacity(0.75);
        painter.setPen(QPen(Qt::white, 1, Qt::DotLine));

        painter.drawArc(state.viewport.width / 2 - 2, state.viewport.height / 2 - 2, 5, 5, 0, 16 * 360);
    }

    QFont f_hud{"Courier"};
    QFont f_hud_small{"Courier", 10};

    int draw_camera_hud(QPainter& painter, int x, int y)
    {
        painter.setFont(f_hud);
        painter.setPen(Qt::white);

        QString text;

        QTextStream s_text(&text);
        s_text.setRealNumberNotation(QTextStream::RealNumberNotation::FixedNotation);
        s_text.setRealNumberPrecision(2);

        if(state.options.free_look) {
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

        return 40;
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
        s_text << "Backface cull" << '\n' << "Segment clip" << '\n' << "Polygon clip";
        painter.drawText(QRect(x, y + 20, 150, 55), Qt::AlignLeft, text);

        painter.setPen(Qt::white);
        text = "";
        s_text << (state.options.use_backface_cull ? "ON" : "off") << '\n' << "--" << '\n' << "--";
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
