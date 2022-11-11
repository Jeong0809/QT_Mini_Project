#include "logthread.h"

#include <QTreeWidgetItem>
#include <QFile>
#include <QDateTime>
#include <QMessageBox>

LogThread::LogThread(QObject *parent)
    : QThread{parent}
{
    /*채팅 로그 기록시 기록 당시 현재의 년, 월, 일, 분, 초를 통해 로그 파일의 제목 설정*/
    QString format = "yyyyMMdd_hhmmss";
    filename = QString("log_%1.txt").arg(QDateTime::currentDateTime().toString(format));
}

/*파일을 실행하면 파일 종료시까지 채팅 기록을 1분마다 저장, 저장버튼을 눌렀을 때 저장 되도록 설정*/
void LogThread::run()
{
    Q_FOREVER {
        saveData();     // 저장 버튼 클릭시 저장
        sleep(60);      // 1분마다 저장
    }
}

/*채팅 로그에 추가되는 기록들을 갱신*/
void LogThread::appendData(QTreeWidgetItem* item)
{
    itemList.append(item);
}

/*Save 버튼 클릭시 그 순간의 채팅 기록을 로그 파일에 추가한 후 CSV 형태로 저장*/
void LogThread::saveData()
{
    /*채팅 로그가 있으면 파일 저장*/
    if(itemList.count() > 0) {
        QFile file(filename);

        /*파일 오픈시 쓰기 전용과 텍스트 파일일 때만 파일을 저장*/
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        QTextStream out(&file);

        /*CSV 형태로 파일 저장*/
        foreach(auto item, itemList) {
            out << item->text(0) << ", ";
            out << item->text(1) << ", ";
            out << item->text(2) << ", ";
            out << item->text(3) << ", ";
            out << item->text(4) << ", ";
            out << item->text(5) << "\n";
        }
        file.close();
    }
    /*채팅 로그가 없으면 return*/
    else {
        return;
    }
}
