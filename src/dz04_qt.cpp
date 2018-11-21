
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
#include "obj_io.hpp"

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
                },
                { }
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

            disco_floor = std::make_shared<const wf_model>(wf_model { vertices, segments, { } });
        }

        obj_file sphere_crude_obj = obj_file::read_file("../resources/meshes/simple/sphere_crude.obj");

        std::shared_ptr<const wf_model> sphere_crude = std::make_shared<const wf_model>(wf_model {
                sphere_crude_obj.vertices,
                { },
                sphere_crude_obj.triangles
        });

        obj_file dodecahedron_obj = obj_file::read_file("../resources/meshes/simple/dodecahedron.obj");

        std::shared_ptr<const wf_model> dodecahedron = std::make_shared<const wf_model>(wf_model {
                dodecahedron_obj.vertices,
                { },
                dodecahedron_obj.triangles
        });

        obj_file icosahedron_obj = obj_file::read_file("../resources/meshes/simple/icosahedron.obj");

        std::shared_ptr<const wf_model> icosahedron = std::make_shared<const wf_model>(wf_model {
                icosahedron_obj.vertices,
                { },
                icosahedron_obj.triangles
        });

        std::shared_ptr<const wf_model> cage;

        {
            std::vector<vec3f> vertices;
            std::vector<std::pair<uint32_t, uint32_t>> segments;

            for(int i = 0; i < 4; i++) {
                for(int j = 0; j < 4; j++) {
                    for(int k = 0; k < 4; k++) {
                        vertices.emplace_back(3 * i, 3 * j, 3 * k);

                        const int i_vertex = (int)vertices.size() - 1;

                        if(i != 0) {
                            segments.emplace_back(i_vertex, i_vertex - 4 * 4);
                        }

                        if(j != 0) {
                            segments.emplace_back(i_vertex, i_vertex - 4);
                        }

                        if(k != 0) {
                            segments.emplace_back(i_vertex, i_vertex - 1);
                        }
                    }
                }
            }

            cage = std::make_shared<const wf_model>(wf_model { vertices, segments, { } });
        }

        obj_file torus_obj = obj_file::read_file("../resources/meshes/simple/torus.obj");

        std::shared_ptr<const wf_model> torus = std::make_shared<const wf_model>(wf_model {
                torus_obj.vertices,
                { },
                torus_obj.triangles
        });

        obj_file tomato_obj = obj_file::read_file("../resources/meshes/simple/tomato.obj");

        std::shared_ptr<const wf_model> tomato = std::make_shared<const wf_model>(wf_model {
                tomato_obj.vertices,
                { },
                tomato_obj.triangles
        });

        state.th_objects = std::vector<wf_state::th_object> {
                { disco_floor, { 0, 0, 0 }, { 0, 0, 0 }, 1, {}, false, false, QColor::fromRgb(0, 127, 0) },
                { fat, { 3, -3, 0.25f }, { 0, 0, 0.1f }, 0.25f, {}, true, false, QColor::fromRgb(255, 150, 0) },
                { fat, { -4, -4, 0.5f }, { 0, 0, 0.2f }, 1, {}, true, false, QColor::fromRgb(255, 190, 0) },
                { fat, { -5, 5, 0.75f }, { 0, 0, 0.3f }, 1.25f, {}, true, false, QColor::fromRgb(255, 230, 0) },
                { fat, { 7.5f, 7.5f, 0 }, { 0, 0, 0.4f }, 1.5f, {}, true, false, QColor::fromRgb(255, 255, 0) },
                { center, { 0, 0, -1.0f }, { 0, 0, 0.5f }, 1, {}, true, false, QColor::fromRgb(255, 255, 255) },
                { center, { 0, 0, -0.5f }, { 0, 0, 1 }, 1, {}, true, false, QColor::fromRgb(255, 255, 255) },
                { center, { 0, 0, 0 }, { 0, 0, 0 }, 1, {}, true, false, QColor::fromRgb(255, 255, 255) },
                { skinny, { 0, 0, 0 }, { 0, 0, 0 }, 1, {}, false, false, QColor::fromRgb(255, 0, 0) },
                { skinny, { 0, 0, 0 }, { 0, 0, 1.57f }, 1, {}, false, false, QColor::fromRgb(0, 255, 0) },
                { skinny, { 0, 0, 0 }, { 0, -1.57f, 0 }, 1, {}, false, false, QColor::fromRgb(0, 0, 255) },
                { cuboid, { 10, 10, 0 }, { 0, 0, 0 }, 1, {}, false, false, QColor::fromRgb(255, 105, 180) },
                { cage, { -25, -25, 0 }, { 0, 0, 0 }, 1, {}, false, false, QColor::fromRgb(127, 127, 180) },
                { dodecahedron, { 17, -15, 0 }, { 0, 0, 0 }, 5.0f, {}, true, false, QColor::fromRgb(255, 228, 181) },
                { icosahedron, { 1.5f, -20, 0 }, { 0, 0, 0 }, 5.0f, {}, true, false, QColor::fromRgb(255, 20, 147) },
                { sphere_crude, { -6.5f, -15, 0 }, { 0, 0, 0 }, 0.2f, {}, true, false, QColor::fromRgb(255, 185, 15) },
                { torus, { 75, 75, 25 }, { 1.57f, 0, 0 }, 20.0f, {}, true, false, QColor::fromRgb(224, 102, 255) },
                { tomato, { 40, 40, 5 }, { 0, 1.57f, 0 }, 0.25f, {}, true, false, QColor::fromRgb(255, 0, 0) }
        };

        state.camera = {
                { 0, 0, 10 }, { 0, 0, 0 }
//                { 15, 0, 0 }, { 1.57f, 1.57f, 0 }
        };

        state.projection = {
                (float)(M_PI * 2 / 3), 1, 0.01f, 100.0f
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
        /**
         * 1. Perform (regular) state updates
         */
        const float sec_since_update = last_update.elapsed() / 1000.0f;
        last_update.restart();

        if(mousetrap_on) {
            /**
             * Mouse look has to be updated in the game loop, simultaneously with position, instead of on mouse move
             * events -- otherwise, a noticeable jitter appears when circle-strafing, etc.
             *
             * Amusingly, the google result where I've got this was about Unity scripting.
             */
            update_mousetrap();
        }

        state.camera.pos += state.v_camera * sec_since_update;
        state.v_camera = calculate_v_camera();

        /**
         * 2. Redraw
         *
         * TODO: skip frames to catch up?
         */

        /**
         * For marking objects under the mouse pointer the triangles' screen coords are good enough, so it's done
         * immediately in the renderer's paint method
         *
         * Processing is placed before the update() call to emphasise that the results are from the previous frame
         *
         * TODO: investigate the sensitivity issue that the hovering code seems to cause -- probably has to do with FPS
         */
        if(!(state.options.hovering_disabled = !state.projection.is_orthographic() && !mousetrap_on)) {
            state.options.hovering_limited = p_widget->hovered_multiple;
        }

        // TODO: ... do something with the hovered object ...

        p_widget->update();

        /**
         * 3. Schedule the next iteration
         */
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
            v_camera.z += v_zworld;

            return v_camera.normalize() *= vbase_camera;
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

            if(mousetrap_on) {
                stop_mousetrap();
            }
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
            case Qt::Key_I:
                state.projection.set_z_near(state.projection.z_near() / 10.0f);
                return;
            case Qt::Key_O:
                state.projection.set_z_near(state.projection.z_near() * 10.0f);
                return;
            case Qt::Key_K:
                state.projection.set_z_far(state.projection.z_far() / 10.0f);
                return;
            case Qt::Key_L:
                state.projection.set_z_far(state.projection.z_far() * 10.0f);
                return;
            case Qt::Key_B:
                state.options.use_backface_cull ^= 1;
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

    void focusOutEvent(QFocusEvent* event) override
    {
        state.controls.forward = state.controls.left
                = state.controls.back = state.controls.right
                = state.controls.jump = state.controls.duck = false;

        if(mousetrap_on) {
            stop_mousetrap();
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

        xy_center = mapToGlobal(QPoint(width() / 2, height() / 2));
        QCursor::setPos(xy_center);

        r_xprev = r_yprev = 0.0f;

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

    /**
     * Smooth the angular velocity with a 2-frame moving average. The improvement is noticeable on a 1920-wide viewport
     */
    float r_xprev, r_yprev;

    void update_mousetrap()
    {
        const QPoint xy_rel = QCursor::pos() - xy_center;

        vec3f& orient = state.camera.orient;

        const float r_x = calc_x_rotation(xy_rel.x()), r_y = calc_y_rotation(xy_rel.y());

        orient.x -= (r_yprev + r_y) / 2.0f;

        if(orient.x < 0) {
            orient.x = 0.0f;
        }

        if(orient.x > (float)M_PI) {
            orient.x = (float)M_PI;
        }

        orient.z -= (r_xprev + r_x) / 2.0f;

        r_xprev = r_x;
        r_yprev = r_y;

        QCursor::setPos(xy_center);
    }

    float calc_x_rotation(int d_x) const
    {
        return 1.0f * d_x * (float)M_PI / state.viewport.width;
    }

    float calc_y_rotation(int d_y) const
    {
        return 1.0f * d_y * (float)M_PI / state.viewport.height;
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

    /**
     * It is indeed tempting to avoid blocking here by using processEvents(), but according to the official docs, this
     * pattern should *never* be used.
     */
    return QApplication::exec();
}

//#include "dz04_qt.moc"
