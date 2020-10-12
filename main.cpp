#include "c1mdatasender.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    C1MDataSender w;
    w.setWindowTitle("Имитатор потока данных преобразователя сигналов датчика С1М (v0.1.1)");
    w.setGeometry(800, 300, 300, 150);
    w.show();
    return a.exec();
}
