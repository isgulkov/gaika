
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

        // TODO: use camera-projection transform (reverse it?) to efficiently draw this up to the horizon
        std::shared_ptr<const wf_model> disco_floor;

        {
            std::vector<vec3f> vertices;
            std::vector<std::pair<uint32_t, uint32_t>> segments;

            for(int i = 0; i < 20; i++) {
                for(int j = 0; j < 20; j++) {
                    vertices.emplace_back(10 * (i - 10), 10 * (j - 10), 0);

                    const int i_vertex = (int)vertices.size() - 1;

                    if(j != 0) {
                        segments.emplace_back(i_vertex, i_vertex - 1);
                    }

                    if(i != 0) {
                        segments.emplace_back(i_vertex, i_vertex - 20);
                    }
                }
            }

            disco_floor = std::make_shared<const wf_model>(wf_model { vertices, segments });
        }

        state.th_objects = std::vector<wf_state::th_object> {
                { disco_floor, { 0, 0, 0 }, { 0, 0, 0 }, 1, QColor::fromRgb(0, 127, 0) },
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
                { 0, 0, 10 }, { 0, 0, 0 }
//                { 15, 0, 0 }, { 1.57f, 1.57f, 0 }
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

        last_tick_end.start();
        last_update.start();
        QTimer::singleShot(0, this, &wf_viewer::tick);

        // TODO: make the world and camera coords right-handed (X right, Y up, Z towards the viewer)
        // TODO: in OpenGL, it's the perspective transform that makes it left-handed
        // https://stackoverflow.com/a/12336360
    }

    display2d_widget* p_widget;

    QElapsedTimer last_tick_end, last_update;

public slots:
    void tick()
    {
        // TODO: skip frames to catch up?

        // Do routine updates
        const float sec_since_update = last_update.elapsed() / 1000.0f;
        last_update.restart();

        state.camera.pos += state.v_camera * sec_since_update;
        state.v_camera = calculate_v_camera();

        // Redraw
        p_widget->update();

        const int64_t t_left = t_tick_target - last_tick_end.elapsed();

        QTimer::singleShot(std::max(0LL, t_left), this, &wf_viewer::tick);
        last_tick_end.restart();
    };

private:
    const uint64_t t_tick_target = 16;
    const float vbase_camera = 10.0f; // per second

    vec3f calculate_v_camera()
    {
        float v_zcam = state.controls.back - state.controls.forward;
        float v_xcam = state.controls.right - state.controls.left;

        if(v_zcam && v_xcam) {
            v_zcam /= (float)M_SQRT2;
            v_xcam /= (float)M_SQRT2;
        }

        return mx_tx::rotate_xyz(state.camera.orient) * vec3f(v_xcam * vbase_camera, 0, v_zcam * vbase_camera);
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

        const int xrel = event->x(), yrel = event->y();

        if(is_rotating) {
            // TODO: a neater way to do this?
            vec3f& orient = state.camera.orient;

            orient.set_z(
                    orient.z() + (xrel - xrel_prev) / 640.0f
            ).set_x(
                    std::min(std::max(orient.x() +  (yrel - yrel_prev) / 640.0f, 0.0f), (float)M_PI)
            );
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
