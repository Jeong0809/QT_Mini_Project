#include "productmanagerform.h"
#include "ui_productmanagerform.h"
#include "productitem.h"

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

ProductManagerForm::ProductManagerForm(QWidget *parent):QWidget(parent), ui(new Ui::ProductManagerForm)
{
    ui->setupUi(this);

    QList<int> sizes;
    sizes << 560 << 400;
    ui->splitter->setSizes(sizes);

    /*Remove가 트리거되면 removeItem 함수 실행*/
    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));

    menu = new QMenu;
    /*메뉴 생성 후 Remove 추가*/
    menu->addAction(removeAction);
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    /*Product 트리 위젯 컬럼명 공간 조절*/
    ui->treeWidget->setColumnWidth(0, 60);
    ui->treeWidget->setColumnWidth(1, 150);

    /*Productmanager 트리 위젯에서 마우스 오른쪽 버튼 클릭시 해당 위치에서 Remove 창이 나오도록 연결*/
    connect(ui->treeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));

    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu_Table(QPoint)));

    /*검색 시 lineedit에 검색할 단어 입력 후 search 버튼 누르면 검색된 결과가 출력되도록 연결*/
    connect(ui->searchLineEdit, SIGNAL(returnPressed()),
            this, SLOT(on_searchPushButton_clicked()));
}

/*productlist.txt 파일에서 저장되어 있었던 상품 데이터를 불러오는 함수*/
void ProductManagerForm::loadData()
{
    QFile file("productlist.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);

    /*in이 파일의 끝이 아니라면 줄마다 라인별로 읽어온다.*/
    while (!in.atEnd()) {
        QString line = in.readLine();
        QList<QString> row = line.split(", ");      /*ID, 상품명, 상품가격, 상품 카테고리를 ", "로 구분*/
        if(row.size()) {
            int id = row[0].toInt();
            int price = row[2].toInt();
            ProductItem* c = new ProductItem(id, row[1], price, row[3]);
            ui->treeWidget->addTopLevelItem(c);     /*txt파일에 저장되어 있던 상품 정보를 트리위젯에 추가*/
            productList.insert(id, c);              /*productList에도 불러온 내용 추가*/

            /*Shopmanagerform에서 콤보박스를 통해 상품 정보를 가져오기 위해 시그널 전송*/
            emit productAdded(id, row[1]);
        }
    }
    file.close( );

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "productConnection");
    db.setDatabaseName("productlist.db");

    if(db.open()){

        product_query = new QSqlQuery(db);
        product_query->exec("CREATE  TABLE IF NOT EXISTS Product ( p_id NUMBER(20) PRIMARY KEY,"
                            "p_name VARCHAR2(100) NOT NULL,"
                            "p_price NUMBER(20),"
                            "p_category VARCHAR2(100));");

        productModel = new QSqlTableModel(this, db);
        productModel->setTable("Product");
        productModel->select();

        productModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
        productModel->setHeaderData(1, Qt::Horizontal, tr("Product Name"));
        productModel->setHeaderData(2, Qt::Horizontal, tr("Price"));
        productModel->setHeaderData(3, Qt::Horizontal, tr("Category"));

        ui->tableView->setModel(productModel);
    }
}

/*productlist.txt 파일에 상품 정보의 데이터를 ", "로 구분해서 저장*/
ProductManagerForm::~ProductManagerForm()
{
    delete ui;

    QFile file("productlist.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    /*ID, 상품명, 상품가격, 상품 카테고리를 ", "로 구분해서 저장*/
    QTextStream out(&file);

    /*처음 데이터부터 productList 끝까지 ID, 상품명, 상품가격, 상품 카테고리를 콤마로 구분하여 데이터 저장*/
    for (const auto& v : productList) {
        ProductItem* c = v;
        out << c->ID() << ", " << c->getProductName() << ", ";
        out << c->getPrice() << ", ";
        out << c->getCategory() << "\n";
    }
    file.close( );

    QSqlDatabase db = QSqlDatabase::database("clientConnection");
    if(db.isOpen()) {
        productModel->submitAll();
        delete productModel;
        db.commit();
        db.close();
    }
}

bool ProductManagerForm::createConnection( )
{
//    QSqlDatabase db = QSqlDatabase::database();
//    db.setDatabaseName("productlist.db");

//    if(!db.open()) return false;

//    product_query = new QSqlQuery;
//    product_query->exec("CREATE  TABLE IF NOT EXISTS Product ( p_id NUMBER(20) PRIMARY KEY,"
//                        "p_name VARCHAR2(100) NOT NULL,"
//                        "p_price NUMBER(20),"
//                        "p_category VARCHAR2(100));");
    return true;
}

/*10000번부터 상품 ID가 자동으로 부여될 수 있도록 설정하였다.*/
int ProductManagerForm::makeId( )
{
    /*productList의 사이즈가 0이면 데이터가 없는 것이므로 ID가 10000번부터 시작*/
    if(productList.size( ) == 0) {
        return 50000;
    } else {
        /*productList의 마지막 키의 ID에서 +1한 ID 번호 자동 부여*/
        auto id = productList.lastKey();
        return ++id;
    }
}

/*상품 데이터에서 선택한 상품 정보를 삭제하는 함수*/
void ProductManagerForm::removeItem()
{
    /*상품 삭제 같은 경우 삭제하고자 하는 상품을 클릭하고 마우스 오른쪽 버튼을 눌렀을 때
    remove action을 통해 삭제하기 때문에 해당 인덱스를 SIGNAL로 보내준다*/
    QTreeWidgetItem* item = ui->treeWidget->currentItem();

    QModelIndex idx = ui->tableView->currentIndex();
    int ID = idx.sibling(idx.row(), 0).data().toInt();

    if(item != nullptr) {

        /*트리위젯에서 클릭한 정보의 행 번호를 index에 저장*/
        int index = ui->treeWidget->currentIndex().row();

        /*productList에서 현재 클릭한 item의 ID에 해당하는 상품 정보를 삭제*/
        productList.remove(item->text(0).toInt());

        /*트리 위젯에서도 해당 item의 상품 정보 제거*/
        ui->treeWidget->takeTopLevelItem(ui->treeWidget->indexOfTopLevelItem(item));
        delete item;
        ui->treeWidget->update();

        /*Shopmanagerform에서의 상품 정보 콤보박스에서도 삭제가 적용되기 위해 시그널 전송*/
        emit productremoved(index);
    }

    /*상품 데이터를 삭제하고 난 이후에 모든 입력란을 clear 해준다.*/
    ui->idLineEdit->clear();
    ui->productnameLineEdit->clear();
    ui->priceLineEdit->clear();
    ui->categoryLineEdit->clear();

    product_query->prepare("DELETE FROM Product WHERE p_id = ?;");
    product_query->addBindValue(ID);
    product_query->exec();
    productModel->select();
}

/*트리위젯에서 마우스 오른쪽 버튼의 위치에 해당하는 행위를 인식하기 위한 함수*/
void ProductManagerForm::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = ui->treeWidget->mapToGlobal(pos);
    menu->exec(globalPos);
}

void ProductManagerForm::showContextMenu_Table(const QPoint &pos)
{
    QPoint globalPos = ui->tableView->mapToGlobal(pos);
        if(ui->tableView->indexAt(pos).isValid())
            menu->exec(globalPos);
}

/*Search 버튼을 눌렀을 때 검색 기능이 수행되는 함수*/
void ProductManagerForm::on_searchPushButton_clicked()
{
    ui->searchTreeWidget->clear();
    /*검색 콤보박스에서의 인덱스 (ID(0), 상품명(1), 상품가격(2), 카테고리(3))*/
    int i = ui->searchComboBox->currentIndex();

    //MatchCaseSensitive : 대소문자 구별, MatchContains : 검색하는게 항목에 포함되어 있는지 확인
    /*검색 기능에서 상품 ID나 가격을 통해 검색 시 정확한 ID나 정확한 가격을 입력할 시에만
    검색이 되도록 구현*/
    auto flag = (i==0 || i==2) ? Qt::MatchCaseSensitive:
                                 Qt::MatchCaseSensitive | Qt::MatchContains;

    {
        /*검색창에 입력된 텍스트를 통해 상품 정보 트리위젯에서 flag 조건을 통해 검색,
          i를 통해 어떤 항목으로 검색할지 결정*/
        auto items = ui->treeWidget->findItems(ui->searchLineEdit->text(), flag, i);

        foreach(auto i, items) {
            /*i의 자료형을 ProductItem 형으로 변환 후 고정*/
            ProductItem* c = static_cast<ProductItem*>(i);
            int id = c->ID();
            QString productname = c->getProductName();
            QString Category = c->getCategory();
            int price = c->getPrice();
            ProductItem* item = new ProductItem(id, productname, price, Category);

            /*검색을 통해 찾은 item 정보를 search 트리위젯에 추가*/
            ui->searchTreeWidget->addTopLevelItem(item);
        }
    }
}

/*Modify 버튼 클릭시 상품 정보가 변경되는 것을 실행하는 함수*/
void ProductManagerForm::on_modifyPushButton_clicked()
{
    /*트리위젯에서 클릭한 상품 정보를 item에 저장*/
    QTreeWidgetItem* item = ui->treeWidget->currentItem();

    QString productname, Category;
    int price;
    int ID = ui->idLineEdit->text().toInt();
    productname = ui->productnameLineEdit->text();
    price = ui->priceLineEdit->text().toInt();
    Category = ui->categoryLineEdit->text();

    product_query->prepare("UPDATE Product SET p_name = ?, p_price = ?, p_category = ? WHERE p_id = ?;");
    product_query->bindValue(0, productname);
    product_query->bindValue(1, price);
    product_query->bindValue(2, Category);
    product_query->bindValue(3, ID);
    product_query->exec();
    productModel->select();

    if(item != nullptr) {
        /*트리위젯에서 선택한 상품 정보의 행 번호를 저장*/
        int index = ui->treeWidget->currentIndex().row();

        /*트리위젯에서 선택한 상품 정보의 ID 저장*/
        int key = item->text(0).toInt();

        /*ID에 해당하는 상품 정보 객체*/
        ProductItem* c = productList[key];      

        /*사용자가 입력한 값으로 데이터 변경*/
        c->setProductName(productname);
        c->setPrice(price);
        c->setCategory(Category);

        /*변경한 데이터의 객체(정보)를 productList에 저장*/
        productList[key] = c;
        ui->treeWidget->update();

        /*shopmanagerform의 상품 정보 콤보박스에 변경된 값 전달하는 시그널*/
        emit productModified(productname, index);
    }

    /*상품 데이터를 변경하고 난 이후에 모든 입력란을 clear 해준다.*/
    ui->idLineEdit->clear();
    ui->productnameLineEdit->clear();
    ui->priceLineEdit->clear();
    ui->categoryLineEdit->clear();
}

/*Add 버튼 클릭시 상품 정보가 추가되는 것을 실행하는 함수*/
void ProductManagerForm::on_addPushButton_clicked()
{
    QString productname, Category;
    int price;

    /*ID는 자동 생성*/
    int id = makeId();

    /*상품명, 상품가격, 상품 카테고리는 사용자가 입력해줌*/
    productname = ui->productnameLineEdit->text();
    price = ui->priceLineEdit->text().toInt();
    Category = ui->categoryLineEdit->text();

    /*입력란에 데이터를 입력하지 않고 Add버튼 클릭시 경고창 띄워주는 예외처리*/
    if(productname == "" || ui->priceLineEdit->text() == "" || Category == "")
    {
        QMessageBox::warning(this, tr("Error"), tr("정보를 모두 입력해주세요"));
        return;
    }

    /*상품명이 입력되면 해당 정보들을 객체로 productList에 추가*/
    if(productname.length()) {
        ProductItem* c = new ProductItem(id, productname, price, Category);
        productList.insert(id, c);

        /*트리위젯에 해당 정보 추가*/
        ui->treeWidget->addTopLevelItem(c);
        ui->treeWidget->update();

        /*상품 정보가 추가되면 shopmanagerform에서 콤보박스에 해당 상품 정보 추가되는 시그널*/
        emit productAdded(id, productname);
    }

    /*고객 데이터를 추가하고 난 이후에 모든 입력란을 clear 해준다.*/
    ui->idLineEdit->clear();
    ui->productnameLineEdit->clear();
    ui->priceLineEdit->clear();
    ui->categoryLineEdit->clear();

    product_query->exec(QString("INSERT INTO Product VALUES(%1, '%2', %3, '%4')").arg(id).arg(productname).arg(price).arg(Category));
    productModel->select();
}

/*상품 정보를 담고 있는 트리위젯에서 해당 상품을 클릭했을 경우
입력란의 lineedit에 해당 상품에 대한 텍스트가 보여질 수 있도록 구현하였다.*/
void ProductManagerForm::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    ui->idLineEdit->setText(item->text(0));
    ui->productnameLineEdit->setText(item->text(1));
    ui->priceLineEdit->setText(item->text(2));
    ui->categoryLineEdit->setText(item->text(3));

    /*Search toolBox가 선택되어 있을 때 트리위젯의 아이템 클릭시 Input toolBox로 변경된다.*/
    ui->toolBox->setCurrentIndex(0);
}

/*Shopmanagerform에서 상품정보 콤보박스를 클릭 시 하단 트리위젯에
클릭한 상품의 정보들의 보여질 수 있도록 하기위한 시그널 함수*/
void ProductManagerForm::SearchProductInfo(QString productname)
{
    int i = 1;
    auto flag = (i)? Qt::MatchCaseSensitive|Qt::MatchContains
                   : Qt::MatchCaseSensitive;
    {
        /*shopmanagerform에서 받아온 상품명을 통해 해당 상품 정보를 검색*/
        auto items = ui->treeWidget->findItems(productname, flag, i);
        foreach(auto i, items) {
            ProductItem* c = static_cast<ProductItem*>(i);
            int id = c->ID();
            QString name = c->getProductName();
            QString category = c->getCategory();
            int price = c->getPrice();

            /*Shopmanagerform에서 받아온 상품명을 통해 상품 정보를 찾아 해당 객체를 넘겨줌*/
            emit ProductInfoSended(c);
        }
    }

}

void ProductManagerForm::on_tableView_clicked(const QModelIndex &idx)
{
    QString ID = idx.sibling(idx.row(), 0).data().toString();
    QString productname = idx.sibling(idx.row(), 1).data().toString();
    QString price = idx.sibling(idx.row(), 2).data().toString();
    QString Category = idx.sibling(idx.row(), 3).data().toString();

    ui->idLineEdit->setText(ID);
    ui->productnameLineEdit->setText(productname);
    ui->priceLineEdit->setText(price);
    ui->categoryLineEdit->setText(Category);
}

