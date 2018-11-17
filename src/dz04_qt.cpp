
#include <vector>
#include <array>
#include <list>
#include <iostream>  // REMOVE: !

#include <QApplication>
#include <QKeyEvent>
#include <QStackedLayout>

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

        auto* layout = new QStackedLayout;
        layout->addWidget(p_widget);
        setLayout(layout);

        auto* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, p_widget, &display2d_widget::animate);
        timer->start(50);

        /**
         * A public data structure with the complete state, which is passed by reference to both the updater and
         * display2d. Inputs are to be processed right in this class, or something.
         */
    }

    display2d_widget* p_widget;

    void keyPressEvent(QKeyEvent* event) override
    {
        switch(event->key()) {
            case Qt::Key_Equal:
                p_widget->chlen = !p_widget->chlen;
                return;
            case Qt::Key_W:
                p_widget->huii += 1;
                return;
            case Qt::Key_S:
                p_widget->huii -= 1;
                return;
            default:
                std::cout << event->text().toStdString() << std::endl;
        }
    }
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    wf_viewer display;
    display.show();

    return QApplication::exec();
}

//#include "dz04_qt.moc"
