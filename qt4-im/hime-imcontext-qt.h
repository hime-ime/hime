#include <QApplication>
#include <QEvent>
#include <QFont>
#include <QInputContext>
#include <QInputMethodEvent>
#include <QObject>
#include <QPoint>
#include <QWidget>
#include <QX11Info>

struct HIME_client_handle_S;

class HIMEIMContext: public QInputContext {
    public:
        HIMEIMContext ();
        ~HIMEIMContext ();
        bool x11FilterEvent (QWidget *widget, XEvent *event);
        bool filterEvent (const QEvent *event);
        void update();
        QString identifierName();
        QString language();
        void mouseHandler (int offset, QMouseEvent *event);
        void setFocusWidget (QWidget *widget);
        void widgetDestroyed (QWidget *widget);
        void reset ();
        HIME_client_handle_S *hime_ch;
        bool isComposing() const;
        void update_cursor(QWidget *);
        void update_preedit();
};
