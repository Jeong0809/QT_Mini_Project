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

    /*Remove가 트리거되면 removeItem 함수 실행*/
    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));

    /*메뉴 생성 후 Remove Action을 메뉴에 추가*/
    menu = new QMenu;
    menu->addAction(removeAction);  

    /*ClientManager 테이블 뷰에서 마우스 오른쪽 버튼 클릭시 해당 위치에서 Remove 창이 나오도록 연결*/
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));

    /*검색 시 line edit에 검색할 단어 입력 후 search 버튼 누르면 검색된 결과가 출력되도록 연결*/
    connect(ui->searchLineEdit, SIGNAL(returnPressed()),
            this, SLOT(on_searchPushButton_clicked()));

    /*검색 모델의 행렬을 0, 4로 초기화*/
    searchModel = new QStandardItemModel(0, 4);

    /*검색 창에서의 헤더 설정*/
    searchModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
    searchModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
    searchModel->setHeaderData(2, Qt::Horizontal, tr("Phone Number"));
    searchModel->setHeaderData(3, Qt::Horizontal, tr("Address"));

    /*테이블 뷰에 searchmodel을 통한 모델 지정*/
    ui->searchTableView->setModel(searchModel);
}

/*clientlist.db 파일에서 저장되어 있었던 고객 데이터를 불러오는 함수*/
void ClientManagerForm::loadData()
{
    /*각각 고객, 상품, 주문 정보 별로 DB 연결을 하기 위해 DB Connection명을 별도로 지정*/
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "clientConnection");

    /*Database명 설정*/
    db.setDatabaseName("clientlist.db");

    /*DB가 오픈되면 Customer 테이블 생성*/
    if(db.open()){

        /*clientConnection에 해당하는 쿼리 생성*/
        client_query = new QSqlQuery(db);

        /*고객ID, 고객명, 휴대폰 번호, 주소 인자를 통한 Customer 테이블 생성*/
        client_query->exec("CREATE  TABLE IF NOT EXISTS Customer ( c_id NUMBER(20) PRIMARY KEY,"
                           "c_name VARCHAR2(100) NOT NULL,"
                           "c_phone_number VARCHAR2(100),"
                           "c_address VARCHAR2(100));");

        /*clientConnection에 해당하는 테이블 모델 생성*/
        clientModel = new QSqlTableModel(this, db);

        /*테이블 모델에 Customer 테이블 설정 및 업데이트*/
        clientModel->setTable("Customer");
        clientModel->select();

        /*테이블 뷰에서 헤더 설정 및 뷰에 보여질 모델 설정*/
        clientModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
        clientModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
        clientModel->setHeaderData(2, Qt::Horizontal, tr("Phone Number"));
        clientModel->setHeaderData(3, Qt::Horizontal, tr("Address"));
        ui->tableView->setModel(clientModel);
    }

    /*데이터를 로드할 때 현재 가지고 있는 고객ID, 고객명을 ChatServerForm에 띄워주기 위해 시그널 전송*/
    for(int i = 0; i < clientModel->rowCount(); i++) {
        int id = clientModel->data(clientModel->index(i, 0)).toInt();
        QString name = clientModel->data(clientModel->index(i, 1)).toString();
        emit clientAdded(id, name);
    }

    /*Client 테이블 뷰의 각 컬럼명 공간 조절*/
    ui->tableView->setColumnWidth(0, 70);
    ui->tableView->setColumnWidth(1, 80);
    ui->tableView->setColumnWidth(2, 120);
    ui->tableView->setColumnWidth(3, 300);
}

/*clientlist.db 파일에 고객 정보의 데이터를 저장*/
ClientManagerForm::~ClientManagerForm()
{
    delete ui;
    /*연결했던 DB를 연결해제*/
    QSqlDatabase db = QSqlDatabase::database("clientConnection");

    /*DB가 오픈되면 소멸자에서 모델 삭제*/
    if(db.isOpen()) {
        clientModel->submitAll();
        delete clientModel;
        delete searchModel;
        db.commit();
        db.close();
    }
}

/*1번부터 고객 ID가 자동으로 부여될 수 있도록 설정*/
int ClientManagerForm::makeId( )
{
    /*clientModel의 사이즈가 0이면 데이터가 없는 것이므로 ID가 1번부터 시작*/
    if(clientModel->rowCount() == 0) {
        return 00001;
    }
    else {
        /*clientModel의 마지막 데이터의 0번째 인덱스(ID)에서 +1한 ID 번호 자동 부여*/
        auto id = clientModel->data(clientModel->index(clientModel->rowCount()-1, 0)).toInt();
        return ++id;
    }
}

/*고객 데이터에서 선택한 고객 정보를 삭제하는 함수*/
void ClientManagerForm::removeItem()
{
    /*테이블 뷰에서 현재 클릭한 데이터의 인덱스 반환*/
    QModelIndex idx = ui->tableView->currentIndex();

    /*테이블 뷰에서 클릭한 데이터의 고객 ID값 반환*/
    int ID = idx.sibling(idx.row(), 0).data().toInt();

    if(idx.isValid()) {
        /*테이블뷰에서 클릭한 정보의 행 번호를 index에 저장*/
        int index = ui->tableView->currentIndex().row();

        /*현재 클릭한 고객 ID에 해당하는 고객 데이터 삭제*/
        client_query->prepare("DELETE FROM Customer WHERE c_id = ?;");
        client_query->addBindValue(ID);

        /*해당 쿼리문 실행*/
        client_query->exec();

        /*해당 데이터 모델 업데이트*/
        clientModel->select();

        /*Shopmanagerform에서의 고객 정보 콤보박스에서도 삭제가 적용되기 위해 시그널 전송*/
        emit clientremoved(index);
    }

    /*고객 데이터를 삭제하고 난 이후에 모든 입력란을 clear 해줌*/
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

/*Search 버튼을 눌렀을 때 데이터 검색이 수행되는 함수*/
void ClientManagerForm::on_searchPushButton_clicked()
{
    /*검색 모델 초기화*/
    searchModel->clear();

    /*검색 콤보박스에서의 인덱스 (ID(0), 이름(1), 휴대폰번호(2), 주소(3))*/
    int i = ui->searchComboBox->currentIndex();

    /*MatchCaseSensitive : 대소문자 구별, MatchContains : 검색하는게 항목에 포함되어 있는지 확인*/
    auto flag = (i)? Qt::MatchCaseSensitive|Qt::MatchContains
                   : Qt::MatchCaseSensitive;

    /*검색창에 입력된 텍스트를 통해 고객 정보 트리위젯에서 flag 조건을 통해 검색,
      i를 통해 어떤 항목으로 검색할지 결정*/
    QModelIndexList indexes = clientModel->match(clientModel->index(0, i), Qt::EditRole, ui->searchLineEdit->text(), -1, Qt::MatchFlags(flag));

    foreach(auto ix, indexes) {
        /*검색창에서 입력한 값으로 찾은 데이터를 통해 고객 데이터들을 반환*/
        int id = clientModel->data(ix.siblingAtColumn(0)).toInt();
        QString name = clientModel->data(ix.siblingAtColumn(1)).toString();
        QString number = clientModel->data(ix.siblingAtColumn(2)).toString();
        QString address = clientModel->data(ix.siblingAtColumn(3)).toString();

        /*찾은 고객 데이터를 stringList에 저장*/
        QStringList strings;
        strings << QString::number(id) << name << number << address;

        /*stringList에 저장한 데이터를 QStandardItem에 추가*/
        QList<QStandardItem *> items;
        for (int i = 0; i < 4; ++i) {
            items.append(new QStandardItem(strings.at(i)));
        }

        /*search Table View에 헤더 및 데이터 추가*/
        searchModel->appendRow(items);
        searchModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
        searchModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
        searchModel->setHeaderData(2, Qt::Horizontal, tr("Phone Number"));
        searchModel->setHeaderData(3, Qt::Horizontal, tr("Address"));

        /*추가된 데이터에 맞게 자동적으로 컬럼 공간 조절*/
        ui->searchTableView->resizeColumnsToContents();
    }
}

/*Modify 버튼 클릭시 고객 정보가 변경되는 것을 실행하는 함수*/
void ClientManagerForm::on_modifyPushButton_clicked()
{
    /*테이블 뷰에서 클릭한 주문내역 인덱스를 idx에 저장*/
    QModelIndex idx = ui->tableView->currentIndex();

    if(idx.isValid()) {
        QString name, number, address;

        /*현재 클릭한 고객 데이터의 텍스트 값을 반환*/
        int index = ui->tableView->currentIndex().row();
        int ID = ui->idLineEdit->text().toInt();
        name = ui->nameLineEdit->text();
        number = ui->phoneNumberLineEdit->text();
        address = ui->addressLineEdit->text();

        /*클릭한 고객 데이터에 해당하는 데이터 값을 변경*/
        client_query->prepare("UPDATE Customer SET c_name = ?, c_phone_number = ?, c_address = ? WHERE c_id = ?;");
        client_query->bindValue(0, name);
        client_query->bindValue(1, number);
        client_query->bindValue(2, address);
        client_query->bindValue(3, ID);

        /*해당 쿼리문 실행 및 업데이트*/
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

    /*DB가 오픈되어있고 고객명(NOT NULL)이 입력되면 Customer 테이블에 데이터 추가 및 업데이트*/
    QSqlDatabase db = QSqlDatabase::database("clientConnection");
    if(db.isOpen() && name.length()) {
        client_query->exec(QString("INSERT INTO Customer VALUES(%1, '%2', '%3', '%4')")
                           .arg(id).arg(name).arg(number).arg(address));
        clientModel->select();

        /*데이터가 추가되면 채팅 서버에 고객 리스트 추가하기 위해 시그널 전송*/
        emit clientAdded(id, name);
    }

    /*고객 데이터를 추가하고 난 이후에 모든 입력란을 clear 해줌*/
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->phoneNumberLineEdit->clear();
    ui->addressLineEdit->clear();
}

/*Shopmanagerform에서 고객정보 콤보박스를 클릭 시 하단 테이블뷰에
클릭한 고객의 정보들의 보여질 수 있도록 하기위한 슬롯 함수*/
void ClientManagerForm::SearchCustomerInfo(int ID)
{
    auto flag = Qt::MatchCaseSensitive;

    /*테이블뷰에서 클릭한 데이터의 고객 ID를 통해 데이터 모델에서 고객 데이터를 찾음*/
    QModelIndexList indexes = clientModel->match(clientModel->index(0, 0), Qt::EditRole, ID,
                                                 -1, Qt::MatchFlags(flag));

    foreach(auto ix, indexes) {

        /*클릭한 데이터의 이름, 휴대폰 번호, 주소를 반환*/
        QString name = clientModel->data(ix.siblingAtColumn(1)).toString();
        QString number = clientModel->data(ix.siblingAtColumn(2)).toString();
        QString address = clientModel->data(ix.siblingAtColumn(3)).toString();

        /*고객 데이터를 StringList에 저장*/
        QStringList strings;
        strings << name << number << address;

        /*Shopmanagerform에서 받아온 ID를 통해 고객 정보를 찾아 해당 정보를 넘겨줌*/
        emit CustomerInfoSended(strings);
    }
}

/*고객 정보를 담고 있는 테이블 뷰에서 해당 고객을 클릭했을 경우
입력란의 line edit에 해당 고객에 대한 텍스트가 보여질 수 있도록 구현하였다.*/
void ClientManagerForm::on_tableView_clicked(const QModelIndex &idx)
{
    /*클릭한 데이터의 ID, 이름, 휴대폰 번호, 주소를 반환*/
    QString ID = idx.sibling(idx.row(), 0).data().toString();
    QString name = idx.sibling(idx.row(), 1).data().toString();
    QString phone_number = idx.sibling(idx.row(), 2).data().toString();
    QString address = idx.sibling(idx.row(), 3).data().toString();

    /*테이블 뷰에서 클릭한 데이터를 line Edit에 설정에서 보여줌*/
    ui->idLineEdit->setText(ID);
    ui->nameLineEdit->setText(name);
    ui->phoneNumberLineEdit->setText(phone_number);
    ui->addressLineEdit->setText(address);
}
