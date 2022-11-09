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
    explicit ChatServerForm(QWidget *parent = nullptr); //ChatServerForm의 생성자
    ~ChatServerForm();                                  //ChatServerForm의 소멸자

private slots:
    void acceptConnection();                // 파일 서버
    void readClient();                      // 파일 전송 위해 고객 정보 읽는 슬롯
    void clientConnect( );                  // 채팅 서버
    void receiveData( );                    // 데이터를 받을 때
    void removeClient( );                   // 연결이 끊어졌을 때 해당 고객을 log out 해주기 위한 슬롯

    void addChatClient(int id, QString name);   // 고객 정보 추가시 서버에도 고객 정보 추가하는 슬롯
    void removeChatClient(int index);       // 고객 정보 삭제시 서버에도 고객 정보 삭제하는 슬롯
    void modifyChatClient(int id, QString name, int index); // 고객 정보 변경시 서버에도 고객 정보 변경하는 슬롯
    void inviteClient();                    // 대기방에 있는 고객을 채팅방으로 초대하는 함수
    void kickOut();                         // 채팅방에 있는 고객을 대기방으로 강퇴시키는 함수

    //마우스 오른쪽 버튼 클릭 시 Invite, Kick out 출력
    void on_clientTreeWidget_customContextMenuRequested(const QPoint &pos);
    void CheckLogIn(QString);               // 클라이언트에서 보낸 고객명이 존재하는 고객인지 확인하는 슬롯

signals:
    void SendLogInChecked(int);             // 존재하는 고객인지의 확인 여부를 int형으로 시그널 전송

private:
    const int BLOCK_SIZE = 1024;    // 블록 사이즈
    const int PORT_NUMBER = 8100;   // 포트 넘버 고정
    Ui::ChatServerForm *ui;         // ChatServerForm의 UI
    QTcpServer *chatServer;         // 채팅 서버
    QTcpServer *fileServer;         // 파일 서버
    QList<QTcpSocket*> clientList;  // 로그인 된 고객 명단
    QList<int> clientIDList;        // 고객 리스트에 추가된 고객의 ID
    QHash<quint16, QString> clientNameHash;         // port(키), name(값)으로 저장하는 Hash 데이터
    QHash<QString, QTcpSocket*> clientSocketHash;   // name(키), socket(값)으로 저장하는 Hash 데이터
    QHash<QString, int> clientIDHash;               // name(키), id(값)으로 저장하는 Hash 데이터
    QMenu* menu;                    // 메뉴
    QFile* file;                    // 전송되는 파일
    QProgressDialog* progressDialog;    // 파일 전송 진행 상황 표시
    qint64 totalSize;               // 전송될 파일 총 크기
    qint64 byteReceived;            // 아직 전송되지 않은 파일
    QByteArray inBlock;             // 파일을 나눠서 전송할 때의 나눈 크기
    LogThread* logThread;           // 채팅 로그 기록을 위한 변수
};

#endif // CHATSERVERFORM_H
