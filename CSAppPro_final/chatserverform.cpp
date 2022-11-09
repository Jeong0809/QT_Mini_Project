#include "chatserverform.h"
#include "ui_chatserverform.h"
#include "logthread.h"

#include <QPushButton>
#include <QBoxLayout>
#include <QTcpServer>
#include <QTcpSocket>
#include <QApplication>
#include <QMessageBox>
#include <QScrollBar>
#include <QDateTime>
#include <QDebug>
#include <QMenu>
#include <QFile>
#include <QFileInfo>
#include <QProgressDialog>
#include <QFileDialog>

ChatServerForm::ChatServerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatServerForm), totalSize(0), byteReceived(0)
{
    /*현재 클래스 위에 UI를 구성*/
    ui->setupUi(this);

    QList<int> sizes;
    sizes << 120 << 500;
    ui->splitter->setSizes(sizes);

    /*채팅 서버 생성*/
    chatServer = new QTcpServer(this);
    connect(chatServer, SIGNAL(newConnection( )), SLOT(clientConnect( )));

    /*서버가 포트 번호를 불러올 수 없으면 경고문 출력*/
    if (!chatServer->listen(QHostAddress::Any, PORT_NUMBER)) {
        QMessageBox::critical(this, tr("Chatting Server"), \
                              tr("Unable to start the server: %1.") \
                              .arg(chatServer->errorString( )));
        //        close( );
        return;
    }

    /*파일 전송이 시간이 오래걸리기 때문에 파일서버와 채팅서버 분리 -> 이원화를 위함 (파일 전송하는 동안에도
      채팅이 가능하도록 하기 위함)*/

    /*파일 서버 생성*/
    fileServer = new QTcpServer(this);
    connect(fileServer, SIGNAL(newConnection()), SLOT(acceptConnection()));

    /*서버가 포트 번호를 불러올 수 없으면 경고문 출력*/
    if (!fileServer->listen(QHostAddress::Any, PORT_NUMBER+1)) {
        QMessageBox::critical(this, tr("Chatting Server"), \
                              tr("Unable to start the server: %1.") \
                              .arg(fileServer->errorString( )));
        //        close( );
        return;
    }

    qDebug("Start listening ...");

    /*초대하는 기능을 하는 Action을 추가*/
    QAction* inviteAction = new QAction(tr("&Invite"));
    inviteAction->setObjectName("Invite");

    /*Invite를 클릭하면 고객을 채팅방으로 초대*/
    connect(inviteAction, SIGNAL(triggered()), SLOT(inviteClient()));

    /*강퇴하는 기능을 Action을 추가*/
    QAction* removeAction = new QAction(tr("&Kick out"));

    /*Kick out을 클릭하면 고객을 대기방으로 강퇴*/
    connect(removeAction, SIGNAL(triggered()), SLOT(kickOut()));

    /*메뉴를 생성해서 Invite, Kick out 액션을 추가*/
    menu = new QMenu;
    menu->addAction(inviteAction);
    menu->addAction(removeAction);
    ui->clientTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    /*파일 전송 시 전송되는 퍼센트를 progressbar로 표시*/
    progressDialog = new QProgressDialog(0);

    /*리셋되면 자동으로 닫히는 것*/
    progressDialog->setAutoClose(true);
    progressDialog->reset();

    /*채팅 기록 저장을 위한 로그 스레드*/
    logThread = new LogThread(this);

    /*채팅 로그 기록 시작*/
    logThread->start();

    /*Save 버튼을 누르면 로그 스레드에 채팅 기록 저장*/
    connect(ui->savePushButton, SIGNAL(clicked()), logThread, SLOT(saveData()));
    qDebug() << tr("The server is running on port %1.").arg(chatServer->serverPort( ));

    /*고객의 현재 상태를 coloumn 0번과 2번 2가지로 나타낸 이유
      : 아이콘으로만 나타내주면 서버단에서 Invite, Kick out 메뉴를 마우스를 통해 선택할 때
        선택한 고객의 해당 아이콘 이미지명을 불러올 수 없기 때문
        따라서 2가지로 나타낸 후 텍스트로 나타낸 상태는 숨김 처리*/
    ui->clientTreeWidget->hideColumn(2);
}

ChatServerForm::~ChatServerForm()
{
    delete ui;
    logThread->terminate();
    chatServer->close( );
    fileServer->close( );
}

void ChatServerForm::clientConnect( )
{
    /*다음에 기다리고 있는 커넥션, 서버-클라이언트가 연결이 되면 소켓이 반환*/
    QTcpSocket *clientConnection = chatServer->nextPendingConnection( );
    connect(clientConnection, SIGNAL(readyRead( )), SLOT(receiveData( )));

    /*연결이 끊어졌을 떄 서버단에서 고객의 아이콘 변경을 위한 커넥션*/
    connect(clientConnection, SIGNAL(disconnected( )), SLOT(removeClient()));
    qDebug("new connection is established...");
}

/*데이터를 받을 때*/
void ChatServerForm::receiveData( )
{
    /*서버에 연결된 클라이언트의 소켓, 어떤 클라이언트가 보낸 소켓인지 구분하는 역할*/
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender( ));

    /*데이터를 읽어옴*/
    QByteArray bytearray = clientConnection->read(BLOCK_SIZE);

    Chat_Status type;       /*채팅의 목적, 4바이트*/
    char data[1020];        /*전송되는 메시지/데이터, 1020바이트*/
    memset(data, 0, 1020);  /*데이터를 0으로 초기화, 데이터 사이즈는 1020*/

    /*전달하기 위한 데이터를 소켓에 넣는 과정*/
    QDataStream in(&bytearray, QIODevice::ReadOnly);

    /*맨 앞에서부터 보기 위함*/
    in.device()->seek(0);

    /*타입이 4바이트, 나머지 1020바이트는 data를 넣음*/
    in >> type;
    in.readRawData(data, 1020);

    /*읽어온 데이터를 통해 IP, 포트번호, 이름을 불러옴*/
    QString ip = clientConnection->peerAddress().toString(); //소켓이 연결된 상태에 있으면 연결된 IP의 주소를 반환
    quint16 port = clientConnection->peerPort();             //소켓이 연결된 상태에 있으면 연결된 포트의 주소를 반환
    QString name = QString::fromStdString(data);
    qDebug() << ip << " : " << type;

    /*클라이언트에서 읽어온 타입에 따라 Log in, Chat, Log out 실행*/
    switch(type) {
    case Chat_Login:
        /*client 트리 위젯을 돌며 log in시 입력받은 이름을 검색*/
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchFixedString, 1)) {

            /*해당 고객이 대기중 상태가 아니면 대기중으로 변경*/
            if(item->text(2) != "-") {
                item->setText(2, "-");
                item->setIcon(0, QIcon("waiting.png"));

                /*연결된 커넥션을 clientList에 추가*/
                clientList.append(clientConnection);

                /*고객명에 소켓을 연결 (키: 이름, 값: 클라이언트 소켓), 이름으로 소켓을 검색 가능*/
                clientSocketHash[name] = clientConnection;
            }
        }
        break;

    case Chat_In:
        /*로그인 시 입력받은 이름으로 검색한 후 해당 고객이 채팅중 상태가 아니면 채팅중으로 변경*/
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchFixedString, 1)) {
            if(item->text(2) != "O") {
                item->setIcon(0, QIcon("success.png"));
                item->setText(2, "O");
            }

            /*입력받은 이름을 clientNameHash에 추가*/
            clientNameHash[port] = name;
        }
        break;

    case Chat_Talk: {
        /*나를 제외한 모두에게 메시지를 보내는 것*/
        foreach(QTcpSocket *sock, clientList) {

            /*채팅방에 있는 사람 중 나를 제외한 모든 사람들을 검색*/
            if(clientNameHash.contains(sock->peerPort()) && sock != clientConnection) {

                /*채팅 메시지를 전달할 array 생성 후 clear*/
                QByteArray sendArray;
                sendArray.clear();

                /*나를 제외한 채팅방에 메시지 전송*/
                QDataStream out(&sendArray, QIODevice::WriteOnly);
                out << Chat_Talk;
                sendArray.append("<font color=lightsteelblue>");
                sendArray.append(clientNameHash[port].toStdString().data());
                sendArray.append("</font> : ");
                sendArray.append(name.toStdString().data());
                sock->write(sendArray);
                qDebug() << sock->peerPort();
            }
        }

        /*채팅 로그를 남기는 부분*/
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->messageTreeWidget);
        item->setText(0, ip);                                                   /*IP 번호*/
        item->setText(1, QString::number(port));                                /*포트 번호*/
        item->setText(2, QString::number(clientIDHash[clientNameHash[port]]));  /*고객 ID*/
        item->setText(3, clientNameHash[port]);                                 /*고객 명*/
        item->setText(4, QString(data));                                        /*채팅 메시지*/
        item->setText(5, QDateTime::currentDateTime().toString());              /*채팅 시간 기록*/

        /*마우스를 올리면 메시지 내용이 조그만 창에 뜸*/
        item->setToolTip(4, QString(data));
        ui->messageTreeWidget->addTopLevelItem(item);

        /*컨텐츠의 길이로 QTreeWidget의 헤더의 크기를 고정*/
        for(int i = 0; i < ui->messageTreeWidget->columnCount(); i++)
            ui->messageTreeWidget->resizeColumnToContents(i);

        /*채팅 기록을 logThread에 추가*/
        logThread->appendData(item);
    }
        break;

    case Chat_Out:
        /*채팅방 -> 대기방으로 이동*/
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchContains, 1)) {

            /*입력된 고객명을 찾아 검색한 후 해당 고객이 Chat out 상태가 아니면 변경*/
            if(item->text(2) != "-") {
                item->setText(2, "-");
                item->setIcon(0, QIcon("waiting.png"));
            }

            /*채팅방에서 해당 포트 번호 제거*/
            clientNameHash.remove(port);
        }
        break;


    case Chat_LogOut:
        /*채팅방 -> 대기방으로 이동*/
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchContains, 1)) {

            /*입력된 고객명을 찾아 검색한 후 해당 고객이 log out 상태가 아니면 변경*/
            if(item->text(2) != "X") {
                item->setText(2, "X");
                item->setIcon(0, QIcon("cancel.png"));

                /*log out되면 커넥션 연결 해제, clientSockethash에서 이름 삭제*/
                clientList.removeOne(clientConnection);
                clientSocketHash.remove(name);
            }
        }
        break;
    }
}

/*연결이 끊어졌을 때 해당 고객을 log out 해주기 위한 슬롯*/
void ChatServerForm::removeClient()
{
    /*서버에 연결된 클라이언트의 소켓, 어떤 클라이언트가 보낸 소켓인지 구분하는 역할*/
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender( ));

    if(clientConnection != nullptr){

        /*포트 번호를 이용해서 고객명을 검색*/
        QString name = clientNameHash[clientConnection->peerPort()];

        /*이름을 트리위젯에서 검색해서 고객의 현재 상태 아이콘을 log out으로 변경*/
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchContains, 1)) {
            item->setText(2, "X");
            item->setIcon(0, QIcon("cancel.png"));
        }
        clientList.removeOne(clientConnection);

        /*바로 삭제하기 보다는 다 사용하면 알아서 삭제하도록 설정*/
        clientConnection->deleteLater();
    }
}

/*클라이언트 리스트에서 마우스 오른쪽 버튼 클릭 시 Invite, Kick out 버튼 보임*/
void ChatServerForm::on_clientTreeWidget_customContextMenuRequested(const QPoint &pos)
{
    /*마우스를 클릭한 위치에서 메뉴에 추가했던 Invite, Kick out을 보여줌*/
    if(ui->clientTreeWidget->currentItem()){
        foreach(QAction *action, menu->actions()) {

            /*해당 고객이 대기방에 있는 상태이면 Invite 버튼 활성화*/
            if(action->objectName() == "Invite"){
                action->setEnabled(ui->clientTreeWidget->currentItem()->text(2) == "-");
            }

            /*해당 고객이 채팅방에 있는 상태이면 Kick out 버튼 활성화*/
            else
                action->setEnabled(ui->clientTreeWidget->currentItem()->text(2) == "O");
        }
        /*마우스 클릭시 해당 위치를 읽어주는 부분*/
        QPoint globalPos = ui->clientTreeWidget->mapToGlobal(pos);
        menu->exec(globalPos);
    }

    /*고객 리스트에서 클릭된 고객 없는 상태로 오른쪽 버튼 클릭시 아무 창도 안뜨도록 설정*/
    else {
        return;
    }
}

/*채팅방에 있는 고객을 대기방으로 강퇴시키는 함수*/
void ChatServerForm::kickOut()
{
    /*클릭한 고객의 이름을 저장*/
    QString name = ui->clientTreeWidget->currentItem()->text(1);

    /*이름으로 현재 연결된 소켓을 찾음*/
    QTcpSocket* sock = clientSocketHash[name];

    /*클라이언트로 Kick out 상태를 바이트 array로 전송*/
    QByteArray sendArray;
    QDataStream out(&sendArray, QIODevice::WriteOnly);
    out << Chat_KickOut;
    out.writeRawData("", 1020);
    sock->write(sendArray);

    /*강퇴 당했을 경우 대기방으로 이동되므로 아이콘 변경*/
    ui->clientTreeWidget->currentItem()->setText(2, "-");
    ui->clientTreeWidget->currentItem()->setIcon(0, QIcon("waiting.png"));
}

/*대기방에 있는 고객을 채팅방으로 초대하는 함수*/
void ChatServerForm::inviteClient()
{
    if(ui->clientTreeWidget->topLevelItemCount()) {

        /*클릭한 고객의 이름을 저장*/
        QString name = ui->clientTreeWidget->currentItem()->text(1);

        /*클라이언트로 Invite 상태를 바이트 array로 타입, 이름과 전송*/
        QByteArray sendArray;
        QDataStream out(&sendArray, QIODevice::WriteOnly);
        out << Chat_Invite;
        qDebug() << "out" << name;
        out.writeRawData("", 1020);

        /*고객 이름으로 현재 연결된 소켓을 찾음*/
        QTcpSocket* sock = clientSocketHash[name];

        /*고객이 선택되어 있지 않으면 Invite, Kick out 메뉴가 뜨지 않도록 예외처리*/
        if (sock == nullptr)
        {
            return;
        }
        sock->write(sendArray);

        /*이름을 트리위젯에서 검색해서 고객의 현재 상태 아이콘을 Chat in으로 변경*/
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchFixedString, 1)) {

            /*고객이 채팅방에 들어왔다고 알리는 아이콘 변경*/
            if(item->text(2) != "O") {
                item->setText(2, "O");
                item->setIcon(0, QIcon("success.png"));
            }
        }

        /*소켓의 포트 번호를 key, 이름을 value로 clientNameHash에 저장*/
        quint64 port = sock->peerPort();
        clientNameHash[port] = name;
    }
}

/*파일 전송을 위해 커넥션 연결*/
void ChatServerForm::acceptConnection()
{
    qDebug("Connected, preparing to receive files!");

    /*연결된 QTcpSocket 객체로 다음 보류 연결을 반환*/
    QTcpSocket* receivedSocket = fileServer->nextPendingConnection();
    connect(receivedSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
}

/*파일 전송을 위해 고객 읽어오기*/
void ChatServerForm::readClient()
{
    qDebug("Receiving file ...");

    /*서버에 연결된 클라이언트의 소켓, 어떤 클라이언트가 보낸 소켓인지 구분하는 역할*/
    QTcpSocket* receivedSocket = dynamic_cast<QTcpSocket *>(sender( ));
    QString filename;
    QString name;

    /*파일 전송 시작 :  파일에 대한 정보를 이용해서 QFile 객체 생성,
      처음 보낼 때 파일을 보내기 위한 준비 과정*/
    if (byteReceived == 0) {

        /*파일 전송 진행 상태 보여줌*/
        progressDialog->reset();
        progressDialog->show();

        /*받은 소켓으로 IP, 포트번호를 읽어옴*/
        QString ip = receivedSocket->peerAddress().toString();
        quint16 port = receivedSocket->peerPort();
        qDebug() << receivedSocket->peerName();

        /*읽어온 소켓을 통해 파일 사이즈, 아직 전송되지 않은 파일의 사이즈, 파일명, 고객명*/
        QDataStream in(receivedSocket);
        in >> totalSize >> byteReceived >> filename >> name;

        /*ProgressDialog에는 총 파일 사이즈를 멕시멈으로 설정*/
        progressDialog->setMaximum(totalSize);

        /*채팅 로그를 남기는 부분*/
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->messageTreeWidget);
        item->setText(0, ip);
        item->setText(1, QString::number(port));
        item->setText(2, QString::number(clientIDHash[name]));
        item->setText(3, name);
        item->setText(4, filename);
        item->setText(5, QDateTime::currentDateTime().toString());

        /*마우스 커서를 가져다 댈 때 해당 정보를 작은 창으로 출력*/
        item->setToolTip(4, filename);

        /*컨텐츠의 길이로 QTreeWidget의 헤더의 크기를 고정*/
        for(int i = 0; i < ui->messageTreeWidget->columnCount(); i++)
            ui->messageTreeWidget->resizeColumnToContents(i);

        /*채팅 로그에 로그 기록 후 로그 txt 파일에 저장*/
        ui->messageTreeWidget->addTopLevelItem(item);
        logThread->appendData(item);

        /*경로에서 파일 이름만 가져옴*/
        QFileInfo info(filename);
        QString currentFileName = info.fileName();
        file = new QFile(currentFileName);
        file->open(QFile::WriteOnly);
    }

    /*파일 데이터를 읽어서 저장, 2번째 부터 실제로 파일 전송*/
    else {
        /*파일의 사이즈를 나눠서 전송하고 파일을 비워줌*/
        inBlock = receivedSocket->readAll();
        byteReceived += inBlock.size();
        file->write(inBlock);
        file->flush();
    }

    /*현재까지 전송이 완료된 파일 크기*/
    progressDialog->setValue(byteReceived);

    /*파일 전송이 완료 되었을 때*/
    if (byteReceived == totalSize) {
        qDebug() << QString("%1 receive completed").arg(filename);

        /*블록 초기화*/
        inBlock.clear();
        byteReceived = 0;
        totalSize = 0;

        /*progressbar 초기화, 숨김*/
        progressDialog->reset();
        progressDialog->hide();
        file->close();
        delete file;
    }
}

/*클라이언트에서 이름을 통해 로그인 시 서버에 Client 리스트 내에
존재하는 고객인지 확인하는 슬롯*/
void ChatServerForm::CheckLogIn(QString name)
{
    auto flag = Qt::MatchFixedString;
    int Check = 1;

    if(ui->clientTreeWidget->findItems(name, flag, 1).count() == 0)
    {
        /*찾는 고객명의 데이터가 없을 때 클라이언트로 없는 고객이라는 시그널 전송*/
        Check = 0;
        emit SendLogInChecked(Check);
    }
    /*찾는 고객명의 데이터가 있을 때 클라이언트로 존재하는 고객이라는 시그널 전송*/
    else
        emit SendLogInChecked(Check);

}

/*Clientmanagerform에서 고객 정보 추가시 서버의 ClientList에도 고객 정보 추가하는 슬롯*/
void ChatServerForm::addChatClient(int id, QString name)
{
    /*새로 추가된 고객 정보를 ClientID 리스트에 추가, 트리위젯에 추가,
      ClientIDHash에도 이름과 함께 추가*/
    clientIDList << id;
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->clientTreeWidget);
    item->setText(2, "X");
    item->setIcon(0, QIcon("cancel.png"));
    item->setText(1, name);
    ui->clientTreeWidget->addTopLevelItem(item);
    clientIDHash[name] = id;
    ui->clientTreeWidget->resizeColumnToContents(0);
}

/*Clientmanagerform에서 고객 정보 삭제시 서버의 ClientList에도 고객 정보 삭제하는 슬롯*/
void ChatServerForm::removeChatClient(int index)
{
    ui->clientTreeWidget->takeTopLevelItem(index);
}

/*Clientmanagerform에서 고객 정보 변경시 서버의 ClientList에도 고객 정보 변경하는 슬롯*/
void ChatServerForm::modifyChatClient(int id, QString name, int index)
{
    /*바뀐 이전 이름은 삭제해주고 Clientmanagerform에서 받아온 새 이름으로 변경*/
    ui->clientTreeWidget->topLevelItem(index)->setText(1, name);
    QString last_name = clientIDHash.key(id);
    clientIDHash[name] = id;
    clientIDHash.remove(last_name);
}
