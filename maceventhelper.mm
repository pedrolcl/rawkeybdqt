#include "maceventhelper.h"
#include <Cocoa/Cocoa.h>

class MacEventHelper::Private
{
public:
   NSEvent *m_event;
};

MacEventHelper::MacEventHelper()
{
    d = new Private();
}

MacEventHelper::~MacEventHelper()
{
    delete d;
}

void
MacEventHelper::setNativeEvent(void *p)
{
    d->m_event = static_cast<NSEvent *>(p);
}

bool
MacEventHelper::isKeyPress()
{
    return ([d->m_event type] == NSEventTypeKeyDown);
}

bool
MacEventHelper::isKeyNotRepeated()
{
    return ( ([d->m_event type] == NSEventTypeKeyDown) || ([d->m_event type] == NSEventTypeKeyUp) )
           && ([d->m_event isARepeat] == NO);
}

int
MacEventHelper::rawKeyCode()
{
    return [d->m_event keyCode];
}

