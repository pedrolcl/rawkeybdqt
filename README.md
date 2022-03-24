# RawKbdQt
Using Raw Keyboard Events in Qt Applications

## Introduction
Here is a bare-bones Qt GUI application processing computer keyboard events at a low level. The "raw keyboard events" use numeric codes early available before translating the keystrokes to alphanumeric characters, and the auto-repeat events should be ignored or discarded.

The use cases for this kind of low level processing are many. Games that use the computer keyboard as a game controller would be one target. My personal case is the [VMPK application](https://github.com/pedrolcl/VMPK) (a virtual MIDI controller) which emulates a piano keyboard using the computer keyboard (among other inputs); each computer keyboard key can be mapped to a MIDI note, and this mapping is configurable by the user.

At this moment, the VMPK program is already released for Windows, macOS and Linux/Unix using X11. The goal is to support Linux using Wayland as well (and alternative platforms in the future). This project also should ...

* Work with Qt5 and Qt6
* Support Windows, macOS and Linux (both X11 and Wayland)
* Use CMake and Qmake build-systems

## What problems has QKeyEvent
[QKeyEvent](https://doc.qt.io/qt-6/qkeyevent.html) is received by a Widget in Qt apps after the low level keyboard event has been already processed by the underlying platform and the input methods to produce characters that could be used to input text data for programs (this is of course the most general use case of keyboard input). QKeyEvent has several functions that identify the key producing the event. Two event handlers are available: keyPressEvent() and keyReleaseEvent(), both receiving a QKeyEvent parameter. The relevant QKeyEvent functions for this project are:

* int key(): Returns the code of the key that was pressed or released. These codes are independent of the underlying window system.
* bool isAutoRepeat(): Returns true if this event comes from an auto-repeating key.
* quint32 nativeScanCode(): Returns the native scan code of the key event. If the key event does not contain this data 0 is returned.
* quint32 nativeVirtualKey(): Returns the native virtual key, or key sym of the key event. If the key event does not contain this data 0 is returned.

Some of the observed problems with QKeyEvent are:

* key() dependency on the national layout of the keyboard: each national keyboard layout needs a different mapping from key() to MIDI note. On the other hand, the numeric identifiers of the raw keys (either scan codes or key codes) depend only on the operating system (and keyboard driver) requiring a single mapping for each platform.
* there dead keys usually found in national layouts (for instance: accents) that complicate the event delivery, and the reliability of the detected keys. Dead keys should be usable like any other key for MIDI note mappings.

It is not possible to have a single keyboard mapping both cross-platform and cross-language, but supporting three mappings (Lin/Win/Mac) is better than hundreds for all the languages.

## Alternative: application level native event filter.
[QCoreApplication](https://doc.qt.io/qt-6/qcoreapplication.html) has the option to filter events before they are processed by Qt or the input methods, and before they are delivered to the application window or focused widget. This project uses a NativeFilter class derived from [QAbstractNativeEventFilter](https://doc.qt.io/qt-6/qabstractnativeeventfilter.html) to implement this alternative.

The native filter can process the event and return 'true', stopping the events from further processing, or return 'false' so the events can be processed as usual by Qt. With this alternative dead keys can be processed like any normal key, and auto-repeat events can be detected and discarded easily. On the other hand, some platform-dependent native code is required, but the flexibility is premium.

One apparent inconvenience is that processing key events at a global application level instead of widget level is a limitation. But QKeyEvents are always received by a single widget: the widget that has the focus in the active window. It should be very easy to emulate the focus mechanism adding a handler property to the NativeFilter class, so it may call a method synchronously on any other class of the application for processing. Or alternatively, the NativeFilter instance may call QCoreApplication::postEvent(handler, qevent) to allow asynchronous processing by arbitrary objects. Deriving the NativeFilter class from QObject (thanks to C++ multiple inheritance) makes it possible to use Qt signals as well.

## Results
:heavy_check_mark: means that it is usable.

:x: means that it is not usable. See the notes.

:question: means that the results are not conclusive.

| QKeyEvent        | Windows            | macOS              | Linux X11          | Wayland            |
| ---------------- + ------------------ + ------------------ + ------------------ + ------------------ |
| dead keys        | :x: [^1]           | :heavy_check_mark: | :heavy_check_mark: | :question:         |
| auto repeat      | :x: [^2]           | :heavy_check_mark: | :heavy_check_mark: | :question:         |
| scan code        | :heavy_check_mark: | :x: [^3]           | :heavy_check_mark: | :question:         |
| virtual key code | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :question:         |


| NativeFilter     | Windows            | macOS              | Linux X11          | Wayland            |
| ---------------- + ------------------ + ------------------ + ------------------ + ------------------ |
| dead keys        | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :x: [^4]           |
| isAutoRepeat     | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :x: [^4]           |
| nativeScanCode   | :heavy_check_mark: | :x: [^3]           | :heavy_check_mark: | :x: [^4]           |
| nativeVirtualKey | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :x: [^4]           |


[^1]: unusable: dead keys are not always received
[^2]: unreliable: some repeated keys return false
[^3]: not accessible: cocoa does not allow to access this code
[^4]: not possible: the native filter is never called
