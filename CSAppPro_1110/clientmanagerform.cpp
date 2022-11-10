#include "clientmanagerform.h"
#include "ui_clientmanagerform.h"

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
    /*Client 테이블 뷰의 컬럼명 공간 조절*/
    ui->tableView->resizeColumnsToContents();

    /*Remove가 트리거되면 removeItem 함수 실행*/
    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));

    menu = new QMenu;
    /*메뉴 생성 후 Remove를 메뉴에 추가*/
    menu->addAction(removeAction);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    /*ClientManager 테이블 뷰에서 마우스 오른쪽 버튼 클릭시 해당 위치에서 Remove 창이 나오도록 연결*/
    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));

    /*검색 시 lineedit에 검색할 단어 입력 후 search 버튼 누르면 검색된 결과가 출력되도록 연결*/
    connect(ui->searchLineEdit, SIGNAL(returnPressed()),
            this, SLOT(on_searchPushButton_clicked()));
}

/*clientlist.db 파일에서 저장되어 있었던 고객 데이터를 불러오는 함수*/
void ClientManagerForm::loadData()
{
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

    for(int i = 0; i < clientModel->rowCount(); i++) {
        int id = clientModel->data(clientModel->index(i, 0)).toInt();
        QString name = clientModel->data(clientModel->index(i, 1)).toString();
        //clientList.insert(id, clientModel->index(i, 0));
        emit clientAdded(id, name);
    }
}

/*clientlist.db 파일에 고객 정보의 데이터를 저장해준다*/
ClientManagerForm::~ClientManagerForm()
{
    delete ui;
    QSqlDatabase db = QSqlDatabase::database("clientConnection");
    if(db.isOpen()) {
        clientModel->submitAll();
        delete clientModel;
        db.commit();
        db.close();
    }
}

/*100번부터 고객 ID가 자동으로 부여될 수 있도록 설정하였다.*/
int ClientManagerForm::makeId( )
{
    if(clientModel->rowCount() == 0) {       /*clientList의 사이즈가 0이면 데이터가 없는 것이므로 ID가 100번부터 시작*/
        return 00001;
    } else {
        auto id = clientModel->data(clientModel->index(clientModel->rowCount()-1, 0)).toInt(); /*clientList의 마지막 키의 ID에서 +1한 ID 번호 자동 부여*/
        return ++id;
    }
}

/*고객 데이터에서 선택한 고객 정보를 삭제하는 함수*/
void ClientManagerForm::removeItem()
{
    QModelIndex idx = ui->tableView->currentIndex();
    int ID = idx.sibling(idx.row(), 0).data().toInt();

    if(idx.isValid()) {
        /*테이블뷰에서 클릭한 정보의 행 번호를 index에 저장*/
        int index = ui->tableView->currentIndex().row();

        client_query->prepare("DELETE FROM Customer WHERE c_id = ?;");
        client_query->addBindValue(ID);
        client_query->exec();
        clientModel->select();

        /*Shopmanagerform에서의 고객 정보 콤보박스에서도 삭제가 적용되기 위해 시그널 전송*/
        emit clientremoved(index);
    }

    /*고객 데이터를 삭제하고 난 이후에 모든 입력란을 clear 해준다.*/
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->phoneNumberLineEdit->clear();
    ui->addressLineEdit->clear();
}

/*테이블 뷰에서 마우스 오른쪽 버튼의 위치에 해당하는 행위를 인식하기 위한 함수*/
void ClientManagerForm::showContextMenu(const QPoint &pos)
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

    /*검색창에 입력된 텍스트를 통해 고객 정보 트리위젯에서 flag 조건을 통해 검색,
          i를 통해 어떤 항목으로 검색할지 결정*/
    QModelIndexList indexes = clientModel->match(clientModel->index(0, i), Qt::EditRole, ui->searchLineEdit->text(), -1, Qt::MatchFlags(flag));

    foreach(auto ix, indexes) {
        int id = clientModel->data(ix.siblingAtColumn(0)).toInt(); //c->id();
        QString name = clientModel->data(ix.siblingAtColumn(1)).toString();
        QString number = clientModel->data(ix.siblingAtColumn(2)).toString();
        QString address = clientModel->data(ix.siblingAtColumn(3)).toString();
        QStringList strings;
        strings << QString::number(id) << name << number << address;
        new QTreeWidgetItem(ui->searchTreeWidget, strings);
        for(int i = 0; i < ui->searchTreeWidget->columnCount(); i++)
            ui->searchTreeWidget->resizeColumnToContents(i);
    }
}

/*Modify 버튼 클릭시 고객 정보가 변경되는 것을 실행하는 함수*/
void ClientManagerForm::on_modifyPushButton_clicked()
{
    /*트리위젯에서 클릭한 고객 정보를 item에 저장*/
    QModelIndex idx = ui->tableView->currentIndex();

    if(idx.isValid()) {

        QString name, number, address;
        int index = ui->tableView->currentIndex().row();
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

        /*shopmanagerform의 고객 정보 콤보박스에 변경된 값 전달하는 시그널*/
        emit clientModified(ID, name, index);
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
    ui->idLineEdit->setText(QString::number(id));

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

    QSqlDatabase db = QSqlDatabase::database("clientConnection");
    if(db.isOpen() && name.length()) {
        client_query->exec(QString("INSERT INTO Customer VALUES(%1, '%2', '%3', '%4')").arg(id).arg(name).arg(number).arg(address));
        clientModel->select();
        emit clientAdded(id, name);
    }

    /*고객 데이터를 추가하고 난 이후에 모든 입력란을 clear 해준다.*/
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->phoneNumberLineEdit->clear();
    ui->addressLineEdit->clear();
}

/*Shopmanagerform에서 고객정보 콤보박스를 클릭 시 하단 트리위젯에
클릭한 고객의 정보들의 보여질 수 있도록 하기위한 슬롯 함수*/
void ClientManagerForm::SearchCustomerInfo(int ID)
{
    auto flag = Qt::MatchCaseSensitive;
    QModelIndexList indexes = clientModel->match(clientModel->index(0, 0), Qt::EditRole, ID, -1, Qt::MatchFlags(flag));

    foreach(auto ix, indexes) {
        QString name = clientModel->data(ix.siblingAtColumn(1)).toString();
        QString number = clientModel->data(ix.siblingAtColumn(2)).toString();
        QString address = clientModel->data(ix.siblingAtColumn(3)).toString();
        QStringList strings;
        strings << name << number << address;

        /*Shopmanagerform에서 받아온 ID를 통해 고객 정보를 찾아 해당 정보를 넘겨줌*/
        emit CustomerInfoSended(strings);
    }
}

/*고객 정보를 담고 있는 테이블 뷰에서 해당 고객을 클릭했을 경우
입력란의 lineedit에 해당 고객에 대한 텍스트가 보여질 수 있도록 구현하였다.*/
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
