#include <QApplication>
#include <QDebug>
#include "nativefilter.h"

#if defined(Q_OS_WIN)
    #include <windows.h>
    /* http://msdn.microsoft.com/en-us/library/ms646280(VS.85).aspx */
#elif defined(Q_OS_MAC)
    #include "maceventhelper.h"
#elif defined(Q_OS_LINUX)
    #include <xcb/xcb.h>
    #if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        #include <QX11Info>
        // private header, avoid if possible...
        //#include <qpa/qplatformnativeinterface.h>
    #else // needs Qt6 >= 6.2
        #include <QGuiApplication>
    #endif
#endif

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
bool NativeFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *)
#else
bool NativeFilter::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *)
#endif
{
    if (eventType == "xcb_generic_event_t") {
#if defined(Q_OS_LINUX)
        static xcb_timestamp_t last_rel_time = 0;
        static xcb_keycode_t last_rel_code = 0;
        static xcb_connection_t *connection = nullptr;
        bool isRepeat = false;
        if (connection == nullptr) {
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
            connection = QX11Info::connection();
            // private API, avoid if possible...
            //QPlatformNativeInterface *native = qApp->platformNativeInterface();
            //void *conn = native->nativeResourceForWindow(QByteArray("connection"), 0);
            //connection = reinterpret_cast<xcb_connection_t *>(conn);
#else // needs Qt6 >= 6.2
            if (auto x11nativeitf = qApp->nativeInterface<QNativeInterface::QX11Application>()) {
                connection = x11nativeitf->connection();
            }
#endif
        }
        xcb_generic_event_t* ev = reinterpret_cast<xcb_generic_event_t *>(message);
        switch (ev->response_type & ~0x80) {
            case XCB_KEY_PRESS:
            {
                xcb_key_press_event_t *kp = reinterpret_cast<xcb_key_press_event_t *>(ev);
                if ((last_rel_code != 0) && (last_rel_time != 0)) {
                    isRepeat = ((kp->detail == last_rel_code) && (kp->time == last_rel_time));
                    last_rel_code = 0;
                    last_rel_time = 0;
                }
                if (!isRepeat) {
                    qDebug() << "key pressed:" << kp->detail;
                }
                return true;
            }
            break;

            case XCB_KEY_RELEASE:
            {
                xcb_key_release_event_t *kr = reinterpret_cast<xcb_key_release_event_t *>(ev);
                if (connection) {
                    xcb_query_keymap_cookie_t cookie = xcb_query_keymap(connection);
                    xcb_query_keymap_reply_t *keymap = xcb_query_keymap_reply(connection, cookie, 0);
                    isRepeat = (keymap->keys[kr->detail / 8] & (1 << (kr->detail % 8)));
                    free(keymap);
                    last_rel_code = kr->detail;
                    last_rel_time = kr->time;
                }
                if (!isRepeat) {
                    qDebug() << "key released:" << kr->detail;
                }
                return true;
            }
            break;

        }
#endif
    } else if (eventType == "windows_dispatcher_MSG" || eventType == "windows_generic_MSG") {
#if defined(Q_OS_WIN)
        bool isRepeat = false;
        MSG* msg = static_cast<MSG *>(message);
        if (msg->message == WM_KEYDOWN || msg->message == WM_KEYUP) {
            int keycode = HIWORD(msg->lParam) & 0xff;
            isRepeat = (msg->message == WM_KEYDOWN) &&
                       ((HIWORD(msg->lParam) & 0x4000) != 0);
            if (!isRepeat) {
                if ( msg->message == WM_KEYDOWN )
                    //return m_handler->handleKeyPressed(keycode);
                    qDebug() << "key pressed:" << keycode;
                else
                    //return m_handler->handleKeyReleased(keycode);
                    qDebug() << "key released:" << keycode;
            }
            return true;
        }
#endif
    } else if (eventType == "mac_generic_NSEvent") {
#if defined(Q_OS_MAC)
        static MacEventHelper* helper = new MacEventHelper();
        helper->setNativeEvent(message);
        if (helper->isKeyNotRepeated()) {
            int keyCode = helper->rawKeyCode();
            if (helper->isKeyPress())
                qDebug() << "key pressed:" << keyCode;
            else
                qDebug() << "key released:" << keyCode;
            return true;
        }
#endif
    } else {
        qDebug() << "eventType =" << eventType;
    }
    return false;
}
