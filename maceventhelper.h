#ifndef MACEVENTHELPER_H
#define MACEVENTHELPER_H

class MacEventHelper
{
public:
    MacEventHelper();
    ~MacEventHelper();
    void setNativeEvent(void *p);
    bool isKeyPress();
    bool isKeyNotRepeated();
    int rawKeyCode();

private:
    class Private;
    Private *d;
};

#endif // MACEVENTHELPER_H
