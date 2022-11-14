#include "shoplistmanagerform.h"
#include "ui_shoplistmanagerform.h"

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

ShoplistManagerForm::ShoplistManagerForm(QWidget *parent) :
    QWidget(parent), ui(new Ui::ShoplistManagerForm)
{
    ui->setupUi(this);

    QList<int> sizes;
    sizes << 790 << 450;
    ui->splitter->setSizes(sizes);

    /*Remove가 트리거되면 removeItem 함수 실행*/
    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));


    /*메뉴 생성 후 Remove 추가*/
    menu = new QMenu;
    menu->addAction(removeAction);

    /*고객 정보 트리 위젯 컬럼명 공간 조절*/
    ui->CustomerInfotreeWidget->setColumnWidth(0, 70);

    /*상품 정보 트리 위젯 컬럼명 공간 조절*/
    ui->ProductInfotreeWidget->setColumnWidth(0, 130);

    /*처음 실행 시 오늘 날짜가 선택되어 있도록 초깃값을 설정*/
    date = new QDate();
    ui->dateEdit->setDate(date->currentDate());

    /*Shoplistmanager 트리 위젯에서 마우스 오른쪽 버튼 클릭시 해당 위치에서 Remove 창이 나오도록 연결*/
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));

    /*검색 시 lineedit에 검색할 단어 입력 후 search 버튼 누르면 검색된 결과가 출력되도록 연결*/
    connect(ui->searchLineEdit, SIGNAL(returnPressed()),
            this, SLOT(on_searchPushButton_clicked()));

    /*검색 모델의 행렬을 0, 8로 초기화*/
    searchModel = new QStandardItemModel(0, 8);

    /*검색 창에서의 헤더 설정*/
    searchModel->setHeaderData(0, Qt::Horizontal, tr("Order\nNumber"));
    searchModel->setHeaderData(1, Qt::Horizontal, tr("Date"));
    searchModel->setHeaderData(2, Qt::Horizontal, tr("Customer"));
    searchModel->setHeaderData(3, Qt::Horizontal, tr("Product"));
    searchModel->setHeaderData(4, Qt::Horizontal, tr("Order\nQuantity"));
    searchModel->setHeaderData(5, Qt::Horizontal, tr("Address"));
    searchModel->setHeaderData(6, Qt::Horizontal, tr("Price"));
    searchModel->setHeaderData(7, Qt::Horizontal, tr("Total Price"));

    /*테이블 뷰에 searchmodel을 통한 모델 지정*/
    ui->searchTableView->setModel(searchModel);
}

/*shoplist.db 파일에서 저장되어 있었던 주문내역 데이터를 불러오는 함수*/
void ShoplistManagerForm::loadData()
{
    /*각각 고객, 상품, 주문 정보 별로 DB 연결을 하기 위해 DB Connection명을 별도로 지정*/
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "shoplistConnection");

    /*Database명 설정*/
    db.setDatabaseName("shoplist.db");

    /*DB가 오픈되면 Shoplist 테이블 생성*/
    if(db.open()){

        /*shoplistConnection에 해당하는 쿼리 생성*/
        shop_query = new QSqlQuery(db);

        /*주문번호, 주문날짜, 고객정보, 상품정보, 주문수량, 주소, 가격, 총 가격 인자를 통한 Shoplist 테이블 생성*/
        shop_query->exec("CREATE  TABLE Shoplist ( s_id NUMBER(20) PRIMARY KEY,"
                         "s_date VARCHAR2(100) NOT NULL,"
                         "s_customer_info VARCHAR2(100),"
                         "s_product_info VARCHAR2(100),"
                         "s_quantity NUMBER(20),"
                         "s_address VARCHAR2(100),"
                         "s_price NUMBER(20),"
                         "s_totalprice NUMBER(20));");

        /*shoplistConnection에 해당하는 테이블 모델 생성*/
        shopModel = new QSqlTableModel(this, db);

        /*테이블 모델에 Shoplist 테이블 설정 및 업데이트*/
        shopModel->setTable("Shoplist");
        shopModel->select();

        /*테이블 뷰에서 헤더 설정 및 뷰에 보여질 모델 설정*/
        shopModel->setHeaderData(0, Qt::Horizontal, tr("Order\nNumber"));
        shopModel->setHeaderData(1, Qt::Horizontal, tr("Date"));
        shopModel->setHeaderData(2, Qt::Horizontal, tr("Customer"));
        shopModel->setHeaderData(3, Qt::Horizontal, tr("Product"));
        shopModel->setHeaderData(4, Qt::Horizontal, tr("Order\nQuantity"));
        shopModel->setHeaderData(5, Qt::Horizontal, tr("Address"));
        shopModel->setHeaderData(6, Qt::Horizontal, tr("Price"));
        shopModel->setHeaderData(7, Qt::Horizontal, tr("Total Price"));
        ui->tableView->setModel(shopModel);
    }

    /*Shoplist 트리 위젯 컬럼명 공간 조절*/
    ui->tableView->setColumnWidth(0, 70);
    ui->tableView->setColumnWidth(1, 80);
    ui->tableView->setColumnWidth(2, 80);
    ui->tableView->setColumnWidth(3, 130);
    ui->tableView->setColumnWidth(4, 70);
    ui->tableView->setColumnWidth(5, 230);
    ui->tableView->setColumnWidth(6, 70);
    ui->tableView->setColumnWidth(7, 70);
}

/*shoplist.db 파일에 주문내역 정보의 데이터를 저장*/
ShoplistManagerForm::~ShoplistManagerForm()
{
    delete ui;
    /*연결했던 DB를 연결해제*/
    QSqlDatabase db = QSqlDatabase::database("shoplistConnection");

    /*DB가 오픈되면 소멸자에서 모델 삭제*/
    if(db.isOpen()) {
        shopModel->submitAll();
        delete shopModel;
        delete searchModel;
        db.commit();
        db.close();
    }
}

/*1번부터 주문번호가 자동으로 부여될 수 있도록 설정*/
int ShoplistManagerForm::makeId( )
{
    /*shopModel의 사이즈가 0이면 데이터가 없는 것이므로 ID가 1번부터 시작*/
    if(shopModel->rowCount() == 0) {
        return 1;
    }
    else {
        /*shopModel의 마지막 데이터의 0번째 인덱스(ID)에서 +1한 ID 번호 자동 부여*/
        auto id = shopModel->data(shopModel->index(shopModel->rowCount()-1, 0)).toInt();
        return ++id;
    }
}

/*주문내역 데이터에서 선택한 주문내역 정보를 삭제하는 함수*/
void ShoplistManagerForm::removeItem()
{
    /*테이블 뷰에서 선택된 주문내역 데이터(주문번호) 반환*/
    QModelIndex idx = ui->tableView->currentIndex();

    /*테이블 뷰에서 클릭한 데이터의 주문 번호 값 반환*/
    int ID = idx.sibling(idx.row(), 0).data().toInt();

    if(idx.isValid()) {
        /*shop model에서 현재 클릭한 ID에 해당하는 주문내역 정보를 삭제*/
        shop_query->prepare("DELETE FROM Shoplist WHERE s_id = ?;");
        shop_query->addBindValue(ID);

        /*해당 쿼리문 실행 및 업데이트*/
        shop_query->exec();
        shopModel->select();
    }

    /*고객 데이터를 삭제하고 난 이후에 모든 입력란을 clear 해줌*/
    ui->idLineEdit->clear();
    ui->QuantityLineEdit->clear();
}

/*트리위젯에서 마우스 오른쪽 버튼의 위치에 해당하는 행위를 인식하기 위한 함수*/
void ShoplistManagerForm::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = ui->tableView->mapToGlobal(pos);
        if(ui->tableView->indexAt(pos).isValid())
            menu->exec(globalPos);
}

/*Search 버튼을 눌렀을 때 데이터 검색 기능이 수행되는 함수*/
void ShoplistManagerForm::on_searchPushButton_clicked()
{
    /*검색 모델 초기화*/
    searchModel->clear();

    /*현재 검색 콤보박스에서 선택된 인덱스, 어떤 인자로 검색할 것인지 나타냄*/
    int i = ui->searchComboBox->currentIndex();

    /*MatchCaseSensitive : 대소문자 구별, MatchContains : 검색하는게 항목에 포함되어 있는지 확인
    검색 기능에서 상품 ID나 수량을 통해 검색 시 정확한 ID나 정확한 수량을 입력할 시에만
    검색이 되도록 구현*/
    auto flag = (i==0 || i==4) ? Qt::MatchCaseSensitive :
                                 Qt::MatchCaseSensitive | Qt::MatchContains;

    /*검색창에 입력된 텍스트를 통해 상품 정보 트리위젯에서 flag 조건을 통해 검색,
      i를 통해 어떤 항목으로 검색할지 결정*/
    QModelIndexList indexes = shopModel->match(shopModel->index(0, i), Qt::EditRole, ui->searchLineEdit->text(), -1, Qt::MatchFlags(flag));

    /*주문 내역에 대한 정보를 shop model에서 불러옴*/
    foreach(auto ix, indexes) {
        /*검색창에서 입력한 값으로 찾은 데이터를 통해 주문 정보 데이터들을 반환*/
        int id = shopModel->data(ix.siblingAtColumn(0)).toInt(); //c->id();
        QString Date = shopModel->data(ix.siblingAtColumn(1)).toString();
        QString CustomerInfo = shopModel->data(ix.siblingAtColumn(2)).toString();
        QString ProductInfo = shopModel->data(ix.siblingAtColumn(3)).toString();
        int Quantity = shopModel->data(ix.siblingAtColumn(4)).toInt();
        QString Address = shopModel->data(ix.siblingAtColumn(5)).toString();
        int Price = shopModel->data(ix.siblingAtColumn(6)).toInt();
        int TotalPrice = shopModel->data(ix.siblingAtColumn(7)).toInt();

        /*찾은 주문 정보 데이터를 stringList에 저장*/
        QStringList strings;
        strings << QString::number(id) << Date << CustomerInfo << ProductInfo
                << QString::number(Quantity) << Address << QString::number(Price)
                << QString::number(TotalPrice);

        /*stringList에 저장한 데이터를 QStandardItem에 추가*/
        QList<QStandardItem *> items;
        for (int i = 0; i < 8; ++i) {
            items.append(new QStandardItem(strings.at(i)));
        }

        /*search Table View에 헤더 추가 및 데이터 추가*/
        searchModel->appendRow(items);
        searchModel->setHeaderData(0, Qt::Horizontal, tr("Order\nNumber"));
        searchModel->setHeaderData(1, Qt::Horizontal, tr("Date"));
        searchModel->setHeaderData(2, Qt::Horizontal, tr("Customer"));
        searchModel->setHeaderData(3, Qt::Horizontal, tr("Product"));
        searchModel->setHeaderData(4, Qt::Horizontal, tr("Order\nQuantity"));
        searchModel->setHeaderData(5, Qt::Horizontal, tr("Address"));
        searchModel->setHeaderData(6, Qt::Horizontal, tr("Price"));
        searchModel->setHeaderData(7, Qt::Horizontal, tr("Total Price"));

        /*추가된 데이터에 맞게 자동적으로 컬럼 공간 조절*/
        ui->searchTableView->resizeColumnsToContents();
    }
}

/*Modify 버튼 클릭시 주문내역 정보가 변경되는 것을 실행하는 함수*/
void ShoplistManagerForm::on_modifyPushButton_clicked()
{
    /*테이블 뷰에서 클릭한 주문내역 인덱스를 idx에 저장*/
    QModelIndex idx = ui->tableView->currentIndex();

    if(idx.isValid()) {
        QString Date, CustomerInfo, ProductInfo, Address;
        int ID, Quantity, TotalPrice, Price;


        /*변경할 주문 내역 데이터 작성*/
        ID = ui->idLineEdit->text().toInt();
        Date = ui->dateEdit->date().toString("yyyy-MM-dd");
        CustomerInfo = ui->CustomerInfocomboBox->currentText().right(7).left(5);
        ProductInfo = ui->ProductInfocomboBox->currentText();
        Quantity = ui->QuantityLineEdit->text().toInt();
        Address = ui->CustomerInfotreeWidget->topLevelItem(0)->text(2);
        Price = ui->ProductInfotreeWidget->topLevelItem(0)->text(1).toInt();
        TotalPrice = Quantity * Price;

        /*사용자가 입력한 값으로 데이터 변경*/
        shop_query->prepare("UPDATE Shoplist SET s_date = ?, s_customer_info = ?,"
                            " s_product_info = ?, s_quantity = ?, s_address = ?,"
                            "s_price = ?, s_totalprice = ? WHERE s_id = ?;");
        shop_query->bindValue(0, Date);
        shop_query->bindValue(1, CustomerInfo);
        shop_query->bindValue(2, ProductInfo);
        shop_query->bindValue(3, Quantity);
        shop_query->bindValue(4, Address);
        shop_query->bindValue(5, Price);
        shop_query->bindValue(6, TotalPrice);
        shop_query->bindValue(7, ID);

        /*해당 쿼리문 실행 및 업데이트*/
        shop_query->exec();
        shopModel->select();
    }

    /*주문 정보 데이터를 변경하고 난 이후에 모든 입력란을 clear*/
    ui->idLineEdit->clear();
    ui->QuantityLineEdit->clear();
}

/*Add 버튼 클릭시 주문내역 정보가 추가되는 것을 실행하는 함수*/
void ShoplistManagerForm::on_addPushButton_clicked()
{
    QString Date, CustomerInfo, ProductInfo, address;
    int Quantity, Price, TotalPrice;

    /*ID는 자동 생성*/
    int ID = makeId();
    ui->idLineEdit->setText(QString::number(ID));

    /*날짜, 고객정보, 상품정보, 수량은 사용자가 입력해줌*/
    Date = ui->dateEdit->date().toString("yyyy-MM-dd");
    CustomerInfo = ui->CustomerInfocomboBox->currentText().right(7).left(5);
    ProductInfo = ui->ProductInfocomboBox->currentText();
    Quantity = ui->QuantityLineEdit->text().toInt();

    /*주문 수량에 따라 재고 수량을 수정하기 위한 시그널*/
    emit QuantitySended(Quantity, ProductInfo);

    /*주문 수량이 재고 수량보다 클 때 경고문 출력*/
    if(orderQuantity == false)
    {
        QMessageBox::warning(this, tr("Error"), tr("재고 수량 초과입니다"));
        return;
    }

    /*입력란에 데이터를 입력하지 않고 Add버튼 클릭시 경고창 띄워주는 예외처리*/
    if(ui->CustomerInfotreeWidget->topLevelItemCount() == 0
            || ui->ProductInfotreeWidget->topLevelItemCount() == 0 || Quantity == 0)
    {
        QMessageBox::warning(this, tr("Error"), tr("정보를 모두 입력해주세요"));
        return;
    }

    /*주소, 가격은 각각 고객 정보, 상품 정보에서 불러오는 데이터*/
    address = ui->CustomerInfotreeWidget->topLevelItem(0)->text(2);
    Price = ui->ProductInfotreeWidget->topLevelItem(0)->text(1).toInt();

    /*총 가격은 불러온 가격 정보와 수량을 곱해서 구해줌*/
    TotalPrice = Price * Quantity;

    /*DB가 오픈되어있고 날짜(NOT NULL)이 입력되면 Shoplist 테이블에 데이터 추가 및 업데이트*/
    QSqlDatabase db = QSqlDatabase::database("shoplistConnection");
    if(db.isOpen() && Date.length()) {
        shop_query->exec(QString("INSERT INTO Shoplist VALUES(%1, '%2', '%3', '%4', %5, '%6', %7, %8)")
                         .arg(ID).arg(Date).arg(CustomerInfo).arg(ProductInfo).arg(Quantity)
                         .arg(address).arg(Price).arg(TotalPrice));
        shopModel->select();
    }
}

/*ProductmanagerForm에서 재고 수량과 주문 수량과의 관계를 저장*/
void ShoplistManagerForm::Informquantity(bool temp)
{
    /*재고 수량 > 주문 수량 -> 1
      재고 수량 < 주문 수량 -> 0 */
    orderQuantity = temp;
}

/*Clientmanagerform에서 고객 정보 추가시 콤보 박스에 추가되는 고객 정보를 처리
고객 정보를 고객 아이디와 함께 불러온 이유는 동명이인일 경우를 구별하기 위함이다.*/
void ShoplistManagerForm::addClient(int CustomerID, QString name)
{
    ui->CustomerInfocomboBox->addItem(name + " ( ID : " + QString::number(CustomerID).rightJustified(5, '0') + " )" );
}

//Clientmanagerform에서 고객 정보 변경시 콤보 박스에서 변경되는 고객 정보를 처리
void ShoplistManagerForm::modifyClient(int CustomerID, QString name, int index)
{    
    ui->CustomerInfocomboBox->setItemText(index, name + " ( ID : " + QString::number(CustomerID) + " )");
}

//Clientmanagerform에서 고객 정보 삭제시 콤보 박스에 삭제되는 고객 정보를 처리
void ShoplistManagerForm::removeClient(int index)
{
    ui->CustomerInfocomboBox->removeItem(index);
}

//Productmanagerform에서 상품 정보 추가시 콤보 박스에 추가되는 상품 정보를 처리
void ShoplistManagerForm::addProduct(int ProductID, QString productname)
{
    Q_UNUSED(ProductID);
    ui->ProductInfocomboBox->addItem(productname);
}

//Productmanagerform에서 상품 정보 변경시 콤보 박스에서 변경되는 상품 정보를 처리
void ShoplistManagerForm::modifyProduct(QString productname, int index)
{
    ui->ProductInfocomboBox->setItemText(index, productname);
}

//Productmanagerform에서 상품 정보 삭제시 콤보 박스에서 삭제되는 상품 정보를 처리
void ShoplistManagerForm::removeProduct(int index)
{
    ui->ProductInfocomboBox->removeItem(index);
}

/*주문내역 정보를 담고 있는 트리위젯에서 해당 주문내역을 클릭했을 경우
입력란의 lineedit에 해당 주문내역에 대한 텍스트가 보여질 수 있도록 구현하였다.*/
void ShoplistManagerForm::on_tableView_clicked(const QModelIndex &idx)
{
    /*클릭한 데이터의 ID, 이름, 휴대폰 번호, 주소를 반환*/
    QString ID = idx.sibling(idx.row(), 0).data().toString();
    QString Date = idx.sibling(idx.row(), 1).data().toString();
    QString CustomerInfo = idx.sibling(idx.row(), 2).data().toString();
    QString ProductInfo = idx.sibling(idx.row(), 3).data().toString();
    QString Quantity = idx.sibling(idx.row(), 4).data().toString();

    /*트리위젯에서 선택한 데이터가 입력란에 보여질 수 있도록 설정*/
    ui->idLineEdit->setText(ID);
    ui->dateEdit->setDate(QDate::fromString(Date, "yyyy-MM-dd"));
    ui->CustomerInfocomboBox->setCurrentIndex(ui->CustomerInfocomboBox->findText(CustomerInfo, Qt::MatchContains));
    ui->ProductInfocomboBox->setCurrentText(ProductInfo);
    ui->QuantityLineEdit->setText(Quantity);

    /*트리위젯에서 데이터를 선택했을 때 고객 ID와 상품명 정보를 통해 오른쪽에 고객정보 트리위젯과
          상품 정보 트리위젯에 그에 해당하는 정보가 보여질 수 있도록 시그널 전송*/
    emit CustomerInfoSearched(CustomerInfo.toInt());
    emit ProductInfoSearched(ProductInfo);

    /*Search toolBox가 선택되어 있을 때 트리위젯의 아이템 클릭시 Input toolBox로 변경된다.*/
    ui->toolBox->setCurrentIndex(0);
}

/*고객 콤보박스에서 "이정연 ( ID : 100 )" 일 경우 오른쪽에서 5자, 왼쪽에서 3자를 빼내어
문자형에서 정수형으로 변환한 후 clientmanagerform으로 보내준 다음 해당 고객 ID의 정보를 불러온다.*/
void ShoplistManagerForm::on_CustomerInfocomboBox_textActivated(const QString &temp)
{
    int ID =  temp.right(7).left(5).toInt();
    emit CustomerInfoSearched(ID);
}

/*Shopmanagerform에서 보낸 고객 ID를 통해 Clientmanagerform에서 해당 고객의 정보를 전체 전송한 후
필요한 정보만 뽑아서 주문내역 트리위젯에 추가해주는 방식이다.*/
void ShoplistManagerForm::SendCustomerInfo(QStringList strings)
{
    ui->CustomerInfotreeWidget->clear();
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, strings[0]);         //이름 정보 추출 후 설정
    item->setText(1, strings[1]);  //휴대폰 번호 정보 추출 후 설정
    item->setText(2, strings[2]);      //주소 정보 추출 후 설정
    ui->CustomerInfotreeWidget->addTopLevelItem(item);  //트리 위젯에 추가
}

/*Shopmanagerform에서 보낸 싱품명을 통해 Productmanagerform에서 해당 상품의 정보를 전체 전송한 후
필요한 정보만 뽑아서 주문내역 트리위젯에 추가해주는 방식이다.*/
void ShoplistManagerForm::SendProductInfo(QStringList strings)
{
    ui->ProductInfotreeWidget->clear();
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, strings[0]);              //상품명 정보 추출 후 설정
    item->setText(1, strings[1]);              //가격 정보 추출 후 설정
    item->setText(2, strings[2]);              //카테고리 정보 추출 후 설정
    ui->ProductInfotreeWidget->addTopLevelItem(item);   //트리 위젯에 추가
}

/*상품 콤보박스의 경우 해당 상품명을 productmanagerform으로 보내준 다음 해당 상품의 정보를 불러온다.*/
void ShoplistManagerForm::on_ProductInfocomboBox_textActivated(const QString &productname)
{
    emit ProductInfoSearched(productname);
}
