
#include <vector>
#include <array>
#include <list>
#include <cmath>
#include <iostream>  // REMOVE: !

#include <QApplication>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QStackedLayout>
#include <QSlider>
#include <include/display2d.hpp>

#include "matrices.hpp"
#include "geometry.hpp"
#include "display2d.hpp"

class wf_viewer : public QWidget
{
//    Q_OBJECT

    wf_state state;

public:
    wf_viewer()
    {
        setWindowTitle("Hello, world!");

        p_widget = new display2d_widget(this, state);

        auto* slider_theta_w = new QSlider(Qt::Horizontal);
        slider_theta_w->setMinimum(0);
        slider_theta_w->setMaximum(180);
        slider_theta_w->setValue(50);
        connect(slider_theta_w, &QSlider::valueChanged, this, &wf_viewer::shitslide);

        auto* layout = new QHBoxLayout;
        layout->setSpacing(0);
        layout->setMargin(0);

        layout->addWidget(p_widget);
        layout->addWidget(slider_theta_w);
        setLayout(layout);

        last_tick.start();
        QTimer::singleShot(0, this, &wf_viewer::tick);

        /**
         * A public data structure with the complete state, which is passed by reference to both the updater and
         * display2d. Inputs are to be processed right in this class, or something.
         */

        // TODO: auto-center each model's center of mass at its origin
        const wf_state::th_model fat = {{
            { -2.5f, -1.5f, 0.0f },
            { 2.5f, 1.5f, 0.0f },
            { -3.5f, 2.6f, 0.0f },
            { -1.5f, 0.5f, 7.5f }
        }};

        const wf_state::th_model center = {{
            { -1.0f, -1.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f },
            { 1.0f, -1.0f, 0.0f },
            { 0.0f, 0.0f, 2.5f }
        }};

        const wf_state::th_model skinny = {{
            { 1.0f, -1.0f, -1.0f },
            { 1.0f, -1.0f, 1.0f },
            { 1.0f, 1.0f, 1.0f },
            { 15.0f, 0.0f, 0.0f }
        }};

        state.th_objects = std::vector<wf_state::th_object> {
                { fat, QColor::fromRgb(255, 107, 0), { -4, 4, 0 }, { 0, 0, 0 } },
                { fat, QColor::fromRgb(255, 117, 0), { 4, 4, -1 }, { 3.14f, 0, 0 } },
                { fat, QColor::fromRgb(255, 127, 0), { 4, -4, -1 }, { 0, 0, 1.57f } },
                { fat, QColor::fromRgb(255, 137, 0), { -4, -4, 0 }, { 0, 3.14f, 1.57f } },
                { center, QColor::fromRgb(255, 255, 255), { 0, 0, -1 }, { 0, 0, 0 } },
                { skinny, QColor::fromRgb(255, 0, 0), { 0, 0, 0 }, { 0, 0, 0 } },
                { skinny, QColor::fromRgb(0, 255, 0), { 0, 0, 0 }, { 0, 0, 1.57f } },
                { skinny, QColor::fromRgb(0, 0, 255), { 0, 0, 0 }, { 0, 1.57f, 0 } }
        };

        state.camera = {
                { 0, 0, 10 },
                { 0, 0, -1 }
        };

        state.v_camera = {
                { 0, 0, 0 },
                { 0, 0, 0 }
        };

        state.perspective = {
                1, 4.0f / 3.0f, 0, -100
        };
    }

    display2d_widget* p_widget;

    QElapsedTimer last_tick;

public slots:
    void tick()
    {
        last_tick.restart(); // TODO: restart at the end of the "previous" iteration?

        // TODO: skip frames to catch up?

        // Do routine updates
        state.camera.pos += state.v_camera.pos;

        // Redraw
        p_widget->update();

        const int64_t t_left = 16 - last_tick.elapsed();

//        std::cout << t_left << '\n';

        QTimer::singleShot(std::max(0LL, t_left), this, &wf_viewer::tick);
    };

    void shitslide(int new_theta_w)
    {
        std::cout << new_theta_w << std::endl;
        state.perspective.theta_w = new_theta_w / 180.0f * (float)M_PI;
    }

protected:
    void keyPressEvent(QKeyEvent* event) override
    {
        if(event->isAutoRepeat()) {
            return; // TODO: keep this in mind for new keys
        }

        switch(event->key()) {
            case Qt::Key_1:
                p_widget->hud_camera = !p_widget->hud_camera;
                return;
            case Qt::Key_2:
                p_widget->hud_perspective = !p_widget->hud_perspective;
                return;
            case Qt::Key_W:
                state.v_camera.pos += { 0, -0.025f, 0 };
                return;
            case Qt::Key_A:
                state.v_camera.pos += { -0.025f, 0, 0 };
                return;
            case Qt::Key_S:
                state.v_camera.pos += { 0, 0.025f, 0 };
                return;
            case Qt::Key_D:
                state.v_camera.pos += { 0.025f, 0, 0 };
                return;
            default:
                QWidget::keyPressEvent(event);
                std::cout << event->text().toStdString() << std::endl;
        }
    }

    void keyReleaseEvent(QKeyEvent* event) override
    {
        if(event->isAutoRepeat()) {
            return;
        }

        switch(event->key()) {
            case Qt::Key_W:
                state.v_camera.pos -= { 0, -0.025f, 0 };
                return;
            case Qt::Key_A:
                state.v_camera.pos -= { -0.025f, 0, 0 };
                return;
            case Qt::Key_S:
                state.v_camera.pos -= { 0, 0.025f, 0 };
                return;
            case Qt::Key_D:
                state.v_camera.pos -= { 0.025f, 0, 0 };
                return;
            default:
                QWidget::keyReleaseEvent(event);
                std::cout << event->text().toStdString() << std::endl;
        }
    }

    void wheelEvent(QWheelEvent* event) override
    {
        state.camera.pos += { 0, 0, event->angleDelta().y() / 100.0f };
    }
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    wf_viewer display;
    display.show();

    // TODO: loop on processEvents instead, or some shit?
    return QApplication::exec();
}

//#include "dz04_qt.moc"
