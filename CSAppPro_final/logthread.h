#ifndef LOGTHREAD_H
#define LOGTHREAD_H

#include <QThread>
#include <QList>
#include <QMessageBox>

class QTreeWidgetItem;

class LogThread : public QThread
{
    Q_OBJECT
public:
    /*LogThread 클래스의 생성자*/
    explicit LogThread(QObject *parent = nullptr);

private:
    void run();         /*프로그램 실행시 종료할때까지 1분마다 채팅 로그 기록*/
    QList<QTreeWidgetItem*> itemList;   /*채팅 로그가 저장되는 리스트*/
    QString filename;   /*오늘 날짜로 저장되는 파일명*/

signals:
    void send(int data);                /*QT 내에 있는 시스템 함수*/

public slots:
    void appendData(QTreeWidgetItem*);  /*채팅 로그의 기록들을 추가*/
    void saveData();                    /*Save 버튼 클릭시 그 순간의 채팅 기록을 로그 파일에 저장*/
};

#endif // LOGTHREAD_H
