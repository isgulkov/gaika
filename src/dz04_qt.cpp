
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

    int i_light = 0;

    wf_state::th_object create_light_source(const vec3f& color, float d)
    {
        auto model = isg::obj_io::read_obj_model("../resources/meshes/lightbulb.obj");

        const vec3f color_bulb = color / std::max(color.x, std::max(color.y, color.z));

        for(auto& mat : model.materials) {
            if(mat.c_diffuse == vec3f(1, 1, 1)) {
                mat.c_diffuse = mat.c_ambient = color_bulb;
            }
        }

        return wf_state::th_object(std::make_shared<const isg::model>(model)).set_scale(d).set_light(color).set_id("LightSource" + std::to_string(i_light++)).set_hoverable(true);
    }

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

        using namespace isg;

        std::shared_ptr<const isg::model> tetrahedron = std::make_shared<const isg::model>(
                obj_io::read_obj_model("../resources/meshes/simple/tetrahedron.obj")
        );

        std::shared_ptr<const isg::model> hexahedron = std::make_shared<const isg::model>(
                obj_io::read_obj_model("../resources/meshes/simple/hexahedron.obj")
        );

        std::shared_ptr<const isg::model> octahedron = std::make_shared<const isg::model>(
                obj_io::read_obj_model("../resources/meshes/simple/octahedron.obj")
        );

        std::shared_ptr<const isg::model> dodecahedron = std::make_shared<const isg::model>(
                obj_io::read_obj_model("../resources/meshes/simple/dodecahedron.obj")
        );

        std::shared_ptr<const isg::model> icosahedron = std::make_shared<const isg::model>(
                obj_io::read_obj_model("../resources/meshes/simple/icosahedron.obj")
        );

        std::shared_ptr<const isg::model> sphere40 = std::make_shared<const isg::model>(
                obj_io::read_obj_model("../resources/meshes/simple/sphere40.obj")
        );

        std::shared_ptr<const isg::model> sphere200 = std::make_shared<const isg::model>(
                obj_io::read_obj_model("../resources/meshes/simple/sphere200.obj")
        );

        std::shared_ptr<const isg::model> tomato = std::make_shared<const isg::model>(
                obj_io::read_obj_model("../resources/meshes/simple/tomato.obj")
        );

        std::shared_ptr<const isg::model> tomato_smooth = std::make_shared<const isg::model>(
                obj_io::read_obj_model("../resources/meshes/simple/tomato_smooth.obj")
        );

        std::shared_ptr<const isg::model> cubecol = std::make_shared<const isg::model>(
                obj_io::read_obj_model("../resources/meshes/simple/cube_colors.obj")
        );

        std::shared_ptr<const isg::model> fish = std::make_shared<const isg::model>(
                obj_io::read_obj_model("../resources/meshes/goldfish.obj")
        );

        std::shared_ptr<const isg::model> flattrn = std::make_shared<const isg::model>(
                obj_io::read_obj_model("../resources/meshes/simple/flattrn.obj")
        );

        std::shared_ptr<const isg::model> lightbulb = std::make_shared<const isg::model>(
                obj_io::read_obj_model("../resources/meshes/lightbulb.obj")
        );

        // TODO: shift mesh coords to center of mass (?) on load
        state.th_objects = std::vector<wf_state::th_object> {
                wf_state::th_object(flattrn).set_scale(2.5f),
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
                wf_state::th_object(sphere40).set_pos({ 70, -30, 5 }).set_hoverable(true),
                wf_state::th_object(sphere200).set_pos({ 85, -45, 10 }).set_scale(2).set_hoverable(true),
                wf_state::th_object(tomato).set_pos({ -45, -30, 5 }).set_scale(0.25f).set_hoverable(true),
                wf_state::th_object(tomato_smooth).set_pos({ -25, -40, 5 }).set_orient({ float(M_PI) / 2, 0, 0 }).set_scale(0.15f).set_hoverable(true),
                wf_state::th_object(cubecol).set_pos({ 20, 20, 2.5f }).set_hoverable(true),
                wf_state::th_object(fish).set_pos({ 20, 50, 5 }).set_orient({ float(M_PI_2), 0, float(M_PI) / 4 }).set_scale(2.0f).set_hoverable(true),
                create_light_source({ 100.0f, 0.5f, 0.5f }, 2.5f).set_pos({ -45, -45, 9 })
        };

        state.lighting.dir_lights.push_back(
                {
                        0.77f, 0.357f,
                        vec3f(0, 1.0f, 0.15f) * 0.2f
                }
        );

        state.lighting.dir_lights.push_back(
                {
                        1.57f, 0.785f,
                        vec3f(1.0f, 1.0f, 0.95f) * .9f
                }
        );

        state.camera = {
//                { 0, 0, 10 }, { 0, 0, 0 }
                { -15, 35, 35 }, { float(M_PI) / 4, 0, -float(M_PI) / 4 * 3 }
        };

        state.projection = {
                (float)(M_PI * 2 / 3), 1, 1.0f, 1000.0f
        };

        state.options.occlusion = wf_state::OCC_BFC_ZBUF;
        state.options.shading = wf_state::SHD_GOURAUD;

        state.viewport = { 640, 640, 1 };
//        state.viewport = { 320, 320, 2 };

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

        if(drag_target != nullptr) {
            update_drag();
        }

        if(drag_target && (state.hovering.mode == wf_state::INT_DRAG || state.hovering.mode == wf_state::INT_CARRY)) {
            drag_target->pos += state.v_camera * sec_since_update;
            drag_target->vertices_world.clear();
        }

        state.camera.pos += state.v_camera * sec_since_update;
        state.v_camera = calculate_v_camera();

        // Sun movement: one full circle every 60s
        state.lighting.dir_lights.back().azimuth += 3.14f * 2 * sec_since_update / 60;

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

        if(!state.hovering.disabled && !state.hovering.fixed) {
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
    const float vbase_camera = 25.0f; // per second

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

    void set_scale(int scale)
    {
        if(scale < 1) {
            return;
        }

        state.viewport.scale = scale;

        state.viewport.width = p_widget->width() / scale;
        state.viewport.height = p_widget->height() / scale;
    }

    void keyPressEvent(QKeyEvent* event) override
    {
        if(event->isAutoRepeat()) {
            return; // NOTE: keep this in mind for new keys
        }

        switch(event->key()) {
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
            case Qt::Key_BracketLeft:
                set_scale(state.viewport.scale - 1);
                return;
            case Qt::Key_BracketRight:
                set_scale(state.viewport.scale + 1);
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
            case Qt::Key_C:
                switch(state.options.occlusion) {
                    case wf_state::OCC_NONE:
                        state.options.occlusion = wf_state::OCC_BFC;
                        break;
                    case wf_state::OCC_BFC:
                        state.options.occlusion = wf_state::OCC_BFC_ZBUF;
                        break;
                    case wf_state::OCC_BFC_ZBUF:
                    default:
                        state.options.occlusion = wf_state::OCC_NONE;
                        break;
                }
                return;
            case Qt::Key_V:
                switch(state.options.shading) {
                    case wf_state::SHD_NONE:
                        state.options.shading = wf_state::SHD_FLAT;
                        break;
                    case wf_state::SHD_FLAT:
                        state.options.shading = wf_state::SHD_GOURAUD;
                        break;
                    case wf_state::SHD_GOURAUD:
//                        state.options.shading = wf_state::SHD_PHONG;
                        state.options.shading = wf_state::SHD_FLAT;
                        break;
                    case wf_state::SHD_PHONG:
                    default:
                        state.options.shading = wf_state::SHD_NONE;
                        break;
                }
                return;
            case Qt::Key_B:
                switch(state.options.lighting) {
                    case wf_state::LGH_AMBIENT:
                        state.options.lighting = wf_state::LGH_DIFFUSE;
                        break;
                    case wf_state::LGH_DIFFUSE:
                        state.options.lighting = wf_state::LGH_SPECULAR;
                        break;
                    case wf_state::LGH_SPECULAR:
                    default:
                        state.options.lighting = wf_state::LGH_AMBIENT;
                        break;
                }
                break;
            case Qt::Key_E:
                if(state.hovering.mode != wf_state::INT_NONE) {
                    stop_drag();
                }
                else if(state.projection.is_perspective() && p_widget->hovered_object) {
                    start_drag(const_cast<wf_state::th_object*>(p_widget->hovered_object), wf_state::INT_CARRY);
                }
                return;
            case Qt::Key_F:
                if(state.hovering.mode != wf_state::INT_NONE) {
                    stop_drag();
                }
                else if(state.projection.is_perspective() && p_widget->hovered_object) {
                    start_drag(const_cast<wf_state::th_object*>(p_widget->hovered_object), wf_state::INT_ROTATE);
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
            case Qt::Key_E:
                if(state.hovering.mode != wf_state::INT_NONE) {
                    stop_drag();
                }
                return;
            case Qt::Key_F:
                if(state.hovering.mode != wf_state::INT_NONE) {
                    stop_drag();
                }
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
        if(state.hovering.mode == wf_state::INT_CARRY) {
            d_carrying *= 1 - event->angleDelta().y() / 1000.0f;
        }
        else if(state.hovering.mode == wf_state::INT_ROTATE) {
            drag_target->scale *= 1 - event->angleDelta().y() / 1000.0f;
            drag_target->vertices_world.clear();
        }
        else if(state.projection.is_perspective()) {
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
            start_drag(state.hovering.object, wf_state::INT_DRAG);
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        // ...

        stop_drag();
    }

    void resizeEvent(QResizeEvent* event) override
    {
        state.viewport.width = p_widget->width() / state.viewport.scale;
        state.viewport.height = p_widget->height() / state.viewport.scale;

        state.projection.set_wh_ratio(float(state.viewport.width) / state.viewport.height);
    }

    wf_state::th_object* drag_target = nullptr;
    QPoint xy_prev;

    vec3f d_carrying;

    void start_drag(wf_state::th_object* new_target, wf_state::interaction_mode mode)
    {
        switch(mode) {
            case wf_state::INT_DRAG:
                xy_prev = QCursor::pos();
            case wf_state::INT_ROTATE:
            case wf_state::INT_CARRY:
                break;
            default:
                throw std::logic_error("Unexpected mode " + std::to_string(mode));
        }

        state.hovering.fixed = true;
        drag_target = state.hovering.object = new_target;

        if(mode == wf_state::INT_CARRY) {
            d_carrying = mx_tx::rotate_xyz(state.camera.orient).inverse() * (drag_target->pos - state.camera.pos);
        }

        state.hovering.mode = mode;
    }

    void stop_drag()
    {
        if(drag_target == nullptr) {
            return;
        }

        if(state.hovering.mode == wf_state::INT_DRAG) {
            p_widget->setMouseTracking(false);
        }

        state.hovering.fixed = false;
        drag_target = state.hovering.object = nullptr;

        state.hovering.mode = wf_state::INT_NONE;
    }

    void update_drag()
    {
        if(state.hovering.mode == wf_state::INT_DRAG) {
            const QPoint xy_current = QCursor::pos();
            const QPoint xy_rel = xy_current - xy_prev;

            xy_prev = xy_current;

            // TODO: instead of using deltas, calculate the current position based on mouse coords transformed into world?

            // REVIEW: do I pointlessly multiply by width then divide by it?..
            const float d_x = xy_rel.x() / state.projection.scale() * 2 * p_widget->width() / p_widget->height() / p_widget->width();
            const float d_y = -xy_rel.y() / state.projection.scale() * 2 / p_widget->height();

            switch(state.projection.axis()) {
                case wf_projection::X:
                    drag_target->pos += { 0, d_x, d_y };
                    break;
                case wf_projection::Y:
                    drag_target->pos += { d_x, 0, d_y };
                    break;
                case wf_projection::Z:
                    drag_target->pos += { d_x, d_y, 0 };
                    break;
            }

            drag_target->vertices_world.clear();
        }
        else if(state.hovering.mode == wf_state::INT_CARRY) {
            drag_target->pos = state.camera.pos + mx_tx::rotate_xyz(state.camera.orient) * d_carrying;
            // TODO: keep relative orientation as well

            drag_target->vertices_world.clear();
        }
        else if(state.hovering.mode == wf_state::INT_ROTATE) {
            const QPoint xy_rel = QCursor::pos() - xy_center;

            const float d_x = xy_rel.x() / state.projection.scale() * 2 * p_widget->width() / p_widget->height() / p_widget->width();
            const float d_y = -xy_rel.y() / state.projection.scale() * 2 / p_widget->height();

            if(d_x != 0 || d_y != 0) {
                /**
                 * Horizontal motion rotates around world's Z axis, vertical motion rotates around camera's X axis
                 */

                const mat_sq4f rot_camera = mx_tx::rotate_xyz(state.camera.orient).transpose();

                const mat_sq4f new_rot = rot_camera.transpose() * mx_tx::rotate_x(-d_y / 5.0f) * rot_camera * mx_tx::rotate_z(d_x / 5.0f) * mx_tx::rotate_xyz(drag_target->orient);

                drag_target->orient = mx_tx::rotate_to_xyz(new_rot);
                drag_target->vertices_world.clear();

                QCursor::setPos(xy_center);
            }
        }
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

        freelook_on = state.hovering.free_look = true;
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
        freelook_on = state.hovering.free_look = false;
    }

    /**
     * Smooth the angular velocity with a 2-frame moving average. The improvement is noticeable on a 1920-wide viewport
     */
    float r_xprev, r_yprev;

    void update_freelook()
    {
        const QPoint xy_rel = QCursor::pos() - xy_center;

        const float r_x = calc_x_rotation(xy_rel.x()), r_y = calc_y_rotation(xy_rel.y());

        if(state.hovering.mode != wf_state::INT_ROTATE) {
            vec3f& orient = state.camera.orient;

            orient.x -= (r_yprev + r_y) / 2.0f;

            if(orient.x < 0) {
                orient.x = 0.0f;
            }

            if(orient.x > (float)M_PI) {
                orient.x = (float)M_PI;
            }

            orient.z -= (r_xprev + r_x) / 2.0f;

            QCursor::setPos(xy_center);
        }

        r_xprev = r_x;
        r_yprev = r_y;
    }

    float calc_x_rotation(int d_x) const
    {
        return 1.0f * d_x * (float)M_PI / p_widget->width();
    }

    float calc_y_rotation(int d_y) const
    {
        return 1.0f * d_y * (float)M_PI / p_widget->height();
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
