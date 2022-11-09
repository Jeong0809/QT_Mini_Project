#include "clientmanagerform.h"
#include "ui_clientmanagerform.h"
#include "clientitem.h"

#include <QFile>
#include <QMenu>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QTableView>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QSqlTableModel>

ClientManagerForm::ClientManagerForm(QWidget *parent)
    : QWidget(parent), ui(new Ui::ClientManagerForm)
{
    ui->setupUi(this);

    QList<int> sizes;
    sizes << 560 << 400;
    ui->splitter->setSizes(sizes);

    /*Remove가 트리거되면 removeItem 함수 실행*/
    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));

    menu = new QMenu;
    /*메뉴 생성 후 Remove를 메뉴에 추가*/
    menu->addAction(removeAction);
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    /*Client 트리 위젯 컬럼명 공간 조절*/
    ui->treeWidget->setColumnWidth(0, 60);
    ui->treeWidget->setColumnWidth(3, 180);

    /*Clientmanager 트리 위젯에서 마우스 오른쪽 버튼 클릭시 해당 위치에서 Remove 창이 나오도록 연결*/
    connect(ui->treeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));

    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu_Table(QPoint)));

    /*검색 시 lineedit에 검색할 단어 입력 후 search 버튼 누르면 검색된 결과가 출력되도록 연결*/
    connect(ui->searchLineEdit, SIGNAL(returnPressed()),
            this, SLOT(on_searchPushButton_clicked()));
}

/*clientlist.txt 파일에서 저장되어 있었던 고객 데이터를 불러오는 함수*/
void ClientManagerForm::loadData()
{
    QFile file("clientlist.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);

    /*in이 파일의 끝이 아니라면 줄마다 라인별로 읽어온다.*/
    while (!in.atEnd()) {
        QString line = in.readLine();
        QList<QString> row = line.split(", ");      /*ID, 이름, 휴대폰 번호, 주소를 ", "로 구분*/
        if(row.size()) {
            int id = row[0].toInt();
            ClientItem* c = new ClientItem(id, row[1], row[2], row[3]);
            ui->treeWidget->addTopLevelItem(c);     /*txt파일에 저장되어 있던 고객 정보를 트리위젯에 불러옴*/
            clientList.insert(id, c);               /*clientList에도 해당 내용 추가*/

            /*Shopmanagerform에서 콤보박스를 통해 고객 정보를 가져오기 위해 시그널 전송*/
            emit clientAdded(id, row[1]);
        }
    }
    file.close( );

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "clientConnection");
    db.setDatabaseName("clientlist.db");

    if(db.open()){

        client_query = new QSqlQuery(db);
        client_query->exec("CREATE  TABLE IF NOT EXISTS Customer ( c_id NUMBER(20) PRIMARY KEY,"
                           "c_name VARCHAR2(100) NOT NULL,"
                           "c_phone_number VARCHAR2(100),"
                           "c_address VARCHAR2(100));");

        clientModel = new QSqlTableModel(this, db);
        clientModel->setTable("Customer");
        clientModel->select();

        clientModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
        clientModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
        clientModel->setHeaderData(2, Qt::Horizontal, tr("Phone Number"));
        clientModel->setHeaderData(3, Qt::Horizontal, tr("Address"));

        ui->tableView->setModel(clientModel);
    }
}

/*clientlist.txt 파일에 고객 정보의 데이터를 ", "로 구분해서 저장해준다*/
ClientManagerForm::~ClientManagerForm()
{
    delete ui;

    QFile file("clientlist.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    //ID, 고객명, 휴대폰번호, 주소를 ,로 구분해서 저장한다.
    QTextStream out(&file);

    /*처음 데이터부터 clientList 끝까지 ID, 이름, 휴대폰번호, 주소를 콤마로 구분하여 데이터 저장*/
    for (const auto& v : clientList) {
        ClientItem* c = v;
        out << c->ID() << ", " << c->getName() << ", ";
        out << c->getPhoneNumber() << ", ";
        out << c->getAddress() << "\n";
    }
    file.close( );

    QSqlDatabase db = QSqlDatabase::database("clientConnection");
    if(db.isOpen()) {
        clientModel->submitAll();
        delete clientModel;
        db.commit();
        db.close();
    }
}

bool ClientManagerForm::createConnection()
{
    return true;
}


/*100번부터 고객 ID가 자동으로 부여될 수 있도록 설정하였다.*/
int ClientManagerForm::makeId( )
{
    if(clientList.size( ) == 0) {       /*clientList의 사이즈가 0이면 데이터가 없는 것이므로 ID가 100번부터 시작*/
        return 00001;
    } else {
        auto id = clientList.lastKey(); /*clientList의 마지막 키의 ID에서 +1한 ID 번호 자동 부여*/
        return ++id;
    }
}

/*고객 데이터에서 선택한 고객 정보를 삭제하는 함수*/
void ClientManagerForm::removeItem()
{
    /*고객 삭제 같은 경우 삭제하고자 하는 고객을 클릭하고 마우스 오른쪽 버튼을 눌렀을 때
    remove action을 통해 삭제하기 때문에 해당 인덱스를 SIGNAL로 보내준다*/
    QTreeWidgetItem* item = ui->treeWidget->currentItem();

    QModelIndex idx = ui->tableView->currentIndex();
    int ID = idx.sibling(idx.row(), 0).data().toInt();


    if(item != nullptr) {
        int index = ui->treeWidget->currentIndex().row();

        /*clientList에서 현재 클릭한 item의 ID에 해당하는 고객 정보를 삭제*/
        clientList.remove(item->text(0).toInt());

        /*트리 위젯에서도 해당 item의 고객 정보 제거*/
        ui->treeWidget->takeTopLevelItem(ui->treeWidget->indexOfTopLevelItem(item));
        delete item;
        ui->treeWidget->update();

        /*Shopmanagerform에서의 고객 정보 콤보박스에서도 삭제가 적용되기 위해 시그널 전송*/
        emit clientremoved(index);
    }

    /*고객 데이터를 삭제하고 난 이후에 모든 입력란을 clear 해준다.*/
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->phoneNumberLineEdit->clear();
    ui->addressLineEdit->clear();

    client_query->prepare("DELETE FROM Customer WHERE c_id = ?;");
    client_query->addBindValue(ID);
    client_query->exec();
    clientModel->select();
}

/*트리위젯에서 마우스 오른쪽 버튼의 위치에 해당하는 행위를 인식하기 위한 함수*/
void ClientManagerForm::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = ui->treeWidget->mapToGlobal(pos);
    menu->exec(globalPos);
}

void ClientManagerForm::showContextMenu_Table(const QPoint &pos)
{
    QPoint globalPos = ui->tableView->mapToGlobal(pos);
        if(ui->tableView->indexAt(pos).isValid())
            menu->exec(globalPos);
}

/*Search 버튼을 눌렀을 때 수행되는 함수*/
void ClientManagerForm::on_searchPushButton_clicked()
{
    ui->searchTreeWidget->clear();
    /*검색 콤보박스에서의 인덱스 (ID(0), 이름(1), 휴대폰번호(2), 주소(3))*/
    int i = ui->searchComboBox->currentIndex();

    /*MatchCaseSensitive : 대소문자 구별, MatchContains : 검색하는게 항목에 포함되어 있는지 확인*/
    auto flag = (i)? Qt::MatchCaseSensitive|Qt::MatchContains
                   : Qt::MatchCaseSensitive;
    {
        /*검색창에 입력된 텍스트를 통해 고객 정보 트리위젯에서 flag 조건을 통해 검색,
          i를 통해 어떤 항목으로 검색할지 결정*/
        auto items = ui->treeWidget->findItems(ui->searchLineEdit->text(), flag, i);

        foreach(auto i, items) {
            //i의 자료형을 ClientItem 형으로 변환 후 고정
            ClientItem* c = static_cast<ClientItem*>(i);
            int id = c->ID();
            QString name = c->getName();
            QString number = c->getPhoneNumber();
            QString address = c->getAddress();
            ClientItem* item = new ClientItem(id, name, number, address);

            /*검색을 통해 찾은 item 정보를 search 트리위젯에 나타냄*/
            ui->searchTreeWidget->addTopLevelItem(item);
        }
    }
}

/*Modify 버튼 클릭시 고객 정보가 변경되는 것을 실행하는 함수*/
void ClientManagerForm::on_modifyPushButton_clicked()
{
    /*트리위젯에서 클릭한 고객 정보를 item에 저장*/
    QTreeWidgetItem* item = ui->treeWidget->currentItem();   

    QString name, number, address;
    int ID = ui->idLineEdit->text().toInt();
    name = ui->nameLineEdit->text();
    number = ui->phoneNumberLineEdit->text();
    address = ui->addressLineEdit->text();

    client_query->prepare("UPDATE Customer SET c_name = ?, c_phone_number = ?, c_address = ? WHERE c_id = ?;");
    client_query->bindValue(0, name);
    client_query->bindValue(1, number);
    client_query->bindValue(2, address);
    client_query->bindValue(3, ID);
    client_query->exec();
    clientModel->select();

    if(item != nullptr) {
        /*트리위젯에서 선택한 고객 정보의 행을 저장*/
        int index = ui->treeWidget->currentIndex().row();

        /*트리위젯에서 선택한 고객 정보의 ID 저장*/
        int key = item->text(0).toInt();

        /*ID에 해당하는 고객 정보 객체*/
        ClientItem* c = clientList[key];

        /*사용자가 입력한 값으로 데이터 변경*/
        c->setName(name);
        c->setPhoneNumber(number);
        c->setAddress(address);

        /*변경한 데이터의 객체를 clientList에 저장*/
        clientList[key] = c;
        ui->treeWidget->update();

        /*shopmanagerform의 고객 정보 콤보박스에 변경된 값 전달하는 시그널*/
        emit clientModified(key, name, index);
    }

    /*고객 데이터를 변경하고 난 이후에 모든 입력란을 clear 해준다.*/
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->phoneNumberLineEdit->clear();
    ui->addressLineEdit->clear();
}

/*Add 버튼 클릭시 고객 정보가 추가되는 것을 실행하는 함수*/
void ClientManagerForm::on_addPushButton_clicked()
{
    QString name, number, address;

    /*ID는 자동 생성*/
    int id = makeId();

    /*이름, 주소, 휴대폰 번호는 입력해줌*/
    name = ui->nameLineEdit->text();
    number = ui->phoneNumberLineEdit->text();
    address = ui->addressLineEdit->text();

    /*입력란에 데이터를 입력하지 않고 Add버튼 클릭시 경고창 띄워주는 예외처리*/
    if(name == "" || number == "" || address == "")
    {
        QMessageBox::warning(this, tr("Error"), tr("정보를 모두 입력해주세요"));
        return;
    }

    /*이름이 입력되면 해당 정보들을 객체로 clientList에 추가*/
    if(name.length()) {
        ClientItem* c = new ClientItem(id, name, number, address);
        clientList.insert(id, c);

        /*트리위젯에 해당 정보 추가*/
        ui->treeWidget->addTopLevelItem(c);
        ui->treeWidget->update();

        /*고객 정보가 추가되면 shopmanagerform에서 콤보박스에 해당 고객 정보 추가되는 시그널*/
        emit clientAdded(id, name);
    }

    /*고객 데이터를 추가하고 난 이후에 모든 입력란을 clear 해준다.*/
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->phoneNumberLineEdit->clear();
    ui->addressLineEdit->clear();

    client_query->exec(QString("INSERT INTO Customer VALUES(%1, '%2', '%3', '%4')").arg(id).arg(name).arg(number).arg(address));
    clientModel->select();
}

/*고객 정보를 담고 있는 트리위젯에서 해당 고객을 클릭했을 경우
입력란의 lineedit에 해당 고객에 대한 텍스트가 보여질 수 있도록 구현하였다.*/
void ClientManagerForm::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    ui->idLineEdit->setText(item->text(0));
    ui->nameLineEdit->setText(item->text(1));
    ui->phoneNumberLineEdit->setText(item->text(2));
    ui->addressLineEdit->setText(item->text(3));

    /*Search toolBox가 선택되어 있을 때 트리위젯의 아이템 클릭시 Input toolBox로 변경된다.*/
    ui->toolBox->setCurrentIndex(0);
}

/*Shopmanagerform에서 고객정보 콤보박스를 클릭 시 하단 트리위젯에
클릭한 고객의 정보들의 보여질 수 있도록 하기위한 슬롯 함수*/
void ClientManagerForm::SearchCustomerInfo(int ID)
{
    ClientItem* c = clientList[ID];

    /*Shopmanagerform에서 받아온 ID를 통해 고객 정보를 찾아 해당 객체를 넘겨줌*/
    emit CustomerInfoSended(c);
}


void ClientManagerForm::on_tableView_clicked(const QModelIndex &idx)
{
    QString ID = idx.sibling(idx.row(), 0).data().toString();
    QString name = idx.sibling(idx.row(), 1).data().toString();
    QString phone_number = idx.sibling(idx.row(), 2).data().toString();
    QString address = idx.sibling(idx.row(), 3).data().toString();

    ui->idLineEdit->setText(ID);
    ui->nameLineEdit->setText(name);
    ui->phoneNumberLineEdit->setText(phone_number);
    ui->addressLineEdit->setText(address);
}

