
#ifndef DZ_GAIKA_DISPLAY2D_HPP
#define DZ_GAIKA_DISPLAY2D_HPP

#include <utility>
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
    void paintEvent(QPaintEvent* event) override
    {
        QPainter painter;

        painter.begin(this);
        painter.setRenderHint(QPainter::Antialiasing);

//        painter.setBrush(QBrush(Qt::white));

//        painter.eraseRect(0, 0, width, height);
        painter.fillRect(0, 0, state.viewport.width, state.viewport.height, Qt::black);

        //const
        mat_sq4f tx_camera = mx_tx::translate(-state.camera.pos);

        if(!state.projection.is_orthographic()) {
            tx_camera = mx_tx::rotate_xyz(state.camera.orient).transpose() * tx_camera;
        }
        else {
            // TODO: why isn't this done inside tx_project? (the necessity of the latter is itself questionable, though)
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

        if(!state.projection.is_perspective()) {
            /**
             * Squeeze along the horizontal axis (camera's X) to compensate for distortion from displaying the square
             * visibility box on a non-square viewport.
             *
             * This doesn't make sense for perspective projection, where the the image plane's aspect ratio translates
             * into a non-linear relationship between horizontal and vertical angles of view.
             */

            tx_camera = mx_tx::scale_xyz((float)state.viewport.height / state.viewport.width, 1, 1) * tx_camera;
        }

        const mat_sq4f tx_projection = state.projection.tx_project();

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

            std::vector<vec3f> vertices_camera = tx_camera * object.vertices_world;

            std::vector<int> is_behind_camera;
            is_behind_camera.reserve(vertices_camera.size());

            const float z_clip = state.projection.is_perspective() ? -state.projection.z_near() : 0;

            for(const vec3f& vertex : vertices_camera) {
                is_behind_camera.push_back(!state.projection.is_orthographic() && vertex.z > z_clip);
            }

            std::vector<vec3f> vertices_projected = tx_projection * vertices_camera;

            std::vector<vec2i> vertices_screen;
            vertices_screen.reserve(vertices_projected.size());

            // TODO: isn't there an STL algorithm for this?
            for(const vec3f& vertex : vertices_projected) {
                vertices_screen.emplace_back(to_screen(vertex));
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
                /**
                 * This is partially broken for perspective projection — artifacts appear with varying frequency,
                 * depending on z_near.
                 *
                 * As it turns out, it's best to do the clipping after multiplying by the perspective matrix, but before
                 * dividing the coords by w. Ouch! So, I now have to either
                 *   - rewrite vec3f into vec4f, which frankly seems wasteful, or
                 *   - rework the camera->screen process into two steps specifically for perspective, which would,
                 *     besides looking stupid, require a ton of refactoring due to perspective's exclusivity.
                 *
                 * TODO: fix for perspective, then set default z_near to 0.1
                 */

                if(is_behind_camera[segment.first] && is_behind_camera[segment.second]) {
                    continue;
                }

                if(!is_behind_camera[segment.first] && !is_behind_camera[segment.second]) {
                    draw_line(painter, vertices_screen[segment.first], vertices_screen[segment.second]);

                    continue;
                }

                uint32_t i_a = segment.first, i_b = segment.second;

                if(is_behind_camera[i_a]) {
                    std::swap(i_a, i_b);
                }

                const vec3f& a = vertices_camera[i_a], b = vertices_camera[i_b];

                const float t = (z_clip - a.z) / (b.z - a.z);
                const vec3f c = { a.x + t * (b.x - a.x), a.y + t * (b.y - a.y), z_clip };

                draw_line(painter, vertices_screen[i_a], to_screen(tx_projection * c));
            }

            for(const auto& triangle : object.model->triangles) {
                if(is_behind_camera[std::get<0>(triangle)] || is_behind_camera[std::get<1>(triangle)] || is_behind_camera[std::get<2>(triangle)]) {
                    continue;
                }

                painter.setOpacity(0.375); // Every segment gets drawn twice

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
