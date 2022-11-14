#ifndef CLIENTLOGTHREAD_H
#define CLIENTLOGTHREAD_H

#include <QThread>
#include <QList>
#include <QMessageBox>

class QTreeWidgetItem;

class ClientLogThread : public QThread
{
    Q_OBJECT
public:
    ClientLogThread(QObject *parent = nullptr);

private:
    void run();         /*프로그램 실행시 종료할때까지 1분마다 채팅 로그 기록*/
    QList<QTreeWidgetItem*> ClientitemList;   /*채팅 로그가 저장되는 리스트*/
    QString timeStr;                          /*현재 로그 기록이 저장되는 시간*/

signals:
    void send(int data);                /*QT 내에 있는 시스템 함수*/

public slots:
    void appendData(QTreeWidgetItem*);  /*채팅 로그의 기록들을 추가*/
    void saveData_C();                    /*Log in 버튼 클릭시 그 순간의 채팅 기록을 로그 파일에 저장*/
};

#endif // CLIENTLOGTHREAD_H
