#include <QtGui/QKeyEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QInputMethod>
#include <QtGui/QTextCharFormat>
#include <QtGui/QPalette>
#include <QtGui/QWindow>

#include <unistd.h>
#include <errno.h>
#include <signal.h>

// confliction of qt & x11
typedef unsigned int KeySym;
struct Display;
typedef unsigned int Window;
typedef struct {
    short x, y;
} XPoint;

#include "../util.h"
#include "hime-im-client.h"
#include "hime-imcontext-qt.h"

static WId focused_win;

#include <qpa/qplatformnativeinterface.h>

#if DEBUG
FILE *out_fp;
void __hime_dbg_(const char *fmt,...)
{
    va_list args;

    if (!out_fp) {
#if 0
        out_fp = fopen("/tmp/a.txt", "w");
#else
        out_fp = stdout;
#endif
    }

    va_start(args, fmt);
    vfprintf(out_fp, fmt, args);
    fflush(out_fp);
    va_end(args);
}
#endif

QHimePlatformInputContext::QHimePlatformInputContext()
{
    dbg("QHimePlatformInputContext::QHimePlatformInputContext() \n");
    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    if(!native)
        return;
    Display *display = static_cast<Display *>(native->nativeResourceForWindow("display", NULL));	
    printf("display %p\n", display);

    if (!(hime_ch = hime_im_client_open(display))) {
        perror("cannot open hime_ch");
        dbg("hime_im_client_open error\n");
        return;
    }

    dbg("QHimePlatformInputContext succ\n");
}

QHimePlatformInputContext::~QHimePlatformInputContext()
{
    if (hime_ch==NULL)
        return;
      hime_im_client_close(hime_ch);
    hime_ch = NULL;
}


bool QHimePlatformInputContext::isValid() const
{
    dbg("QHimePlatformInputContext::isValid()\n");
    return true;
}

void QHimePlatformInputContext::invokeAction(QInputMethod::Action action, int cursorPosition)
{
    dbg("QHimePlatformInputContext::invokeAction(n");
    // FIXME nop? remove me
}

void QHimePlatformInputContext::commitPreedit()
{
    dbg("QHimePlatformInputContext::commitPreedit\n");
    // use this to flush
    int preedit_cursor_position=0;
    int sub_comp_len;
    char *str=NULL;
    HIME_PREEDIT_ATTR att[HIME_PREEDIT_ATTR_MAX_N];
    hime_im_client_get_preedit(hime_ch, &str, att, &preedit_cursor_position, &sub_comp_len);
    if (str) {
        if (strlen(str) > 0) {
            dbg("send enter to flush\n");
            send_key_press(0xff0d, 0); // Enter
        } else {
            dbg("empty string\n");
        }

        free(str);
        update_preedit();
    } else {
        dbg("no str\n");
    }
}


void QHimePlatformInputContext::reset()
{
    dbg("QHimePlatformInputContext::reset()\n");
    if (hime_ch) {
        hime_im_client_reset(hime_ch);
        update_preedit();
    }
}

void QHimePlatformInputContext::update(Qt::InputMethodQueries queries )
{
    dbg("QHimePlatformInputContext::update\n");
    QObject *input = qApp->focusObject();
    if (!input)
        return;

    QInputMethodQueryEvent query(queries);
    QGuiApplication::sendEvent(input, &query);

    if (queries & Qt::ImCursorRectangle) {
        cursorMoved();
    }
}

// this one is essential
void QHimePlatformInputContext::commit()
{
    dbg("QHimePlatformInputContext::commit()\n");
    commitPreedit();
}


void QHimePlatformInputContext::setFocusObject(QObject* object)
{
    dbg("QHimePlatformInputContext::setFocusObject\n");
    QWindow *window = qApp->focusWindow();
    if (!window) {
        dbg("no window, focus out\n");
        focused_win = 0;
        char *rstr = NULL;
        hime_im_client_focus_out2(hime_ch, &rstr);
        if (rstr) {
            send_str(rstr);
        } else {
            dbg("no str in preedit\n");
        }
        return;
    }

    WId win = window->winId();

    if (focused_win && win != focused_win) {
        if (hime_ch) {
            hime_im_client_focus_out(hime_ch);
        }
    }

    focused_win = win;

    if (hime_ch) {
        hime_im_client_set_window(hime_ch, win);
        hime_im_client_focus_in(hime_ch);
        cursorMoved();
    }
}

static int last_x=-1, last_y=-1;

void QHimePlatformInputContext::cursorMoved()
{
    dbg(" QHimePlatformInputContext::cursorMoved()\n");

    QWindow *inputWindow = qApp->focusWindow();
    if (!inputWindow)
        return;

    QRect r = qApp->inputMethod()->cursorRectangle().toRect();
    if(!r.isValid())
        return;

    // hime server will clear the string if the cursor is moved, make sure the x,y is valid
    int x = r.left(),  y = r.bottom();
    if (x > inputWindow->width() || y > inputWindow->height() || x < 0 || y < 0)
        return;

    if (hime_ch && (x !=  last_x || y != last_y)) {
        last_x = x; last_y = y;
        dbg("move cursor %d, %d\n", x, y);
        hime_im_client_set_cursor_location(hime_ch, x,  y);
    }
}



void QHimePlatformInputContext::update_preedit()
{
    if (!hime_ch)
        return;
    QList<QInputMethodEvent::Attribute> attrList;
//  QString preedit_string;
    int preedit_cursor_position=0;
    int sub_comp_len;
    char *str=NULL;
    HIME_PREEDIT_ATTR att[HIME_PREEDIT_ATTR_MAX_N];
    int attN = hime_im_client_get_preedit(hime_ch, &str, att, &preedit_cursor_position, &sub_comp_len);

    int ret;
    hime_im_client_set_flags(hime_ch, FLAG_HIME_client_handle_use_preedit, &ret);

    QObject *input = qApp->focusObject();

    if (!input || !str) {
      free(str);
      return;
    }


#if DEBUG
    dbg("update_preedit attN:%d '%s'\n", attN, str);
#endif

    int i;
    for(i=0; i < attN; i++) {
        int ofs0 = att[i].ofs0;
        int len = att[i].ofs1 - att[i].ofs0;
        QTextCharFormat format;

        switch (att[i].flag) {
            case HIME_PREEDIT_ATTR_FLAG_REVERSE:
            {
                QBrush brush;
                QPalette palette;
                palette = QGuiApplication::palette();
                format.setBackground(QBrush(QColor(palette.color(QPalette::Active, QPalette::Highlight))));
                format.setForeground(QBrush(QColor(palette.color(QPalette::Active, QPalette::HighlightedText))));
            }
            break;
            case HIME_PREEDIT_ATTR_FLAG_UNDERLINE:
            {
                format.setUnderlineStyle(QTextCharFormat::DashUnderline);
            }
            break;
            default:
                ;
        }

        attrList.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, ofs0,  len, format));
    }

    attrList.append(QInputMethodEvent::Attribute(QInputMethodEvent::Cursor,  preedit_cursor_position, 1, 0));

    QInputMethodEvent im_event (QString::fromUtf8(str), attrList);
    send_event (im_event);
    free(str);
}

void QHimePlatformInputContext::send_event(QInputMethodEvent e) {
    QObject *input = qApp->focusObject();
    if (!input)
        return;
    QCoreApplication::sendEvent(input, &e);
}

void QHimePlatformInputContext::send_str(char *rstr) {
    dbg("send_str %s\n",  rstr);
    QString inputText = QString::fromUtf8(rstr);
    free(rstr);
    QInputMethodEvent commit_event;
    commit_event.setCommitString (inputText);
    send_event (commit_event);
}

bool QHimePlatformInputContext::send_key_press(quint32 keysym, quint32 state) {
    dbg("send_key_press\n");
    char *rstr  = NULL;
    int result = hime_im_client_forward_key_press(hime_ch, keysym, state, &rstr);

    if (rstr) {
        send_str(rstr);
    }

    return result;
}

bool QHimePlatformInputContext::filterEvent(const QEvent* event)
{
    dbg("QHimePlatformInputContext::filterEvent\n");
    if (event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease) {
        goto ret;
    }

    const QKeyEvent* keyEvent;
    keyEvent = static_cast<const QKeyEvent*>(event);
    quint32 keysym ;
    keysym = keyEvent->nativeVirtualKey();
    quint32  state;
    state = keyEvent->nativeModifiers();

    if (!inputMethodAccepted()) {
        goto ret;
    }

    QObject *input;
    input = qApp->focusObject();

    if (!input) {
        goto ret;
    }

    int result;
    if (event->type() == QEvent::KeyPress) {
        if (send_key_press(keysym, state)) {
            update_preedit();
            return true;
        }
    } else {
        char *rstr = NULL;
        result = hime_im_client_forward_key_release(hime_ch,   keysym, state, &rstr);
        if (rstr) {
            free(rstr);
        }

        if (result) {
            return true;
        }
    }

ret:
    return QPlatformInputContext::filterEvent(event);
}
