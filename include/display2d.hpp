
#ifndef DZ_GAIKA_DISPLAY2D_HPP
#define DZ_GAIKA_DISPLAY2D_HPP

#include <cmath>
#include <iostream> // REMOVE: !

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

struct wf_state
{
    // REMOVE: move to another file?

    // TODO: implement other objects (triangle- and segment-based)
    struct th_model {
        // In order A, B, C, P where (A, B, C) is clockwise when looking in P's direction
        const std::vector<vec3f> vertices;
    };

    struct th_object {
        th_model model; // TODO: reference
        QColor color;
        vec3f pos, orient;
    };

    std::vector<th_object> th_objects;

    struct {
        vec3f pos, orient;
    } camera, v_camera;

    // TODO: orthographic perspectives
    struct {
        float theta_w, wh_ratio, z_near, z_far;
    } perspective;

    struct {
        int width, height;
    } viewport = { 640, 480 };
};

class display2d_widget : public QWidget
{
//    Q_OBJECT

    const wf_state& state;

public:
//    display2d_widget(QWidget* parent, const wf_state& state) : state(state)
    display2d_widget(QWidget* parent, wf_state& state) : state(state)
    {
        setFixedSize(state.viewport.width, state.viewport.height);
    }

    bool hud_camera = true;
    bool hud_perspective = true;

protected:
    static void draw_line(QPainter& painter, vec2i a, vec2i b)
    {
        painter.drawLine(a.x(), a.y(), b.x(), b.y());
    }

    void paintEvent(QPaintEvent* event) override
    {
        QPainter painter;

        painter.begin(this);
        painter.setRenderHint(QPainter::Antialiasing);

//        painter.setBrush(QBrush(Qt::white));

//        painter.eraseRect(0, 0, width, height);
        painter.fillRect(0, 0, state.viewport.width, state.viewport.height, Qt::black);

        painter.setOpacity(0.75);

        float z_min = 1000, z_max = -1000;

        const mat_sq4f tx_camera = mx_tx::rotate_xyz(-state.camera.orient) * mx_tx::translate(-state.camera.pos);
        const mat_sq4f tx_projection = mx_tx::perspective_z(state.perspective.theta_w,
                                                            state.perspective.wh_ratio,
                                                            state.perspective.z_near,
                                                            state.perspective.z_far);

        for(wf_state::th_object object : state.th_objects) {
            const mat_sq4f tx_world = mx_tx::rotate_xyz(object.orient) * mx_tx::translate(object.pos);

            std::vector<vec3f> vertices_camera = tx_camera * mx_tx::scale(1.0f / state.camera.pos.z()) * tx_world * object.model.vertices;

            for(vec3f h : tx_projection * vertices_camera) {
                z_min = std::min(h.x(), z_min);
                z_max = std::max(h.x(), z_max);
            }

            std::vector<vec2i> vertices_screen;
            vertices_screen.reserve(vertices_camera.size());

            for(const vec3f& vertex : tx_projection * vertices_camera) {
                vertices_screen.emplace_back(vertex.onto_xy_screen(state.viewport.width, state.viewport.height));
            }

            painter.setPen(object.color);
            draw_line(painter, vertices_screen[0], vertices_screen[1]);
            draw_line(painter, vertices_screen[1], vertices_screen[2]);
            draw_line(painter, vertices_screen[0], vertices_screen[2]);
            draw_line(painter, vertices_screen[0], vertices_screen[3]);
            draw_line(painter, vertices_screen[1], vertices_screen[3]);
            draw_line(painter, vertices_screen[2], vertices_screen[3]);
        }

//        std::cout << z_min << " " << z_max << '\n';

        if(hud_camera) {
            painter.setFont(QFont("Courier"));
            painter.setPen(QPen(Qt::white, 1));
            painter.setOpacity(1.0);

            // TODO: draw vectors as columns

            QString text;

            QTextStream s_text(&text);
            s_text.setRealNumberNotation(QTextStream::RealNumberNotation::FixedNotation);
            s_text.setRealNumberPrecision(2);

            painter.drawText(QRect(5, 5, 90, 55), Qt::AlignHCenter, "Camera");

            s_text << state.camera.pos.x() << '\n'
                   << state.camera.pos.y() << '\n'
                   << state.camera.pos.z();
            painter.drawText(QRect(5, 25, 90, 55), Qt::AlignLeft, text);

            text = "";
            s_text.setRealNumberPrecision(1);
            s_text << state.camera.orient.x() / (float)M_PI * 180 << QString::fromUtf8("°") << '\n'
                   << state.camera.orient.y() / (float)M_PI * 180 << QString::fromUtf8("°") << '\n'
                   << state.camera.orient.z() / (float)M_PI * 180 << QString::fromUtf8("°");
            painter.drawText(QRect(5, 25, 90, 55), Qt::AlignRight, text);
        }

        if(hud_perspective) {
            painter.setFont(QFont("Courier"));
            painter.setPen(QPen(Qt::white, 1));
            painter.setOpacity(1.0);

            // TODO: draw vectors as columns

            QString text;

            QTextStream s_text(&text);
            s_text.setRealNumberNotation(QTextStream::RealNumberNotation::FixedNotation);

            painter.drawText(QRect(5, 80, 90, 55), Qt::AlignHCenter, "Perspective");

            float thw_deg = state.perspective.theta_w / (float)M_PI * 180;
            float thh_deg = thw_deg / state.perspective.wh_ratio;

            s_text.setRealNumberPrecision(1);
            s_text << thw_deg << QString::fromUtf8("°") << " " << thh_deg << QString::fromUtf8("°") << '\n';
            s_text.setRealNumberPrecision(0);
            s_text << "[" << state.perspective.z_near << "," << state.perspective.z_far << "]";
            painter.drawText(QRect(5, 100, 90, 55), Qt::AlignHCenter, text);
        }

        painter.end();
    }
};

#endif //DZ_GAIKA_DISPLAY2D_HPP
