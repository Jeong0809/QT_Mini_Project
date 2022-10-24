#include "chatclientform.h"

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
    // 연결한 서버 정보 입력을 위한 위젯들
    name = new QLineEdit(this);
    QSettings settings("ChatClient", "Chat Client");
    name->setText(settings.value("ChatClient/ID").toString());


    serverAddress = new QLineEdit(this);
    serverAddress->setText("127.0.0.1");
    //serverAddress->setInputMask("999.999.999.999;_");
    QRegularExpression re("^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    QRegularExpressionValidator validator(re);
    serverAddress->setPlaceholderText("Server IP Address");
    serverAddress->setValidator(&validator);

    serverPort = new QLineEdit(this);
    serverPort->setText(QString::number(PORT_NUMBER));
    serverPort->setInputMask("00000;_");
    serverPort->setPlaceholderText("Server Port No");

    connectButton = new QPushButton(tr("Log In"), this);

    QHBoxLayout *serverLayout = new QHBoxLayout;
    serverLayout->addWidget(name);
    serverLayout->addStretch(1);
    serverLayout->addWidget(serverAddress);
    serverLayout->addWidget(serverPort);
    serverLayout->addWidget(connectButton);

    message = new QTextEdit(this);		// 서버에서 오는 메시지 표시용
    message->setReadOnly(true);

    // 서버로 보낼 메시지를 위한 위젯들
    inputLine = new QLineEdit(this);
    connect(inputLine, SIGNAL(returnPressed( )), SLOT(sendData( )));
    connect(inputLine, SIGNAL(returnPressed( )), inputLine, SLOT(clear( )));
    sentButton = new QPushButton("Send", this);
    connect(sentButton, SIGNAL(clicked( )), SLOT(sendData( )));
    connect(sentButton, SIGNAL(clicked( )), inputLine, SLOT(clear( )));
    inputLine->setEnabled(false);
    sentButton->setEnabled(false);

    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(inputLine);
    inputLayout->addWidget(sentButton);

    fileButton = new QPushButton("File Transfer", this);
    connect(fileButton, SIGNAL(clicked( )), SLOT(sendFile( )));
    fileButton->setDisabled(true);

    // 종료 기능
    QPushButton* quitButton = new QPushButton("Log Out", this);
    connect(quitButton, SIGNAL(clicked( )), this, SLOT(close( )));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(fileButton);
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(quitButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(serverLayout);
    mainLayout->addWidget(message);
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    //채팅을 위한 소켓, 클라이언트 소켓 생성
    clientSocket = new QTcpSocket(this);
    connect(clientSocket, &QAbstractSocket::errorOccurred,
            [=]{ qDebug( ) << clientSocket->errorString( ); });
    connect(clientSocket, SIGNAL(readyRead( )), SLOT(receiveData( )));
    connect(clientSocket, SIGNAL(disconnected( )), SLOT(disconnect( )));

    //파일 전송을 위한 소켓 : 데이터가 많이 이동한다.
    fileClient = new QTcpSocket(this);
    connect(fileClient, SIGNAL(bytesWritten(qint64)), SLOT(goOnSend(qint64)));
//    connect(fileClient, SIGNAL(disconnected( )), SLOT(deletelater( )));

    progressDialog = new QProgressDialog(0);
    progressDialog->setAutoClose(true);
    progressDialog->reset();


    connect(connectButton, &QPushButton::clicked,
            [=]{
        if(connectButton->text() == tr("Log In")) {
            clientSocket->connectToHost(serverAddress->text( ),
                                        serverPort->text( ).toInt( ));
            //데이터를 보내기 전에 서버가 접속되기까지 기다리는 것
            clientSocket->waitForConnected();

            //없는 고객명을 입력받았을 때 경고창을 띄워주는 예외처리
            emit LogInChecked(name->text());
            if(LogInCheck == 0)
            {
                QMessageBox::critical(this, tr("Chatting Client"), \
                                      tr("Customer name does not exist."));
                return;
            }
            sendProtocol(Chat_Login, name->text().toStdString().data());
            connectButton->setText(tr("Chat in"));
            name->setReadOnly(true);

        } else if(connectButton->text() == tr("Chat in"))  {
            sendProtocol(Chat_In, name->text().toStdString().data());
            connectButton->setText(tr("Chat Out"));
            inputLine->setEnabled(true);
            sentButton->setEnabled(true);
            fileButton->setEnabled(true);

        } else if(connectButton->text() == tr("Chat Out"))  {
            sendProtocol(Chat_Out, name->text().toStdString().data());
            connectButton->setText(tr("Chat in"));
            inputLine->setDisabled(true);
            sentButton->setDisabled(true);
            fileButton->setDisabled(true);
        }
    } );

    setWindowTitle(tr("Chat Client"));
}

ChatClientForm::~ChatClientForm( )
{
    clientSocket->close( );
    QSettings settings("ChatClient", "Chat Client");
    settings.setValue("ChatClient/ID", name->text());
}

void ChatClientForm::closeEvent(QCloseEvent*)
{
    sendProtocol(Chat_LogOut, name->text().toStdString().data());
    clientSocket->disconnectFromHost();
    if(clientSocket->state() != QAbstractSocket::UnconnectedState)
        clientSocket->waitForDisconnected();
}

void ChatClientForm::receiveData( )
{
    QTcpSocket *clientSocket = dynamic_cast<QTcpSocket *>(sender( ));
    if (clientSocket->bytesAvailable() > BLOCK_SIZE) return;
    QByteArray bytearray = clientSocket->read(BLOCK_SIZE);

    Chat_Status type;       // 채팅의 목적
    char data[1020];        // 전송되는 메시지/데이터
    memset(data, 0, 1020);  // 데이터를 0으로 초기화, 데이터 사이즈는 1020

    QDataStream in(&bytearray, QIODevice::ReadOnly);
    in.device()->seek(0);
    in >> type ;
    in.readRawData(data, 1020);

    switch(type) {
    case Chat_Talk:
        if(flag == 0){
        message->append(QString(data));
        inputLine->setEnabled(true);
        sentButton->setEnabled(true);
        fileButton->setEnabled(true);
        }

        //kick_out으로 대기방에 있을 경우
        else{
            inputLine->setEnabled(false);
            sentButton->setEnabled(false);
            fileButton->setEnabled(false);
        }
        break;

    case Chat_KickOut:
        flag = 1;
        QMessageBox::critical(this, tr("Chatting Client"), \
                              tr("Kick out from Server"));
        inputLine->setDisabled(true);
        sentButton->setDisabled(true);
        fileButton->setDisabled(true);
        name->setReadOnly(false);
        connectButton->setText(tr("Chat in"));
        connectButton->setEnabled(false);
        break;

    case Chat_Invite:
        flag = 0;
        QMessageBox::critical(this, tr("Chatting Client"), \
                              tr("Invited from Server"));
        inputLine->setEnabled(true);
        sentButton->setEnabled(true);
        fileButton->setEnabled(true);
        name->setReadOnly(true);
        connectButton->setText(tr("Chat Out"));
        connectButton->setEnabled(true);
        break;
    };
}

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
    QByteArray dataArray;           // 소켓으로 보낼 데이터를 채우고
    QDataStream out(&dataArray, QIODevice::WriteOnly);
    out.device()->seek(0);
    out << type;
    out.writeRawData(data, size);
    clientSocket->write(dataArray);     // 서버로 전송
    clientSocket->flush();
    while(clientSocket->waitForBytesWritten()); //최대 3초정도 대기
}

/* 메시지 보내기 */
void ChatClientForm::sendData(  )
{
    QString str = inputLine->text( );
    if(str.length( )) {
        QByteArray bytearray;
        bytearray = str.toUtf8( );
        /* 화면에 표시 : 앞에 '나'라고 추가 */
        message->append("<font color=red>나</font> : " + str);
        sendProtocol(Chat_Talk, bytearray.data());
    }
}

/* 파일 전송시 여러번 나눠서 전송 */
void ChatClientForm::goOnSend(qint64 numBytes) // Start sending file content
{
    byteToWrite -= numBytes; // Remaining data size
    outBlock = file->read(qMin(byteToWrite, numBytes));
    fileClient->write(outBlock);

    progressDialog->setMaximum(totalSize);
    progressDialog->setValue(totalSize-byteToWrite);

    if (byteToWrite == 0) { // Send completed
        qDebug("File sending completed!");
        progressDialog->reset();
    }
}

/* 파일 보내기 */
void ChatClientForm::sendFile() // Open the file and get the file name (including path)
{
    loadSize = 0;
    byteToWrite = 0;
    totalSize = 0;
    outBlock.clear();

    QString filename = QFileDialog::getOpenFileName(this);
    if(filename.length()) {
        file = new QFile(filename);
        file->open(QFile::ReadOnly);

        qDebug() << QString("file %1 is opened").arg(filename);
        progressDialog->setValue(0); // Not sent for the first time

        if (!isSent) { // Only the first time it is sent, it happens when the connection generates the signal connect
            fileClient->connectToHost(serverAddress->text( ),
                                      serverPort->text( ).toInt( ) + 1);
            isSent = true;
        }
        // When sending for the first time, connectToHost initiates the connect signal to call send, and you need to call send after the second time

        byteToWrite = totalSize = file->size(); // The size of the remaining data
        loadSize = 1024; // The size of data sent each time

        QDataStream out(&outBlock, QIODevice::WriteOnly);
        out << qint64(0) << qint64(0) << filename << name->text();

        totalSize += outBlock.size(); // The total size is the file size plus the size of the file name and other information
        byteToWrite += outBlock.size();

        out.device()->seek(0); // Go back to the beginning of the byte stream to write a qint64 in front, which is the total size and file name and other information size
        out << totalSize << qint64(outBlock.size());

        fileClient->write(outBlock); // Send the read file to the socket

        progressDialog->setMaximum(totalSize);
        progressDialog->setValue(totalSize-byteToWrite);
        progressDialog->show();
    }
    qDebug() << QString("Sending file %1").arg(filename);
}

void ChatClientForm::LogInCheckSended(int Check)
{
    LogInCheck = Check;
}
