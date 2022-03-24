#include <QDebug>
#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QKeyEvent>
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setAttribute(Qt::WA_InputMethodEnabled, false);
    setAttribute(Qt::WA_KeyCompression, false);
    QWidget *widget = new QWidget(this);;
    this->setCentralWidget(widget);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    QLabel *lbl = new QLabel(this);
    layout->addWidget(lbl);
    lbl->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    lbl->setText(QString("Qt: %1<br/>platform: %2").arg(qVersion(), qApp->platformName()));
    QCheckBox *chk = new QCheckBox(this);
    layout->addWidget(chk);
    chk->setText("Use app level event filter");
    connect(chk, &QCheckBox::stateChanged, qApp, [=](int state){
        if (state == Qt::Checked) {
            qApp->installNativeEventFilter(&m_filter);
        } else {
            qApp->removeNativeEventFilter(&m_filter);
        }
    });
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (!event->isAutoRepeat()) {
        qDebug() << Q_FUNC_INFO
                 << "scanCode:" << event->nativeScanCode()
                 << "keyCode:" << event->nativeVirtualKey()
                 << "key:" << event->key();
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (!event->isAutoRepeat()) {
        qDebug() << Q_FUNC_INFO
                 << "scanCode:" << event->nativeScanCode()
                 << "keyCode:" << event->nativeVirtualKey()
                 << "key:" << event->key();
    }
}
