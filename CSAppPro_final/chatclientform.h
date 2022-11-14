#ifndef CHATCLIENTFORM_H
#define CHATCLIENTFORM_H

#include <QWidget>
#include <QDataStream>
#include "chatserverform.h"

class QTextEdit;
class QLineEdit;
class QTcpSocket;
class QPushButton;
class QFile;
class QProgressDialog;
class ClientLogThread;

class ChatClientForm : public QWidget
{
    Q_OBJECT

public:
    const int PORT_NUMBER = 8100;               // 포트 번호 : 8100번

    ChatClientForm(QWidget *parent = nullptr);  // ChatClientForm 생성자
    ~ChatClientForm();                          // ChatClientForm 소멸자

private slots:
    void receiveData( );			// 서버에서 데이터가 올 때
    void sendData( );               // 서버로 데이터를 보낼 때
    void disconnect( );             // 서버와 연결이 끊어졌을 때
    void sendProtocol(Chat_Status, char*, int = 1020);  // 프로토콜을 생성해서 서버로 전송
    void sendFile();                // 파일 전송 : 파일을 열고 파일명을 받아오는 슬롯
    void goOnSend(qint64);          // 파일 전송시 분할해서 전송하는 슬롯
    void LogInCheckSended(int);     // 클라이언트에서 입력한 고객명의 존재 유무를 담고 있는 슬롯

signals:
    void LogInChecked(QString);     // 로그인 시 해당 이름이 존재하는 고객인지 확인하기 위한 시그널
    void ClientName(QString);       // 고객마다 고객명으로 채팅 기록이 저장되도록 고객명을 전송

private:
    void closeEvent(QCloseEvent*) override;
    QLineEdit *name;                // ID(이름)을 입력하는 창
    QTextEdit *message;             // 서버에서 오는 메세지 표시용
    QLineEdit* serverAddress;       // 서버의 IP 주소
    QLineEdit* serverPort;          // 서버의 포트
    QLineEdit *inputLine;           // 서버로 보내는 메시지 입력용
    QPushButton *connectButton;     // 서버 로그인 등 접속 처리
    QPushButton *sentButton;        // 메시지 전송
    QPushButton* fileButton;        // 파일 전송
    QTcpSocket *clientSocket;		// 클라이언트용 소켓
    QTcpSocket *fileClient;         // 파일 전송을 위한 소켓
    QProgressDialog* progressDialog;    // progressbar로 파일 전송의 진행 상태 확인
    QFile* file;                    // 서버로 보내는 파일
    qint64 loadSize;                // 파일의 크기
    qint64 byteToWrite;             // 보내는 파일의 크기
    qint64 totalSize;               // 파일 전송 시 보내는 파일의 총 사이즈
    QByteArray outBlock;            // 전송되는 파일의 데이터
    bool isSent;                    // 파일 전송이 시작되었는지 확인을 위한 변수
    int flag = 0;                   // 강퇴 시 채팅방에 메시지가 보이지 않도록 하기위한 변수
    int LogInCheck;                 // 로그인 시 해당 이름이 존재하는 고객인지 확인하기 위한 변수
    QTreeWidget *clientLog;         // 고객 별 채팅 로그 기록
    ClientLogThread* clientLogThread;   // 채팅 로그 기록을 위한 변수
};
#endif // WIDGET_H
