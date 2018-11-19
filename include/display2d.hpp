
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
#include "viewer_state.hpp"

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

        const mat_sq4f tx_camera = mx_tx::rotate_xyz(-state.camera.orient) * mx_tx::translate(-state.camera.pos);
        const mat_sq4f tx_projection = mx_tx::perspective_z(state.perspective.theta_w,
                                                            state.perspective.wh_ratio,
                                                            state.perspective.z_near,
                                                            state.perspective.z_far);

        for(wf_state::th_object object : state.th_objects) {
            const mat_sq4f tx_world = mx_tx::rotate_xyz(object.orient) * mx_tx::translate(object.pos);

            std::vector<vec3f> vertices_camera = tx_camera * tx_world * object.model->vertices;

            bool is_behind = false;

            for(const vec3f& vertex : vertices_camera) {
                if(vertex.z() > 0) {
                    is_behind = true;
                    break;
                }
            }

            if(is_behind) {
                break;
            }

            std::vector<vec3f> vertices_projected = tx_projection * vertices_camera;

            std::vector<vec2i> vertices_screen;
            vertices_screen.reserve(vertices_projected.size());

            for(const vec3f& vertex : vertices_projected) {
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

        painter.setOpacity(1.0);
        int y_hud = 5;

        if(hud_camera) {
            y_hud += draw_camera_hud(painter, 5, y_hud);
        }

        if(hud_perspective) {
            y_hud += draw_perspective_hud(painter, 5, y_hud);
        }

        painter.end();
    }

    QFont f_hud{"Courier"};
    QFont f_hud_small{"Courier", 10};

    int draw_camera_hud(QPainter& painter, int x, int y)
    {
        painter.setFont(f_hud);
        painter.setPen(QPen(Qt::white, 1));

        QString text;

        QTextStream s_text(&text);
        s_text.setRealNumberNotation(QTextStream::RealNumberNotation::FixedNotation);
        s_text.setRealNumberPrecision(2);

        painter.drawText(QRect(x, y, 90, 55), Qt::AlignHCenter, "Camera");

        s_text << state.camera.pos.x() << '\n'
               << state.camera.pos.y() << '\n'
               << state.camera.pos.z();
        painter.drawText(QRect(x, y + 20, 90, 55), Qt::AlignLeft, text);

        text = "";
        s_text.setRealNumberPrecision(1);
        s_text << state.camera.orient.x() / (float)M_PI * 180 << QString::fromUtf8("°") << '\n'
               << state.camera.orient.y() / (float)M_PI * 180 << QString::fromUtf8("°") << '\n'
               << state.camera.orient.z() / (float)M_PI * 180 << QString::fromUtf8("°");
        painter.drawText(QRect(x, y + 20, 90, 55), Qt::AlignRight, text);

        text = "";
        s_text.setRealNumberPrecision(2);
        s_text << state.v_camera.x() << '\n'
               << state.v_camera.y() << '\n'
               << state.v_camera.z();
        painter.drawText(QRect(x, y + 20, 150, 55), Qt::AlignRight, text);

        return 75;
    }

    int draw_perspective_hud(QPainter& painter, int x, int y)
    {
        painter.setFont(f_hud);
        painter.setPen(QPen(Qt::white, 1));

        // TODO: draw vectors as columns

        QString text;

        QTextStream s_text(&text);
        s_text.setRealNumberNotation(QTextStream::RealNumberNotation::FixedNotation);

        painter.drawText(QRect(5, y, 90, 55), Qt::AlignHCenter, "Perspective");

        float thw_deg = state.perspective.theta_w / (float)M_PI * 180;
        float thh_deg = thw_deg / state.perspective.wh_ratio;

        s_text.setRealNumberPrecision(1);
        s_text << thw_deg << QString::fromUtf8("°") << " "
               << thh_deg << QString::fromUtf8("°") << '\n';
        s_text.setRealNumberPrecision(0);
        s_text << "[" << state.perspective.z_near << "," << state.perspective.z_far << "]";
        painter.drawText(QRect(5, y + 20, 90, 55), Qt::AlignHCenter, text);

        return 40;
    }
};

#endif //DZ_GAIKA_DISPLAY2D_HPP
