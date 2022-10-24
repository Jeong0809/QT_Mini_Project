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
    ui->setupUi(this);
    QList<int> sizes;
    sizes << 120 << 500;
    ui->splitter->setSizes(sizes);

    chatServer = new QTcpServer(this);
    connect(chatServer, SIGNAL(newConnection( )), SLOT(clientConnect( )));
    if (!chatServer->listen(QHostAddress::Any, PORT_NUMBER)) {
        QMessageBox::critical(this, tr("Chatting Server"), \
                              tr("Unable to start the server: %1.") \
                              .arg(chatServer->errorString( )));
        close( );
        return;
    }

    fileServer = new QTcpServer(this);
    connect(fileServer, SIGNAL(newConnection()), SLOT(acceptConnection()));
    if (!fileServer->listen(QHostAddress::Any, PORT_NUMBER+1)) {
        QMessageBox::critical(this, tr("Chatting Server"), \
                              tr("Unable to start the server: %1.") \
                              .arg(fileServer->errorString( )));
        close( );
        return;
    }

    qDebug("Start listening ...");

    QAction* inviteAction = new QAction(tr("&Invite"));
    inviteAction->setObjectName("Invite");
    connect(inviteAction, SIGNAL(triggered()), SLOT(inviteClient()));

    QAction* removeAction = new QAction(tr("&Kick out"));
    connect(removeAction, SIGNAL(triggered()), SLOT(kickOut()));

    menu = new QMenu;
    menu->addAction(inviteAction);
    menu->addAction(removeAction);
    ui->clientTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    progressDialog = new QProgressDialog(0);
    progressDialog->setAutoClose(true);
    progressDialog->reset();

    logThread = new LogThread(this);
    logThread->start();

    connect(ui->savePushButton, SIGNAL(clicked()), logThread, SLOT(saveData()));
    qDebug() << tr("The server is running on port %1.").arg(chatServer->serverPort( ));

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
    QTcpSocket *clientConnection = chatServer->nextPendingConnection( );
    connect(clientConnection, SIGNAL(readyRead( )), SLOT(receiveData( )));
    connect(clientConnection, SIGNAL(disconnected( )), SLOT(removeClient()));
    qDebug("new connection is established...");
}

void ChatServerForm::receiveData( )
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender( ));
    QByteArray bytearray = clientConnection->read(BLOCK_SIZE);

    Chat_Status type;       // 채팅의 목적, 4바이트
    char data[1020];        // 전송되는 메시지/데이터, 1020바이트
    memset(data, 0, 1020);

    QDataStream in(&bytearray, QIODevice::ReadOnly);
    in.device()->seek(0);
    in >> type;
    in.readRawData(data, 1020);

    QString ip = clientConnection->peerAddress().toString();
    quint16 port = clientConnection->peerPort();
    QString name = QString::fromStdString(data);
    qDebug() << ip << " : " << type;

    switch(type) {
    case Chat_Login:

        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchFixedString, 1)) {
            if(item->text(0) != "-") {
                item->setText(0, "-");
                clientList.append(clientConnection);        // QList<QTcpSocket*> clientList;
                clientSocketHash[name] = clientConnection;
            }
        }
        break;

    case Chat_In:
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchFixedString, 1)) {
            if(item->text(0) != "O") {
                item->setText(0, "O");
            }
            clientNameHash[port] = name;
        }
        break;

    case Chat_Talk: {
        foreach(QTcpSocket *sock, clientList) {
            //sock != clientConnection이 채팅방에 있는 사람을 검색
            if(clientNameHash.contains(sock->peerPort()) && sock != clientConnection) {
                QByteArray sendArray;
                sendArray.clear();
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

        QTreeWidgetItem* item = new QTreeWidgetItem(ui->messageTreeWidget);
        item->setText(0, ip);
        item->setText(1, QString::number(port));
        item->setText(2, QString::number(clientIDHash[clientNameHash[port]])); // id
        item->setText(3, clientNameHash[port]);                                  // name
        item->setText(4, QString(data));
        item->setText(5, QDateTime::currentDateTime().toString());
        item->setToolTip(4, QString(data));
        ui->messageTreeWidget->addTopLevelItem(item);

        for(int i = 0; i < ui->messageTreeWidget->columnCount(); i++)
            ui->messageTreeWidget->resizeColumnToContents(i);

        logThread->appendData(item);
    }
        break;

    case Chat_Out:
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchContains, 1)) {
            if(item->text(0) != "-") {
                item->setText(0, "-");
            }
            clientNameHash.remove(port);
        }
        break;
    case Chat_LogOut:
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchContains, 1)) {
            if(item->text(0) != "X") {
                item->setText(0, "X");
                clientList.removeOne(clientConnection);        // QList<QTcpSocket*> clientList;
                clientSocketHash.remove(name);
            }
        }

        break;
    }
}

void ChatServerForm::removeClient()
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender( ));

    if(clientConnection != nullptr){
        QString name = clientNameHash[clientConnection->peerPort()];
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchContains, 1)) {
            item->setText(0, "X");
        }
        clientList.removeOne(clientConnection);
        clientConnection->deleteLater();
    }
}

void ChatServerForm::on_clientTreeWidget_customContextMenuRequested(const QPoint &pos)
{
    if(ui->clientTreeWidget->currentItem()){
        foreach(QAction *action, menu->actions()) {
            if(action->objectName() == "Invite"){
                action->setEnabled(ui->clientTreeWidget->currentItem()->text(0) != "O");
            }
            else
                action->setEnabled(ui->clientTreeWidget->currentItem()->text(0) == "O");
        }
        QPoint globalPos = ui->clientTreeWidget->mapToGlobal(pos);
        menu->exec(globalPos);
    }
    else {
        return;
    }
}

void ChatServerForm::kickOut()
{
    QString name = ui->clientTreeWidget->currentItem()->text(1);
    QTcpSocket* sock = clientSocketHash[name];

    QByteArray sendArray;
    QDataStream out(&sendArray, QIODevice::WriteOnly);
    out << Chat_KickOut;
    out.writeRawData("", 1020);
    sock->write(sendArray);
    ui->clientTreeWidget->currentItem()->setText(0, "-");
}

void ChatServerForm::inviteClient()
{
    if(ui->clientTreeWidget->topLevelItemCount()) {
        QString name = ui->clientTreeWidget->currentItem()->text(1);

        QByteArray sendArray;
        QDataStream out(&sendArray, QIODevice::WriteOnly);
        out << Chat_Invite;
        qDebug() << "out" << name;
        out.writeRawData("", 1020);

        QTcpSocket* sock = clientSocketHash[name];

        //고객이 대기방에 없을 때 Invite 되지 않도록 예외처리
        if (sock == nullptr)
        {
            return;
        }

        sock->write(sendArray);
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchFixedString, 1)) {
            if(item->text(1) != "O") {
                item->setText(0, "O");
                //                clientList.append(sock);        // QList<QTcpSocket*> clientList;
            }
        }
        quint64 port = sock->peerPort();
        clientNameHash[port] = name;
    }
}

//파일 전송
void ChatServerForm::acceptConnection()
{
    qDebug("Connected, preparing to receive files!");

    QTcpSocket* receivedSocket = fileServer->nextPendingConnection();
    connect(receivedSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
}

void ChatServerForm::readClient()
{
    qDebug("Receiving file ...");
    QTcpSocket* receivedSocket = dynamic_cast<QTcpSocket *>(sender( ));
    QString filename;
    QString name;

    if (byteReceived == 0) { // just started to receive data, this data is file information
        progressDialog->reset();
        progressDialog->show();

        QString ip = receivedSocket->peerAddress().toString();
        quint16 port = receivedSocket->peerPort();
        qDebug() << receivedSocket->peerName();

        QDataStream in(receivedSocket);
        in >> totalSize >> byteReceived >> filename >> name;
        progressDialog->setMaximum(totalSize);

        QTreeWidgetItem* item = new QTreeWidgetItem(ui->messageTreeWidget);
        item->setText(0, ip);
        item->setText(1, QString::number(port));
        item->setText(2, QString::number(clientIDHash[name]));
        item->setText(3, name);
        item->setText(4, filename);
        item->setText(5, QDateTime::currentDateTime().toString());
        item->setToolTip(4, filename);

        for(int i = 0; i < ui->messageTreeWidget->columnCount(); i++)
            ui->messageTreeWidget->resizeColumnToContents(i);

        ui->messageTreeWidget->addTopLevelItem(item);
        logThread->appendData(item);

        QFileInfo info(filename);
        QString currentFileName = info.fileName();
        file = new QFile(currentFileName);
        file->open(QFile::WriteOnly);
    } else { // Officially read the file content
        inBlock = receivedSocket->readAll();

        byteReceived += inBlock.size();
        file->write(inBlock);
        file->flush();
    }

    progressDialog->setValue(byteReceived);

    if (byteReceived == totalSize) {
        qDebug() << QString("%1 receive completed").arg(filename);

        inBlock.clear();
        byteReceived = 0;
        totalSize = 0;
        progressDialog->reset();
        progressDialog->hide();
        file->close();
        delete file;
    }
}

void ChatServerForm::CheckLogIn(QString name)
{
    auto flag = Qt::MatchFixedString;
    int Check = 1;

    if(ui->clientTreeWidget->findItems(name, flag, 1).count() == 0)
    {
        // 찾는 고객명의 데이터가 없을 때
        Check = 0;
        emit SendLogInChecked(Check);
    }
        // 찾는 고객명의 데이터가 있을 때
    else
        emit SendLogInChecked(Check);

}

void ChatServerForm::addChatClient(int id, QString name)
{
    clientIDList << id;
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->clientTreeWidget);
    item->setText(0, "X");
    item->setText(1, name);
    ui->clientTreeWidget->addTopLevelItem(item);
    clientIDHash[name] = id;
    ui->clientTreeWidget->resizeColumnToContents(0);
}

void ChatServerForm::removeChatClient(int index)
{
    ui->clientTreeWidget->takeTopLevelItem(index);
}

void ChatServerForm::modifyChatClient(int id, QString name, int index)
{
    ui->clientTreeWidget->topLevelItem(index)->setText(1, name);
    QString last_name = clientIDHash.key(id);
    clientIDHash[name] = id;
    clientIDHash.remove(last_name);
}
