
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

        std::shared_ptr<const isg::model> fat = std::make_shared<const isg::model>(isg::model::tetrahedron(
                { -1.25f, -2.25f, 0.0f },
                { -2.25f, 1.75f, 0.0f },
                { 3.75f, 0.75f, 0.0f },
                { -0.25f, -0.25f, 7.5f }
        ));

        std::shared_ptr<const isg::model> center = std::make_shared<const isg::model>(isg::model::tetrahedron(
                { -1.0f, -0.67f, 0.0f },
                { 0.0f, 1.33f, 0.0f },
                { 1.0f, -1.67f, 0.0f },
                { 0.0f, 0.0f, 2.5f }
        ));

        std::shared_ptr<const isg::model> skinny = std::make_shared<const isg::model>(isg::model::tetrahedron(
                { 1.0f, -1.0f, -1.0f },
                { 1.0f, -1.0f, 1.0f },
                { 1.0f, 1.0f, 1.0f },
                { 15.0f, 0.0f, 0.0f }
        ));

        std::shared_ptr<const isg::model> cuboid = std::make_shared<const isg::model>(isg::model {
                "Cuboid",
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
        std::shared_ptr<const isg::model> disco_floor;

        {
            std::vector<vec3f> vertices;
            std::vector<isg::model::segment_line> segments;

            for(int i = 0; i < 20; i++) {
                for(int j = 0; j < 20; j++) {
                    vertices.emplace_back(10 * (i - 10), 10 * (j - 10), 0);

                    const uint16_t i_vertex = (uint16_t)(vertices.size() - 1);

                    if(j != 0) {
                        isg::model::segment_line segment({ i_vertex, (uint16_t)(i_vertex - 1), { 0, 240, 0 } });
                        segments.push_back(segment);
                    }

                    if(i != 0) {
                        isg::model::segment_line segment({ i_vertex, (uint16_t)(i_vertex - 20), { 0, 240, 0 } });
                        segments.push_back(segment);
                    }
                }
            }

            disco_floor = std::make_shared<const isg::model>(isg::model { "DiscoFloor", vertices, segments, { } });
        }

        obj_file sphere_crude_obj = obj_file::read_file("../resources/meshes/simple/sphere_crude.obj");

        std::shared_ptr<const isg::model> sphere_crude = std::make_shared<const isg::model>(isg::model {
                "Sphere",
                sphere_crude_obj.vertices,
                { },
                sphere_crude_obj.triangles
        });

        obj_file tetrahedron_obj = obj_file::read_file("../resources/meshes/simple/tetrahedron.obj");
        std::shared_ptr<const isg::model> tetrahedron = std::make_shared<const isg::model>(isg::model {
                tetrahedron_obj.name, tetrahedron_obj.vertices, { }, tetrahedron_obj.triangles
        });

        obj_file hexahedron_obj = obj_file::read_file("../resources/meshes/simple/hexahedron.obj");
        std::shared_ptr<const isg::model> hexahedron = std::make_shared<const isg::model>(isg::model {
                hexahedron_obj.name, hexahedron_obj.vertices, { }, hexahedron_obj.triangles
        });

        obj_file octahedron_obj = obj_file::read_file("../resources/meshes/simple/octahedron.obj");
        std::shared_ptr<const isg::model> octahedron = std::make_shared<const isg::model>(isg::model {
                octahedron_obj.name, octahedron_obj.vertices, { }, octahedron_obj.triangles
        });

        obj_file dodecahedron_obj = obj_file::read_file("../resources/meshes/simple/dodecahedron.obj");
        std::shared_ptr<const isg::model> dodecahedron = std::make_shared<const isg::model>(isg::model {
                dodecahedron_obj.name, dodecahedron_obj.vertices, { }, dodecahedron_obj.triangles
        });

        obj_file icosahedron_obj = obj_file::read_file("../resources/meshes/simple/icosahedron.obj");
        std::shared_ptr<const isg::model> icosahedron = std::make_shared<const isg::model>(isg::model {
                icosahedron_obj.name, icosahedron_obj.vertices, { }, icosahedron_obj.triangles
        });

        obj_file sphere40_obj = obj_file::read_file("../resources/meshes/simple/sphere40.obj");
        std::shared_ptr<const isg::model> sphere200 = std::make_shared<const isg::model>(isg::model {
                sphere40_obj.name, sphere40_obj.vertices, { }, sphere40_obj.triangles
        });

        std::shared_ptr<const isg::model> cage;

        {
            std::vector<vec3f> vertices;
            std::vector<isg::model::segment_line> segments;

            for(int i = 0; i < 4; i++) {
                for(int j = 0; j < 4; j++) {
                    for(int k = 0; k < 4; k++) {
                        vertices.emplace_back(3 * i, 3 * j, 3 * k);

                        const int i_vertex = (int)vertices.size() - 1;

                        std::array<uint8_t, 3> color{ (uint8_t)(127 + 5 * i), (uint8_t)(127 - 5 * j), (uint8_t)(180 + 5 * k) };

                        if(i != 0) {
                            segments.emplace_back(i_vertex, i_vertex - 4 * 4, color);
                        }

                        if(j != 0) {
                            segments.emplace_back(i_vertex, i_vertex - 4, color);
                        }

                        if(k != 0) {
                            segments.emplace_back(i_vertex, i_vertex - 1, color);
                        }
                    }
                }
            }

            cage = std::make_shared<const isg::model>(isg::model { "WireCage", vertices, segments, { } });
        }

        obj_file tomato_obj = obj_file::read_file("../resources/meshes/simple/tomato.obj");

        std::shared_ptr<const isg::model> tomato = std::make_shared<const isg::model>(isg::model {
                "Tomato",
                tomato_obj.vertices,
                { },
                tomato_obj.triangles
        });

        qDebug() << "seg" << sizeof(isg::model::segment_line) << alignof(isg::model::segment_line);
        qDebug() << "tri" << sizeof(isg::model::triangle_face) << alignof(isg::model::triangle_face);

        state.th_objects = std::vector<wf_state::th_object> {
                wf_state::th_object(disco_floor),
                wf_state::th_object(cuboid).set_pos({ 10, 10, 0 }),
                wf_state::th_object(cage).set_pos({ -25, -25, 0 }),
                wf_state::th_object(fat).set_id("Fat NW").set_pos({ 3, -3, 0.25f }).set_orient({ 0, 0, 0.1f }).set_scale(0.25f).set_hoverable(true),
                wf_state::th_object(fat).set_id("Fat SW").set_pos({ -4, -4, 0.5f }).set_orient({ 0, 0, 0.2f }).set_hoverable(true),
                wf_state::th_object(fat).set_id("Fat SE").set_pos({ -5, 5, 0.75f }).set_orient({ 0, 0, 0.3f }).set_scale(1.25f).set_hoverable(true),
                wf_state::th_object(fat).set_id("Fat NE").set_pos({ 7.5f, 7.5f, 0 }).set_orient({ 0, 0, 0.4f }).set_scale({ 1.5f, 1.5f, 0.9f }).set_hoverable(true),
                wf_state::th_object(skinny),
                wf_state::th_object(skinny).set_orient({ 0, 0, 1.57f }),
                wf_state::th_object(skinny).set_orient({ 0, -1.57f, 0 }),
                wf_state::th_object(tetrahedron).set_pos({ 20, -80, 0 }).set_hoverable(true),
                wf_state::th_object(hexahedron).set_pos({ 30, -70, 0 }).set_hoverable(true),
                wf_state::th_object(octahedron).set_pos({ 40, -60, 0 }).set_hoverable(true),
                wf_state::th_object(dodecahedron).set_pos({ 50, -50, 0 }).set_hoverable(true),
                wf_state::th_object(icosahedron).set_pos({ 60, -40, 0 }).set_hoverable(true),
                wf_state::th_object(sphere200).set_pos({ 70, -30, 5 }).set_hoverable(true),
                wf_state::th_object(tomato).set_pos({ -45, -30, 5 }).set_scale(0.25f).set_hoverable(true)
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
        setFocusPolicy(Qt::ClickFocus);

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

        if(freelook_on) {
            /**
             * Mouse look has to be updated in the game loop, simultaneously with position, instead of on mouse move
             * events -- otherwise, a noticeable jitter appears when circle-strafing, etc.
             *
             * Amusingly, the google result where I've got this was about Unity scripting.
             */
            update_freelook();
        }
        else if(drag_target != nullptr) {
            update_drag();
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
         */
        if(!(state.hovering.disabled = !state.projection.is_orthographic() && !freelook_on)) {
            state.hovering.limited = p_widget->hovered_multiple;
        }

        if(!state.hovering.disabled) {
            // TODO: ... do something with the hovered object ...
            state.hovering.object = const_cast<wf_state::th_object*>(p_widget->hovered_object);
        }

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

            stop_freelook();
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
            case Qt::Key_W:
                state.controls.forward = true;
                stop_drag();
                return;
            case Qt::Key_A:
                state.controls.left = true;
                stop_drag();
                return;
            case Qt::Key_S:
                state.controls.back = true;
                stop_drag();
                return;
            case Qt::Key_D:
                state.controls.right = true;
                stop_drag();
                return;
            case Qt::Key_Space:
                state.controls.jump = true;
                stop_drag();
                return;
            case Qt::Key_Control:
                state.controls.duck = true;
                stop_drag();
                return;
            case Qt::Key_Escape:
                stop_freelook();
                return;
            case Qt::Key_1:
                p_widget->hud_camera = !p_widget->hud_camera;
                return;
            case Qt::Key_2:
                p_widget->hud_projection = !p_widget->hud_projection;
                return;
            case Qt::Key_3:
                p_widget->hud_viewport = !p_widget->hud_viewport;
                return;
            case Qt::Key_8:
                stop_freelook();
                state.projection.set_orthographic(wf_projection::X);
                stop_drag();
                return;
            case Qt::Key_9:
                stop_freelook();
                state.projection.set_orthographic(wf_projection::Y);
                stop_drag();
                return;
            case Qt::Key_0:
                stop_freelook();
                state.projection.set_orthographic(wf_projection::Z);
                stop_drag();
                return;
            case Qt::Key_Minus:
                state.projection.set_parallel();
                stop_drag();
                return;
            case Qt::Key_Equal:
                state.projection.set_perspective();
                stop_drag();
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
                switch(state.options.use_backface_cull) {
                    case wf_state::BFC_DISABLE:
                        state.options.use_backface_cull = wf_state::BFC_TRANSPARENT;
                        break;
                    case wf_state::BFC_TRANSPARENT:
                        state.options.use_backface_cull = wf_state::BFC_CULL;
                        break;
                    case wf_state::BFC_CULL:
                    default:
                        state.options.use_backface_cull = wf_state::BFC_DISABLE;
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

    void focusOutEvent(QFocusEvent* event) override
    {
        state.controls.forward = state.controls.left
                = state.controls.back = state.controls.right
                = state.controls.jump = state.controls.duck = false;

        stop_freelook();
        stop_drag();
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

    void mousePressEvent(QMouseEvent* event) override
    {
        if(!freelook_on && !state.projection.is_orthographic()) {
            start_freelook();
        }

        if(state.projection.is_orthographic() && !state.hovering.limited && drag_target == nullptr && state.hovering.object != nullptr) {
            start_drag(state.hovering.object);
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        // ...

        stop_drag();
    }

    void resizeEvent(QResizeEvent* event) override
    {
        state.viewport.width = p_widget->size().width();
        state.viewport.height = p_widget->size().height();

        state.projection.set_wh_ratio((float)state.viewport.width / state.viewport.height);
    }

    wf_state::th_object* drag_target = nullptr;
    QPoint xy_prev;

    void start_drag(wf_state::th_object* new_target)
    {
        // REMOVE: allow this interaction after fixing it (see update_drag)
        if(state.controls.forward || state.controls.back || state.controls.left || state.controls.right || state.controls.jump || state.controls.duck) {
            return;
        }

        p_widget->setMouseTracking(true);

        xy_prev = QCursor::pos();

        state.hovering.fixed = true;
        drag_target = state.hovering.object = new_target;
    }

    void stop_drag()
    {
        if(drag_target == nullptr) {
            return;
        }

        p_widget->setMouseTracking(false);

        state.hovering.fixed = false;
        drag_target = state.hovering.object = nullptr;
    }

    void update_drag()
    {
        const QPoint xy_current = QCursor::pos();
        const QPoint xy_rel = xy_current - xy_prev;

        xy_prev = xy_current;

        // TODO: instead of using deltas, calculate the current position based on mouse coords transformed into world
        // TODO: -> don't stop drag on keyboard control and allow starting it with keyboard control
        // TODO: -> implement Shift-lock

        const float d_x = xy_rel.x() / state.projection.scale() * 2 * state.viewport.width / state.viewport.height / state.viewport.width;
        const float d_y = -xy_rel.y() / state.projection.scale() * 2 / state.viewport.height;

        switch(state.projection.axis()) {
            case wf_projection::X:
                drag_target->pos += { 0, d_x, d_y };
                break;
            case wf_projection::Y:
                drag_target->pos += { d_x, 0, d_y };
                break;
            case wf_projection::Z:
            default:
                drag_target->pos += { d_x, d_y, 0 };
                break;
        }
        drag_target->vertices_world.clear();
    }

    bool freelook_on = false;
    QPoint xy_before, xy_center;

    void start_freelook()
    {
//        setMouseTracking(true);
        p_widget->setMouseTracking(true);

        setCursor(Qt::BlankCursor);

        xy_before = mapFromGlobal(QCursor::pos());

        xy_center = mapToGlobal(QPoint(width() / 2, height() / 2));
        QCursor::setPos(xy_center);

        r_xprev = r_yprev = 0.0f;

        freelook_on = state.options.free_look = true;
    }

    void stop_freelook()
    {
        if(!freelook_on) {
            return;
        }

//        setMouseTracking(false);
        p_widget->setMouseTracking(false);

        QCursor::setPos(mapToGlobal(xy_before));

        setCursor(Qt::ArrowCursor);
        freelook_on = state.options.free_look = false;
    }

    /**
     * Smooth the angular velocity with a 2-frame moving average. The improvement is noticeable on a 1920-wide viewport
     */
    float r_xprev, r_yprev;

    void update_freelook()
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
