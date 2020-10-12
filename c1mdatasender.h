#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <vector>
#include <string>
#include <QUdpSocket>
#include <QTimer>

/******************************************************************************/




/******************************************************************************/

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class C1MDataSender : public QWidget
{
    Q_OBJECT

public:
    C1MDataSender(QWidget *parent = nullptr);
    ~C1MDataSender();

private:
    Ui::Widget *ui;
    std::vector<std::string> sData;
    QUdpSocket udpSocket;
    QHostAddress groupAddress;
    QTimer timer;
    uint16_t    counter;

    void loadData(const std::string &filename = "sensor_data.txt");
    bool readFile(const std::string &file, int ignoreSizeString = 50);

private slots:
    void sendDatagram();
    void on_pbtnOpen_clicked();
    void on_pBtnInfo_clicked();
};
#endif // WIDGET_H
