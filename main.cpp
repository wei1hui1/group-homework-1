#include "mainwindow.h"

#include <QApplication>
#include <QGuiApplication>

int main(int argc, char *argv[])
{
    // 启用高DPI缩放支持
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    // 启用高DPI pixmap支持
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    QApplication a(argc, argv);
    MainWindow w;
    // 明确设置窗口大小
    w.resize(397, 663);
    w.show();
    return a.exec();
}
