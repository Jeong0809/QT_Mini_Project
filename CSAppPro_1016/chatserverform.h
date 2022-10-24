#ifndef CHATSERVERFORM_H
#define CHATSERVERFORM_H

#include <QWidget>
#include <QList>
#include <QHash>
#include <QTreeWidgetItem>

class QLabel;
class QTcpServer;
class QTcpSocket;
class QFile;
class QProgressDialog;
class LogThread;

namespace Ui {
class ChatServerForm;
}

typedef enum {
    Chat_Login,             // 로그인(서버 접속)   --> 초대를 위한 정보 저장
    Chat_In,                // 채팅방 입장
    Chat_Talk,              // 채팅
    Chat_Out,               // 채팅방 퇴장         --> 초대 가능
    Chat_LogOut,            // 로그 아웃(서버 단절) --> 초대 불가능
    Chat_Invite,            // 초대
    Chat_KickOut,           // 강퇴
} Chat_Status;

class ChatServerForm : public QWidget
{
    Q_OBJECT

public:
    explicit ChatServerForm(QWidget *parent = nullptr);
    ~ChatServerForm();

private:
    const int BLOCK_SIZE = 1024;
    const int PORT_NUMBER = 8100;

    Ui::ChatServerForm *ui;
    QTcpServer *chatServer;
    QTcpServer *fileServer;
    QList<QTcpSocket*> clientList;
    QList<int> clientIDList;
    QHash<quint16, QString> clientNameHash;                 // port, name
    QHash<QString, QTcpSocket*> clientSocketHash;
    QHash<QString, int> clientIDHash;                       // name, id
    QMenu* menu;
    QFile* file;
    QProgressDialog* progressDialog;
    qint64 totalSize;
    qint64 byteReceived;
    QByteArray inBlock;
    QList<QTreeWidgetItem*> ChatList;
    LogThread* logThread;


private slots:
    void acceptConnection();                /* 파일 서버 */
    void readClient();

    void clientConnect( );                  /* 채팅 서버 */
    void receiveData( );
    void removeClient( );

    void addChatClient(int id, QString name);
    void removeChatClient(int index);
    void modifyChatClient(int id, QString name, int index);

    void inviteClient();
    void kickOut();
    void on_clientTreeWidget_customContextMenuRequested(const QPoint &pos);

    void CheckLogIn(QString);

signals:
    void SendLogInChecked(int);
};

#endif // CHATSERVERFORM_H




//내꺼
//#ifndef CHATSERVERFORM_H
//#define CHATSERVERFORM_H

//#include <QWidget>
//#include <QList>
//#include <QHash>
//#include <QTreeWidgetItem>

//class QLabel;
//class QTcpServer;
//class QTcpSocket;
//class QFile;
//class QProgressDialog;

//namespace Ui {
//class ChatServerForm;
//}

//typedef enum {
//    Chat_Login,             // 로그인(서버 접속)   --> 초대를 위한 정보 저장
//    Chat_In,                // 채팅방 입장
//    Chat_Talk,              // 채팅
//    Chat_Out,             // 채팅방 퇴장         --> 초대 가능
//    Chat_LogOut,            // 로그 아웃(서버 단절) --> 초대 불가능
//    Chat_Invite,            // 초대
//    Chat_KickOut,           // 강퇴
//    Chat_FileTrans_Start,   // 파일 전송 시작(파일명) --> 파일 오픈
//    Chat_FileTransfer,      // 파일 데이터 전송      --> 데이터를 파일에 저장
//    Chat_FileTrans_End,     // 파일 전송 완료        --> 파일 닫기
//} Chat_Status;

//class ChatServerForm : public QWidget
//{
//    Q_OBJECT

//public:
//    explicit ChatServerForm(QWidget *parent = nullptr);
//    ~ChatServerForm();

//private:
//    const int BLOCK_SIZE = 1024;
//    const int PORT_NUMBER = 8100;

//    Ui::ChatServerForm *ui;
//    QTcpServer *tcpServer;
//    QTcpServer *fileServer;
//    QList<QTcpSocket*> clientList;
//    QList<int> clientIDList;
//    QHash<QString, QString> clientNameHash;
//    QHash<QString, QTcpSocket*> clientSocketHash;
//    QHash<QString, int> clientIDHash;
//    QMenu* menu;
//    QFile* file;
//    QProgressDialog* progressDialog;
//    qint64 totalSize;
//    qint64 byteReceived;
//    QList<QTreeWidgetItem*> ChatList;

//private slots:
//    void clientConnect( );                       /* 채팅 서버 */
//    void receiveData( );
//    void removeClient( );
//    void addClient(int, QString);

//    void addChatClient(int id, QString name);
//    void removeChatClient(int index);
//    void modifyChatClient(int id, QString name, int index);

//    void inviteClient();
//    void kickOut();
//    void on_clientTreeWidget_customContextMenuRequested(const QPoint &pos);
//    void on_savePushButton_pressed();
//};

//#endif // CHATSERVERFORM_H


