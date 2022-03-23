#ifndef NATIVEFILTER_H
#define NATIVEFILTER_H

#include <QAbstractNativeEventFilter>

class NativeFilter : public QAbstractNativeEventFilter
{
public:
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *) override;
#else
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr*) override;
#endif

};

#endif // NATIVEFILTER_H
