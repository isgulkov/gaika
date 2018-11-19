
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
#include "viewer_state.hpp"
#include "display2d.hpp"

class wf_viewer : public QWidget
{
//    Q_OBJECT

    wf_state state;

    void init_state()
    {
        // TODO: move somewhere reasonable

        std::shared_ptr<const wf_model> fat = std::make_shared<const wf_model>(wf_model::tetrahedron(
                { -1.25f, -2.25f, 0.0f },
                { 3.75f, 0.75f, 0.0f },
                { -2.25f, 1.75f, 0.0f },
                { -0.25f, -0.25f, 7.5f }
        ));

        std::shared_ptr<const wf_model> center = std::make_shared<const wf_model>(wf_model::tetrahedron(
                { -1.0f, -0.67f, 0.0f },
                { 0.0f, 1.33f, 0.0f },
                { 1.0f, -1.67f, 0.0f },
                { 0.0f, 0.0f, 2.5f }
        ));

        std::shared_ptr<const wf_model> skinny = std::make_shared<const wf_model>(wf_model::tetrahedron(
                { 1.0f, -1.0f, -1.0f },
                { 1.0f, -1.0f, 1.0f },
                { 1.0f, 1.0f, 1.0f },
                { 15.0f, 0.0f, 0.0f }
        ));

        std::shared_ptr<const wf_model> cuboid = std::make_shared<const wf_model>(wf_model {
                {
                        { 0, -2, -2 },
                        { 0, -2, 2 },
                        { 0, 2, 2 },
                        { 0, 2, -2 },
                        { 10, -2, -2 },
                        { 10, -2, 2 },
                        { 10, 2, 2 },
                        { 10, 2, -2 }
                },
                {
                    { 0, 1 }, { 1, 2 }, { 2, 3 }, { 0, 3 },
                    { 4, 5 }, { 5, 6 }, { 6, 7 }, { 4, 7 },
                    { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 }
                }
        });

        state.th_objects = std::vector<wf_state::th_object> {
                { fat, { 3, -3, 0.25f }, { 0, 0, 0.1f }, 0.25f, QColor::fromRgb(255, 150, 0) },
                { fat, { -4, -4, 0.5f }, { 0, 0, 0.2f }, 1, QColor::fromRgb(255, 190, 0) },
                { fat, { -5, 5, 0.75f }, { 0, 0, 0.3f }, 1.25f, QColor::fromRgb(255, 230, 0) },
                { fat, { 7.5f, 7.5f, 0 }, { 0, 0, 0.4f }, 1.5f, QColor::fromRgb(255, 255, 0) },
                { center, { 0, 0, -1.0f }, { 0, 0, 0.5f }, 1, QColor::fromRgb(255, 255, 255) },
                { center, { 0, 0, -0.5f }, { 0, 0, 1 }, 1, QColor::fromRgb(255, 255, 255) },
                { center, { 0, 0, 0 }, { 0, 0, 0 }, 1, QColor::fromRgb(255, 255, 255) },
                { skinny, { 0, 0, 0 }, { 0, 0, 0 }, 1, QColor::fromRgb(255, 0, 0) },
                { skinny, { 0, 0, 0 }, { 0, 0, 1.57f }, 1, QColor::fromRgb(0, 255, 0) },
                { skinny, { 0, 0, 0 }, { 0, -1.57f, 0 }, 1, QColor::fromRgb(0, 0, 255) },
                { cuboid, { 10, 10, 0 }, { 0, 0, 0 }, 1, QColor::fromRgb(255, 105, 180) }
        };

        state.camera = {
//                { 0, 0, 10 }, { 0, 0, 0 }
                { 15, 0, 0 }, { 1.57f, 1.57f, 0 }
        };

        state.perspective = {
                (float)(M_PI * 2 / 3), 1.0f, 0.1f, 100.f
        };

        state.viewport = { 640, 640 };
    }

public:
    wf_viewer()
    {
        setWindowTitle("Hello, world!");

        init_state();

        p_widget = new display2d_widget(this, state);

        auto* layout = new QHBoxLayout;
        layout->setSpacing(0);
        layout->setMargin(0);

        layout->addWidget(p_widget);

        setLayout(layout);

        last_tick.start();
        QTimer::singleShot(0, this, &wf_viewer::tick);

        // TODO: make the world and camera coords right-handed (X right, Y up, Z towards the viewer)
        // TODO: in OpenGL, it's the perspective transform that makes it left-handed
        // https://stackoverflow.com/a/12336360
    }

    display2d_widget* p_widget;

    QElapsedTimer last_tick;

public slots:
    void tick()
    {
        last_tick.restart(); // TODO: restart at the end of the "previous" iteration?

        // TODO: skip frames to catch up?

        // Do routine updates
        state.camera.pos += state.v_camera * 0.5f; // TODO: base on time, not frames
        state.v_camera = calculate_v_camera();

        // Redraw
        p_widget->update();

        const int64_t t_left = 16 - last_tick.elapsed();

        QTimer::singleShot(std::max(0LL, t_left), this, &wf_viewer::tick);
    };

private:
    vec3f calculate_v_camera()
    {
        // TODO: do this more efficiently, ffs

        vec3f v_camera = { 0, 0, 0 };

        if(state.controls.forward) {
            v_camera += { 0, 0, -1 };
        }

        if(state.controls.back) {
            v_camera += { 0, 0, 1 };
        }

        if(state.controls.left) {
            v_camera += { -1, 0, 0 }; // TODO: lock L-R movement on the world's horizontal plane!
        }

        if(state.controls.right) {
            v_camera += { 1, 0, 0 };
        }

        return (mx_tx::rotate_xyz(state.camera.orient) * v_camera).unit();
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
                state.controls.forward = true;
                return;
            case Qt::Key_A:
                state.controls.left = true;
                return;
            case Qt::Key_S:
                state.controls.back = true;
                return;
            case Qt::Key_D:
                state.controls.right = true;
                return;
            default:
                QWidget::keyPressEvent(event);
        }
    }

    void keyReleaseEvent(QKeyEvent* event) override
    {
        if(event->isAutoRepeat()) {
            return;
        }

        switch(event->key()) {
            case Qt::Key_W:
                state.controls.forward = false;
                return;
            case Qt::Key_A:
                state.controls.left = false;
                return;
            case Qt::Key_S:
                state.controls.back = false;
                return;
            case Qt::Key_D:
                state.controls.right = false;
                return;
            default:
                QWidget::keyReleaseEvent(event);
        }
    }

    void wheelEvent(QWheelEvent* event) override
    {
        float& theta_w = state.perspective.theta_w;

        theta_w += event->angleDelta().y() / 1000.f;

        if(theta_w < 0.0f) {
            theta_w = 0.0f;
        }

        if(theta_w > (float)M_PI) {
            theta_w = (float)M_PI;
        }
    }

    bool is_rotating = false;
    int xrel_prev, yrel_prev;

    void mouseMoveEvent(QMouseEvent* event) override
    {
        // TODO: enable tracking, implement fps-like control (toggle with LMB/Esc), figure out sensitivity
        // TODO: in FPS, left-right turns around world Z, moves along world XY

        const int xrel = event->x(), yrel = event->y();

        if(is_rotating) {
            state.camera.orient += {
                (yrel - yrel_prev) / 640.0f,
                (xrel - xrel_prev) / 640.0f,
                0
            };
        }

        xrel_prev = xrel;
        yrel_prev = yrel;
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if(!is_rotating) {
            is_rotating = true;

            xrel_prev = event->x();
            yrel_prev = event->y();
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        is_rotating = false;
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
