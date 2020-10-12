#include "c1mdatasender.h"
#include "ui_widget.h"
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cassert>
#include <QMessageBox>
#include <QFileDialog>

/******************************************************************************/

C1MDataSender::C1MDataSender(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setLayout(ui->gridLayout);
    groupAddress.setAddress("235.0.0.1");
    udpSocket.bind(QHostAddress(QHostAddress::AnyIPv4), 0);
    udpSocket.setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
    connect(&timer, SIGNAL(timeout()), this, SLOT(sendDatagram()));
    loadData("sensor_data.txt");
    timer.start(100);
}

/******************************************************************************/

C1MDataSender::~C1MDataSender()
{
    delete ui;
}

/******************************************************************************/

void C1MDataSender::loadData(const std::string &filename)
{
    ui->tblWidget->clear();
    sData.clear();

    if (readFile(filename)) {
        ui->lblInfo->setText("Номер пакета UDP:");
        ui->lblCounter->show();
    } else {
        ui->lblInfo->setText("Файл с данными в текущем каталоге не найден.");
        ui->lblCounter->hide();
        return;
    }

    if (sData.size() == 0) return;

    ui->tblWidget->setColumnCount(20);
    ui->tblWidget->setRowCount(static_cast<int>(sData.size())-1);
    ui->tblWidget->setShowGrid(true);
    ui->tblWidget->resizeColumnsToContents();
    ui->tblWidget->verticalHeader()->hide();

    std::istringstream iss(sData.at(0));
    for(int j=0; j<20; j++){
        std::string subs;
        iss >> subs;
        ui->tblWidget->setHorizontalHeaderItem(j, new QTableWidgetItem(subs.c_str()));
    }

    for (unsigned long i = 0; i < (sData.size() - 1); i++){
        std::istringstream iss(sData.at(i+1));
        for(int j=0; j<20; j++){
            std::string subs;
            iss >> subs;
            ui->tblWidget->setItem(static_cast<int>(i), j, new QTableWidgetItem(subs.c_str()));
        }
    }
}

/******************************************************************************/

bool C1MDataSender::readFile(const std::string &file, int ignoreSizeString){

    assert(ignoreSizeString > 0);
    std::ifstream fin(file);
    if(!fin.is_open())
        return false;

    // Replacing commas
    auto editString{
        [](std::string &s) {
            replace_if(s.begin(), s.end(), [](char c) { return c == ','; }, '.');
            return s.erase(0, 1).append("\n");
        }
    };

    // Reading data
    std::string  str;
    while (getline(fin, str)) {
        if (str.size() > static_cast<unsigned>(ignoreSizeString))
            sData.push_back(editString(str));
    }

    fin.close();
    return true;
}

/******************************************************************************/

void C1MDataSender::sendDatagram(){

    if (sData.size() == 0) return;

    QByteArray datagram;
    QDataStream out(&datagram, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_3);

    uint16_t    sm[18],
                hz = 0,
                z = 123;

    std::istringstream iss(sData.at(static_cast<unsigned long>(ui->tblWidget->currentRow())+1));
    for(int i=0; i<20; i++){
        float f;
        iss >> f;
        if (i > 0 && i < 19)
            sm[i-1] = static_cast<uint16_t>((f + 10)/20*65535);
        if (i == 19)
            hz = static_cast<uint16_t>(f/5000*65535);
    }

    counter++;
    ui->lblCounter->setNum(counter);
    out << counter << z << z << z << z
        << sm[0] << sm[1] << sm[2] << sm[3] << sm[4] << sm[5] << sm[6] << sm[17] << hz
        << counter << z << z << z << z
        << sm[0] << sm[7] << sm[8] << sm[9] << sm[10] << sm[11] << sm[12] << sm[17] << hz
        << counter << z << z << z << z
        << sm[0] << sm[13] << sm[14] << sm[15] << sm[16] << z << z << sm[17] << hz;
    udpSocket.writeDatagram(datagram, QHostAddress::LocalHost, 1234);
}

/******************************************************************************/

void C1MDataSender::on_pbtnOpen_clicked()
{
    QString str = QFileDialog::getOpenFileName(nullptr, "Open sDialog", "", "*.txt");
    if (!str.isEmpty())
        loadData(str.toStdString());
}

/******************************************************************************/

void C1MDataSender::on_pBtnInfo_clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Справка");
    msgBox.setText("Для иммитации сигналов датчика уровня используется "
                   "данные из файла \"sensor_data.txt\", который должен "
                   "находиться в текущем каталоге. Можно выбрать любой другой "
                   "текстовый файл данных, содержащий заглавную строку, "
                   "определяющую формат записи данных в файле:\n\n"
                   "	mm	Signal	m1	m2	m3	m4	m5	m6	m7	m8	m9"
                   "	m10	m11	m12	m13	m14	m15	m16	Vpit	Hz"
                   "\n\n  mm - положение поплавка,"
                   "\n  Signal, m1...m16 - значения сигналов датчика,"
                   "\n  Vpit - значение напряжения питания датчика,"
                   "\n  Hz - значение частоты сигнала датчика."
                   "\n\nВ случае успешного открытия файла приложение "
                   "начинает формировать UDP пакеты с данными строки, "
                   "соответствующей активной (выбранной) ячейке таблицы."
                   );
    msgBox.exec();
}

