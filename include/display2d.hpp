
#ifndef DZ_GAIKA_DISPLAY2D_HPP
#define DZ_GAIKA_DISPLAY2D_HPP

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

    struct model_tri {
        const std::vector<vec3f> vertices;
        const std::vector<std::array<size_t, 3>> faces;
    };

    struct object_tri {
        const model_tri& model;
        QColor color;
        vec3f translate, orient;
    };

    std::vector<object_tri> objects;

    struct {
        vec3f pos, dir;
    } camera;

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
    display2d_widget(QWidget* parent, const wf_state& state) : state(state)
    {
        setFixedSize(state.viewport.width, state.viewport.height);
    }

    bool chlen = true;
    int huii = 0;

public slots:
    void animate()
    {
        update();
    };

protected:

    void paintEvent(QPaintEvent* event) override
    {
        QPainter painter;

        painter.begin(this);
        painter.setRenderHint(QPainter::Antialiasing);

//        painter.setBrush(QBrush(Qt::white));

//        painter.eraseRect(0, 0, width, height);
        painter.fillRect(0, 0, state.viewport.width, state.viewport.height, Qt::black);

        painter.setPen(QPen(Qt::red, 1));
        painter.drawRect(10 + huii, 10 + huii, state.viewport.width - 20 - 2 * huii, state.viewport.height - 20 - 2 * huii);

        if(chlen) {
            painter.setFont(QFont("Courier"));
            painter.setPen(QPen(Qt::white, 1));

            // TODO: draw vectors as columns

            QString text;

            QTextStream s_text(&text);
            s_text.setRealNumberNotation(QTextStream::RealNumberNotation::FixedNotation);
            s_text.setRealNumberPrecision(2);

//            s_cam_info << "Camera: (" << state.camera.pos.x() << ", "
//                                    << state.camera.pos.x() << ", "
//                                    << state.camera.pos.x() << ") -> ("
//                                    << state.camera.dir.x() << ", "
//                                    << state.camera.dir.x() << ", "
//                                    << state.camera.dir.x() << ")";

            painter.drawText(QRect(5, 5, 90, 55), Qt::AlignHCenter, "Camera");

            s_text << state.camera.pos.x() << '\n'
                   << state.camera.pos.y() << '\n'
                   << state.camera.pos.z();
            painter.drawText(QRect(5, 25, 90, 55), Qt::AlignLeft, text);

            text = "";
            s_text << state.camera.dir.x() << '\n'
                   << state.camera.dir.y() << '\n'
                   << state.camera.dir.z();
            painter.drawText(QRect(5, 25, 90, 55), Qt::AlignRight, text);
        }

        painter.end();
    }
};

#endif //DZ_GAIKA_DISPLAY2D_HPP
