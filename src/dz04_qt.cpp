
#include <vector>
#include <array>
#include <list>
#include <cmath>

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QShortcut>
#include <QIcon>

#include "matrices.hpp"
#include "geometry.hpp"
#include "viewer_state.hpp"
#include "display2d.hpp"

class wf_viewer : public QMainWindow
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

        state.projection = {
                (float)(M_PI * 2 / 3), 1, 0.1f, 100.0f
        };

        state.viewport = { 640, 640 };

        // REMOVE:
        state.perf_stats.n = 7709;
        state.perf_stats.sum_frame = 89789.109f;
        state.perf_stats.sum_transform = 67899.79f;
        state.perf_stats.sum_lines = 10990.88f;
    }

public:
    wf_viewer()
    {
        setWindowTitle("Hello, world!");

        init_state();

        p_widget = new display2d_widget(this, state);

        setCentralWidget(p_widget);

        QMenu* m_file = new QMenu(tr("File"));
        m_file->addAction(tr("Settings"));
//        m_file->addAction(tr("Exit"));

        QMenu* m_help = new QMenu(tr("Help"));
        m_help->addAction(tr("About"));

        menuBar()->addMenu(m_file);
        menuBar()->addMenu(m_help);

        new QShortcut(QKeySequence::StandardKey::Close, this, SLOT(close()));

        last_tick_end.start();
        last_update.start();
        QTimer::singleShot(0, this, &wf_viewer::tick);
    }

    display2d_widget* p_widget;

    QElapsedTimer last_tick_end, last_update;

public slots:
    void tick()
    {
        // TODO: skip rendering frames to catch up?

        // Do routine updates
        const float sec_since_update = last_update.elapsed() / 1000.0f;
        last_update.restart();

        if(mousetrap_on) {
            /**
             * Mouse look has to be updated in the game loop, simultaneously with position, instead of on mouse move
             * events -- otherwise, a noticeable jitter appears when circle-strafing, etc.
             *
             * Amusingly, the google result where I've got this was about Unity scripting.
             *
             * TODO: try applying a moving average over several frames
             */
            update_mousetrap();
        }

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
        const float v_long = state.controls.forward - state.controls.back;
        const float v_lat = state.controls.right - state.controls.left;

        if(!state.projection.is_orthographic()) {
            const float v_zcam = -v_long;
            const float v_xcam = v_lat;
            const float v_zworld = state.controls.jump - state.controls.duck;

            vec3f v_camera = mx_tx::rotate_xyz(state.camera.orient) * vec3f(v_xcam, 0, v_zcam);

            return v_camera.set_z(v_camera.z() + v_zworld).normalize() *= vbase_camera;
        }

        switch(state.projection.axis()) {
            case wf_projection::X:
                return { 0, v_lat * vbase_camera, v_long * vbase_camera };
            case wf_projection::Y:
                return { v_lat * vbase_camera, 0, v_long * vbase_camera,  };
            case wf_projection::Z:
                return { v_lat * vbase_camera, v_long * vbase_camera, 0 };
        }

        return { 0, 0, 0 };
    }

protected:
    void switch_projection()
    {
        if(state.projection.is_perspective()) {
            state.projection.set_parallel();
        }
        else if(state.projection.is_parallel()) {
            state.projection.set_orthographic(wf_projection::X);
            stop_mousetrap();
        }
        else {
            switch(state.projection.axis()) {
                case wf_projection::X:
                    state.projection.set_orthographic(wf_projection::Y);
                    break;
                case wf_projection::Y:
                    state.projection.set_orthographic(wf_projection::Z);
                    break;
                case wf_projection::Z:
                    state.projection.set_perspective();
                    break;
            }
        }
    }

    void keyPressEvent(QKeyEvent* event) override
    {
        if(event->isAutoRepeat()) {
            return; // NOTE: keep this in mind for new keys
        }

        switch(event->key()) {
            case Qt::Key_1:
                p_widget->hud_camera = !p_widget->hud_camera;
                return;
            case Qt::Key_2:
                p_widget->hud_projection = !p_widget->hud_projection;
                return;
            case Qt::Key_3:
                p_widget->hud_viewport = !p_widget->hud_viewport;
                return;
            case Qt::Key_Equal:
                switch_projection();
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
            case Qt::Key_Space:
                state.controls.jump = true;
                return;
            case Qt::Key_Control:
                state.controls.duck = true;
                return;
            case Qt::Key_Escape:
                if(mousetrap_on) {
                    stop_mousetrap();
                }
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
            case Qt::Key_Space:
                state.controls.jump = false;
                return;
            case Qt::Key_Control:
                state.controls.duck = false;
                return;
            default:
                QWidget::keyReleaseEvent(event);
        }
    }

    void wheelEvent(QWheelEvent* event) override
    {
        if(state.projection.is_perspective()) {
            float theta = state.projection.theta();

            theta += event->angleDelta().y() / 1000.f;

            if(theta < 0.0f) {
                theta = 0.0f;
            }

            if(theta > (float)M_PI) {
                theta = (float)M_PI;
            }

            state.projection.set_theta(theta);
        }
        else {
            float scale = state.projection.scale();

            scale *= std::pow(1.005f, -event->angleDelta().y());

            state.projection.set_scale(scale);
        }
    }

    bool mousetrap_on = false;
    QPoint xy_before, xy_center;

    void start_mousetrap()
    {
        setMouseTracking(true);
        p_widget->setMouseTracking(true);

        setCursor(Qt::BlankCursor);

        xy_before = mapFromGlobal(QCursor::pos());

        // TODO: use center of the display widget
        xy_center = mapToGlobal(QPoint(width() / 2, height() / 2));
        QCursor::setPos(xy_center);

        mousetrap_on = state.options.free_look = true;
    }

    void stop_mousetrap()
    {
        setMouseTracking(false);
        p_widget->setMouseTracking(false);

        QCursor::setPos(mapToGlobal(xy_before));

        setCursor(Qt::ArrowCursor);
        mousetrap_on = state.options.free_look = false;
    }

    float calc_x_rotation(int d_x) const
    {
        return 1.0f * d_x * (float)M_PI / state.viewport.width;
    }

    float calc_y_rotation(int d_y) const
    {
        return 1.0f * d_y * (float)M_PI / state.viewport.height;
    }

    void update_mousetrap()
    {
        const QPoint xy_rel = QCursor::pos() - xy_center;

        vec3f& orient = state.camera.orient;

        orient.set_z(
                orient.z() - calc_x_rotation(xy_rel.x())
        ).set_x(
                std::min(std::max(orient.x() - calc_y_rotation(xy_rel.y()), 0.0f), (float)M_PI)
        );

        QCursor::setPos(xy_center);
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if(!mousetrap_on && !state.projection.is_orthographic()) {
            start_mousetrap();
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        // ...
    }

    void resizeEvent(QResizeEvent* event) override
    {
        state.viewport.width = p_widget->size().width();
        state.viewport.height = p_widget->size().height();

        state.projection.set_wh_ratio((float)state.viewport.width / state.viewport.height);
    }
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QApplication::setAttribute(Qt::AA_MacDontSwapCtrlAndMeta);

    QIcon icon_app;

    icon_app.addFile(":/icons/teapot-32.png");
    icon_app.addFile(":/icons/teapot-128.png");
    icon_app.addFile(":/icons/teapot-256.png");
    icon_app.addFile(":/icons/teapot-512.png");
    icon_app.addFile(":/icons/teapot-1024.png");

    QApplication::setWindowIcon(icon_app);

    wf_viewer display;
    display.show();

    // TODO: loop on processEvents instead?
    return QApplication::exec();
}

//#include "dz04_qt.moc"
