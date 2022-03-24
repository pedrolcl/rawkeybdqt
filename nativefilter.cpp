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
                    qDebug() << Q_FUNC_INFO << "key pressed. scanCode:" << kp->detail;
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
                    qDebug() << Q_FUNC_INFO << "key released. scanCode:" << kr->detail;
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
        /* https://docs.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input#keystroke-message-flags */
        if (msg->message == WM_KEYDOWN || msg->message == WM_KEYUP ||
        msg->message == WM_SYSKEYDOWN || msg->message == WM_SYSKEYUP ) {
        int vkCode = LOWORD(msg->wParam); // virtual-key code: https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
        int scanCode = LOBYTE(HIWORD(msg->lParam));  
        isRepeat = (msg->message == WM_KEYDOWN || msg->message == WM_SYSKEYDOWN) &&
                   (HIWORD(msg->lParam) & KF_REPEAT) == KF_REPEAT;
        if (!isRepeat) {
            if ( msg->message == WM_KEYDOWN || msg->message == WM_SYSKEYDOWN ) {
                qDebug() << Q_FUNC_INFO << "key pressed. scanCode:" << scanCode << "keyCode:" << vkCode;
            } else {
                qDebug() << Q_FUNC_INFO << "key released. scanCode:" << scanCode << "keyCode:" << vkCode;
            }
            return true;
        }
    }
#endif
    } else if (eventType == "mac_generic_NSEvent") {
#if defined(Q_OS_MAC)
        static MacEventHelper* helper = new MacEventHelper();
        helper->setNativeEvent(message);
        if (helper->isKeyNotRepeated()) {
            int keyCode = helper->rawKeyCode();
            if (helper->isKeyPress())
                qDebug() << Q_FUNC_INFO << "key pressed. keyCode:" << keyCode;
            else
                qDebug() << Q_FUNC_INFO << "key released. keyCode:" << keyCode;
            return true;
        }
#endif
    } else {
        qDebug() << Q_FUNC_INFO << "eventType =" << eventType;
    }
    return false;
}
