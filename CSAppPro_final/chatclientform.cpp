#include "chatclientform.h"
#include "clientlogthread.h"

#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDataStream>
#include <QTcpSocket>
#include <QApplication>
#include <QThread>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QProgressDialog>

#define BLOCK_SIZE      1024

ChatClientForm::ChatClientForm(QWidget *parent) : QWidget(parent), isSent(false) {

    /*연결한 서버 정보 입력을 위한 위젯들*/
    name = new QLineEdit(this);
    QSettings settings("ChatClient", "Chat Client");

    /*이름을 적는 lineedit에 기존에 사용했던 이름이 입력되어 있도록 설정*/
    name->setText(settings.value("ChatClient/ID").toString());

    serverAddress = new QLineEdit(this);
    /*local host를 나타내는 IP 주소*/
    serverAddress->setText("127.0.0.1");

    /*정규표현식 : 주소의 범위 (0.0.0.0 ~ 255.255.255.255)*/
    QRegularExpression re("^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");

    QRegularExpressionValidator validator(re);

    /*IP 주소 입력란에 아무 입력이 없을 때 아래의 문구가 보이도록 설정*/
    serverAddress->setPlaceholderText("Server IP Address");

    /*IP 주소의 범위 지정*/
    serverAddress->setValidator(&validator);
    serverPort = new QLineEdit(this);

    /*채팅의 포트 번호를 8000번으로 지정*/
    serverPort->setText(QString::number(PORT_NUMBER));

    /*port 번호는 5자리의 숫자로 입력 마스크 설정*/
    serverPort->setInputMask("00000;_");

    /*port 입력란에 아무 입력이 없을 때 아래의 문구가 보이도록 설정*/
    serverPort->setPlaceholderText("Server Port No");

    /*Log in, Chat in, Chat out 기능을 수행하는 커넥트 버튼 생성*/
    connectButton = new QPushButton(tr("Log In"), this);

    /*ClientChat UI 생성 후 레이아웃 설정*/
    QHBoxLayout *serverLayout = new QHBoxLayout;
    serverLayout->addWidget(name);
    serverLayout->addStretch(1);
    serverLayout->addWidget(serverAddress);
    serverLayout->addWidget(serverPort);
    serverLayout->addWidget(connectButton);

    /*서버에서 오는 메시지 표시용*/
    message = new QTextEdit(this);
    message->setReadOnly(true);

    /*고객 별 채팅 로그를 출력하기 위한 트리 위젯*/
    clientLog = new QTreeWidget(this);

    /*서버로 보낼 메시지를 위한 위젯들*/

    /*보낼 메시지를 적는 LineEdit 생성*/
    inputLine = new QLineEdit(this);

    /*버튼 클릭시 메시지 전송 후 메시지 입력란 clear*/
    connect(inputLine, SIGNAL(returnPressed( )), SLOT(sendData( )));
    connect(inputLine, SIGNAL(returnPressed( )), inputLine, SLOT(clear( )));

    /*메시지를 전송하기 위한 Send 버튼 생성*/
    sentButton = new QPushButton("Send", this);

    /*Send 클릭시 메시지 전송 후 Send 버튼 누르면 inputLine을 clear*/
    connect(sentButton, SIGNAL(clicked( )), SLOT(sendData( )));
    connect(sentButton, SIGNAL(clicked( )), inputLine, SLOT(clear( )));

    /*초기에는(접속 전) Send, input란 비활성화*/
    inputLine->setEnabled(false);
    sentButton->setEnabled(false);

    /*ClientChat UI 생성 후 레이아웃 설정*/
    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(inputLine);
    inputLayout->addWidget(sentButton);

    /*파일을 전송하기 위한 버튼 생성*/
    fileButton = new QPushButton("File Transfer", this);

    /*파일 전송 버튼 클릭 시 파일이 전송되고 초기에는 파일 전송 버튼 비활성화*/
    connect(fileButton, SIGNAL(clicked( )), SLOT(sendFile( )));
    fileButton->setDisabled(true);

    /*종료 기능을 하는 Log out 버튼 생성*/
    QPushButton* quitButton = new QPushButton("Log Out", this);

    /*Log out 버튼 클릭시 chatClient창 닫음*/
    connect(quitButton, SIGNAL(clicked( )), this, SLOT(close( )));

    /*ClientChat UI 생성 후 레이아웃 설정*/
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(fileButton);
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(quitButton);

    /*ClientChat UI 생성 후 레이아웃 설정*/
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(serverLayout);
    mainLayout->addWidget(message);
    mainLayout->addWidget(clientLog);
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    /*서버와 연결하기 위해 채팅을 위한 클라이언트 소켓 생성, 8000번*/
    clientSocket = new QTcpSocket(this);
    connect(clientSocket, &QAbstractSocket::errorOccurred,
            [=]{ qDebug( ) << clientSocket->errorString( ); });
    connect(clientSocket, SIGNAL(readyRead( )), SLOT(receiveData( )));
    connect(clientSocket, SIGNAL(disconnected( )), SLOT(disconnect( )));

    /*파일 전송을 위한 소켓 : 데이터가 많이 이동, 8001번*/
    fileClient = new QTcpSocket(this);

    /*데이터를 쪼개서 보내기 위한 커넥트*/
    connect(fileClient, SIGNAL(bytesWritten(qint64)), SLOT(goOnSend(qint64)));

    /*파일 전송 시 전송되는 퍼센트를 progressbar로 표시*/
    progressDialog = new QProgressDialog(0);
    progressDialog->setAutoClose(true);
    progressDialog->reset();

    /*connectButton을 클릭했을 때 Log in, Chat in, Chat out 기능을 수행*/
    connect(connectButton, &QPushButton::clicked, [=]{

        /*Log in 수행할 때*/
        if(connectButton->text() == tr("Log In")) {
            clientSocket->connectToHost(serverAddress->text(), serverPort->text().toInt());

            /*데이터를 보내기 전에 서버가 접속되기까지 기다리는 것*/
            clientSocket->waitForConnected();

            /*입력받은 이름이 Client List에 있는 고객인지 확인하기 위해 signal로 전송*/
            emit LogInChecked(name->text());

            if(LogInCheck == 0)
            {
                /*존재하지 않는 고객명을 입력받았을 때 경고창을 띄워주는 예외처리*/
                QMessageBox::critical(this, tr("Chatting Client"), tr("Customer name does not exist."));
                name->clear();
                return;
            }

            /*로그인 프로토콜 전송하며 입력받은 이름도 함께 전송*/
            sendProtocol(Chat_Login, name->text().toStdString().data());

            /*버튼을 Chat in으로 변경*/
            connectButton->setText(tr("Chat in"));
            name->setReadOnly(true);

          /*Chat in 수행할 때*/
        } else if(connectButton->text() == tr("Chat in")){

            /*Chat in 프로토콜 전송하며 입력받은 이름도 함께 전송*/
            sendProtocol(Chat_In, name->text().toStdString().data());

            /*버튼을 Chat out으로 변경*/
            connectButton->setText(tr("Chat Out"));

            /*입력란, send 버튼, file 전송 버튼 활성화*/
            inputLine->setEnabled(true);
            sentButton->setEnabled(true);
            fileButton->setEnabled(true);

          /*Chat out 수행할 때*/
        } else if(connectButton->text() == tr("Chat Out")){

            /*Chat out 프로토콜 전송하며 입력받은 이름도 함께 전송*/
            sendProtocol(Chat_Out, name->text().toStdString().data());

            /*버튼을 Chat in으로 변경*/
            connectButton->setText(tr("Chat in"));

            /*입력란, send 버튼, file 전송 버튼 비활성화*/
            inputLine->setDisabled(true);
            sentButton->setDisabled(true);
            fileButton->setDisabled(true);
        }
    });

    /*window 타이틀명을 chat client로 설정*/
    setWindowTitle(tr("Chat Client"));

    clientLogThread = new ClientLogThread(this);
    clientLogThread->start();

    connect(connectButton, SIGNAL(clicked()), clientLogThread, SLOT(saveData_C()));
}

ChatClientForm::~ChatClientForm( )
{
    clientSocket->close( );

    /*이름 입력란을 마지막에 사용한 이름으로 파일로 저장했다가 설정*/
    QSettings settings("ChatClient", "Chat Client");
    settings.setValue("ChatClient/ID", name->text());
}

/*창이 닫힐 때 서버에 연결 접속 메시지를 보내고 종료*/
void ChatClientForm::closeEvent(QCloseEvent*)
{
    /*로그아웃 프로토콜 전송 및 이름란에 입력된 이름 전송*/
    sendProtocol(Chat_LogOut, name->text().toStdString().data());

    /*현재 소켓의 연결을 차단*/
    clientSocket->disconnectFromHost();
    if(clientSocket->state() != QAbstractSocket::UnconnectedState)
        clientSocket->waitForDisconnected();        /*충돌을 방지하기 위해 연결 차단시 대기*/
}

/*데이터를 받을 때*/
void ChatClientForm::receiveData( )
{
    /*서버와 통신하고 있는 sender*/
    QTcpSocket *clientSocket = dynamic_cast<QTcpSocket *>(sender());
    if (clientSocket->bytesAvailable() > BLOCK_SIZE) return;

    /*데이터를 읽어옴*/
    QByteArray bytearray = clientSocket->read(BLOCK_SIZE);

    Chat_Status type;       /*채팅의 목적, 4바이트*/
    char data[1020];        /*전송되는 메시지/데이터*/
    memset(data, 0, 1020);  /*데이터를 0으로 초기화, 데이터 사이즈는 1020*/

    /*전달하기 위한 데이터를 소켓에 넣는 과정*/
    QDataStream in(&bytearray, QIODevice::ReadOnly);
    in.device()->seek(0);

    /*타입이 4바이트, 나머지 1020바이트는 data를 넣음*/
    in >> type ;
    in.readRawData(data, 1020);

    switch(type) {
    case Chat_Talk:
        /*채팅방에서 메시지를 보낼 수 있는 상태인 경우*/
        if(flag == 0){
        message->append(QString(data));
        inputLine->setEnabled(true);
        sentButton->setEnabled(true);
        fileButton->setEnabled(true);
        }

        /*kick_out으로 대기방에 있을 경우*/
        else{
            inputLine->setEnabled(false);
            sentButton->setEnabled(false);
            fileButton->setEnabled(false);
        }
        break;

    /*kick_out으로 강퇴당하여 대기방에 있는 경우*/
    case Chat_KickOut:
        flag = 1;
        QMessageBox::critical(this, tr("Chatting Client"), \
                              tr("Kick out from Server"));
        inputLine->setDisabled(true);
        sentButton->setDisabled(true);
        fileButton->setDisabled(true);
        name->setReadOnly(true);
        name->setDisabled(true);
        connectButton->setText(tr("Chat in"));
        connectButton->setEnabled(false);
        break;

    /*Invite로 다시 채팅방에 들어온 경우*/
    case Chat_Invite:
        flag = 0;
        QMessageBox::critical(this, tr("Chatting Client"), \
                              tr("Invited from Server"));
        inputLine->setEnabled(true);
        sentButton->setEnabled(true);
        fileButton->setEnabled(true);
        name->setReadOnly(true);        /*메시지 입력 가능*/
        name->setDisabled(false);
        connectButton->setText(tr("Chat Out"));
        connectButton->setEnabled(true);
        break;
    };
}

/*연결이 끊어졌을 때 : 버튼과 lineEdit의 상태 변경*/
void ChatClientForm::disconnect( )
{
    QMessageBox::critical(this, tr("Chatting Client"), \
                          tr("Disconnect from Server"));
    inputLine->setEnabled(false);
    name->setReadOnly(false);
    sentButton->setEnabled(false);
    connectButton->setText(tr("Log in"));
}

/* 프로토콜을 생성해서 서버로 전송 */
void ChatClientForm::sendProtocol(Chat_Status type, char* data, int size)
{
    QByteArray dataArray;               /*소켓으로 보낼 데이터를 채움*/
    QDataStream out(&dataArray, QIODevice::WriteOnly);

    /*스트림의 맨 처음부터 타입과 데이터를 담아 서버로 전송*/
    out.device()->seek(0);
    out << type;
    out.writeRawData(data, size);
    clientSocket->write(dataArray);     /*서버로 전송*/
    clientSocket->flush();              /*데이터를 작성한 소켓을 정리*/
    while(clientSocket->waitForBytesWritten()); /*최대 3초정도 대기*/
}

/* 채팅창에서 메시지 보내기 */
void ChatClientForm::sendData()
{
    /*입력란의 작성한 메시지를 str에 저장*/
    QString str = inputLine->text( );

    /*메시지의 길이가 있으면 채팅창에 메시지 출력*/
    if(str.length( )) {
        QByteArray bytearray;
        bytearray = str.toUtf8( );

        /* 화면에 표시 : 앞에 '나'라고 추가 */
        message->append("<font color=red>나</font> : " + str);
        sendProtocol(Chat_Talk, bytearray.data());
        emit ClientName(name->text());
    }

    /*해당 고객별로 채팅 메시지 별도로 저장*/
    QTreeWidgetItem* item = new QTreeWidgetItem(clientLog);
    item->setText(0, serverAddress->text());            /*IP 번호*/
    item->setText(1, serverPort->text());               /*포트 번호*/
    item->setText(2, name->text());                     /*고객명*/
    item->setText(3, str);                              /*채팅 메시지*/
    item->setText(4, QDateTime::currentDateTime().toString());  /*현재 날짜 및 시간*/
    clientLog->setColumnCount(5);

    clientLogThread->appendData(item);
}

/* 파일 전송시 여러번 나눠서 전송 시작 */
void ChatClientForm::goOnSend(qint64 numBytes)
{
    /*전송되지 않고 남은 파일 크기*/
    byteToWrite -= numBytes;
    outBlock = file->read(qMin(byteToWrite, numBytes));
    fileClient->write(outBlock);

    /*ProgressDialog에 총 파일 크기를 멕시멈으로 설정*/
    progressDialog->setMaximum(totalSize);
    progressDialog->setValue(totalSize-byteToWrite);

    /*남은 파일 크기가 0이면 파일이 모두 전송되었다는 의미*/
    if (byteToWrite == 0) { // Send completed
        qDebug("File sending completed!");
        progressDialog->reset();
    }
}

/* 파일 보내기 : 파일을 열고 파일명(파일경로)을 받아옴 */
void ChatClientForm::sendFile()
{
    loadSize = 0;
    byteToWrite = 0;
    totalSize = 0;
    outBlock.clear();

    /*Send File 누르면 파일 다이얼로그를 띄워서 어떤 파일을 보낼지 선택*/
    QString filename = QFileDialog::getOpenFileName(this);

    /*파일명이 있으면 파일을 오픈*/
    if(filename.length()) {
        file = new QFile(filename);
        file->open(QFile::ReadOnly);

        qDebug() << QString("file %1 is opened").arg(filename);
        progressDialog->setValue(0); /*처음에는 전송하지 않음*/

        /*파일을 한번 보낸 적이 있으면 이미 서버에 연결 되어있는 상태,
          보낸 적이 없으면 서버와 연결시켜줌*/
        if (!isSent) {
            fileClient->connectToHost(serverAddress->text( ),
                                      serverPort->text( ).toInt( ) + 1);
            isSent = true;
        }

        /*처음 보낼 때 connectToHost는 send를 호출하기 위해 연결 신호를 시작하고 두 번째부터는 send를 호출해야 함*/

        /*파일 사이즈만큼 전송*/
        byteToWrite = totalSize = file->size(); // 전송되지 않고 남아있는 데이터의 사이즈
        loadSize = 1024;

        QDataStream out(&outBlock, QIODevice::WriteOnly);

        /*파일명의 길이를 알 수 없으므로 앞에 0으로 초기화 후 일단 넣음*/
        out << qint64(0) << qint64(0) << filename << name->text();

        /*전체 파일 크기는 파일명 + 데이터*/
        totalSize += outBlock.size();
        byteToWrite += outBlock.size();

        /*스트림의 시작으로 돌아가 전체 크기와 파일 이름 및 데이터의 크기인 qint64를 앞에 작성*/
        out.device()->seek(0);

        /*totalSize가 앞에 qint64(0)으로 덮어씌워지고 qint64(outBlock.size())도 qint64(0)으로 덮어씌워짐*/
        out << totalSize << qint64(outBlock.size());

        /*읽어온 파일을 소켓을 통해 서버로 전송*/
        fileClient->write(outBlock);

        /*progressbar에 파일 전송되는 상태 표시*/
        progressDialog->setMaximum(totalSize);
        progressDialog->setValue(totalSize-byteToWrite);
        progressDialog->show();
    }
    qDebug() << QString("Sending file %1").arg(filename);
}

/*클라이언트에서 입력한 고객명이 서버의 ClientList에서의 유무를 담고 있는 슬롯*/
void ChatClientForm::LogInCheckSended(int Check)
{
    LogInCheck = Check;
}
