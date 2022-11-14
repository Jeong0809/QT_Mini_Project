#include "clientlogthread.h"


#include <QTreeWidgetItem>
#include <QFile>
#include <QDateTime>
#include <QMessageBox>

ClientLogThread::ClientLogThread(QObject *parent)
        : QThread{parent}
{
    /*format 형식에 맞게 현재 로그 기록이 저장되는 시간을 저장*/
    QString format = "yyyyMMdd_hhmm";
    timeStr = QDateTime::currentDateTime().toString(format);
}

/*파일을 실행하면 파일 종료시까지 채팅 기록을 1초마다 저장*/
void ClientLogThread::run()
{
    Q_FOREVER {
        saveData_C();     // 저장 버튼 클릭시 저장
        sleep(1);         // 1분마다 저장
    }
}

/*채팅 로그에 추가되는 기록들을 갱신*/
void ClientLogThread::appendData(QTreeWidgetItem* item)
{
    ClientitemList.append(item);
}

/*1초마다 채팅 기록을 로그 파일에 추가한 후 CSV 형태로 저장*/
void ClientLogThread::saveData_C()
{
    /*채팅 로그가 있으면 파일 저장*/
    if(ClientitemList.count() > 0) {

        /*CSV 형태로 파일 저장*/
        foreach(auto item, ClientitemList) {

            /*고객별로 고객명 + 시간으로 파일명을 설정하기 위함*/
            QString clientName = item->text(2);
            QString filename = QString("%1_log_%2.txt").arg(clientName).arg(timeStr);

            /*누적된 채팅 기록을 지우고 다시 기록*/
            QFile::remove(filename);
        }

        foreach(auto item, ClientitemList) {

            /*고객별로 고객명 + 시간으로 파일명을 설정하기 위함*/
            QString clientName = item->text(2);
            QString filename = QString("%1_log_%2.txt").arg(clientName).arg(timeStr);
            QFile file(filename);

            /*파일 오픈시 쓰기 전용과 텍스트 파일일 때만 파일을 저장, 채팅 데이터 추가*/
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODeviceBase::Append))
                return;

            QTextStream out(&file);
            out << item->text(0) << ", ";
            out << item->text(1) << ", ";
            out << item->text(2) << ", ";
            out << item->text(3) << ", ";
            out << item->text(4) << ", ";
            out << item->text(5) << "\n";

            file.close();
        }
    }

    /*채팅 로그가 없으면 return*/
    else {
        return;
    }
}
