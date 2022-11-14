#include "productmanagerform.h"
#include "ui_productmanagerform.h"

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


    /*메뉴 생성 후 Remove Action을 메뉴에 추가*/
    menu = new QMenu;
    menu->addAction(removeAction);

    /*Productmanager 테이블 뷰에서 마우스 오른쪽 버튼 클릭시 해당 위치에서 Remove 창이 나오도록 연결*/
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));

    /*검색 시 lineedit에 검색할 단어 입력 후 search 버튼 누르면 검색된 결과가 출력되도록 연결*/
    connect(ui->searchLineEdit, SIGNAL(returnPressed()),
            this, SLOT(on_searchPushButton_clicked()));

    /*검색 모델의 행렬을 0, 5로 초기화*/
    searchModel = new QStandardItemModel(0, 5);

    /*검색 창에서의 헤더 설정*/
    searchModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
    searchModel->setHeaderData(1, Qt::Horizontal, tr("Product Name"));
    searchModel->setHeaderData(2, Qt::Horizontal, tr("Price"));
    searchModel->setHeaderData(3, Qt::Horizontal, tr("Category"));
    searchModel->setHeaderData(4, Qt::Horizontal, tr("Total\nQuantity"));

    /*테이블 뷰에 searchmodel을 통한 모델 지정*/
    ui->searchTableView->setModel(searchModel);
}

/*productlist.db 파일에서 저장되어 있었던 상품 데이터를 불러오는 함수*/
void ProductManagerForm::loadData()
{
    /*각각 고객, 상품, 주문 정보 별로 DB 연결을 하기 위해 DB Connection명을 별도로 지정*/
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "productConnection");

    /*Database명 설정*/
    db.setDatabaseName("productlist.db");

    /*DB가 오픈되면 Product 테이블 생성*/
    if(db.open()){

        /*productConnection에 해당하는 쿼리 생성*/
        product_query = new QSqlQuery(db);

        /*상품ID, 상품명, 상품가격, 카테고리, 재고 수량인자를 통한 Product 테이블 생성*/
        product_query->exec("CREATE  TABLE IF NOT EXISTS Product ( p_id INTEGER PRIMARY KEY,"
                            "p_name VARCHAR2(100) NOT NULL,"
                            "p_price INTEGER,"
                            "p_category VARCHAR2(100),"
                            "p_quantity INTEGER );");

        /*productConnection에 해당하는 테이블 모델 생성*/
        productModel = new QSqlTableModel(this, db);

        /*테이블 모델에 Product 테이블 설정 및 업데이트*/
        productModel->setTable("Product");
        productModel->select();

        /*테이블 뷰에서 헤더 설정 및 뷰에 보여질 모델 설정*/
        productModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
        productModel->setHeaderData(1, Qt::Horizontal, tr("Product Name"));
        productModel->setHeaderData(2, Qt::Horizontal, tr("Price"));
        productModel->setHeaderData(3, Qt::Horizontal, tr("Category"));
        productModel->setHeaderData(4, Qt::Horizontal, tr("Total\nQuantity"));
        ui->tableView->setModel(productModel);
    }

    /*데이터를 로드할 때 현재 가지고 있는 상품ID, 상품명을 ShoplistmanagerForm의
     * 콤보박스에 띄워주기 위해 시그널 전송*/
    for(int i = 0; i < productModel->rowCount(); i++) {
        int id = productModel->data(productModel->index(i, 0)).toInt();
        QString name = productModel->data(productModel->index(i, 1)).toString();
        emit productAdded(id, name);
    }

    /*Product 테이블 뷰의 컬럼명 공간 조절*/
    ui->tableView->setColumnWidth(0, 70);
    ui->tableView->setColumnWidth(1, 200);
    ui->tableView->setColumnWidth(2, 90);
    ui->tableView->setColumnWidth(3, 100);
    ui->tableView->setColumnWidth(4, 70);
}

/*productlist.db 파일에 상품 정보의 데이터를 저장*/
ProductManagerForm::~ProductManagerForm()
{
    delete ui;
    /*연결했던 DB를 연결해제*/
    QSqlDatabase db = QSqlDatabase::database("productConnection");

    /*DB가 오픈되면 소멸자에서 모델 삭제*/
    if(db.isOpen()) {
        productModel->submitAll();
        delete productModel;
        delete searchModel;
        db.commit();
        db.close();
    }
}

/*50000번부터 상품 ID가 자동으로 부여될 수 있도록 설정*/
int ProductManagerForm::makeId( )
{
    /*productModel의 사이즈가 0이면 데이터가 없는 것이므로 ID가 50000번부터 시작*/
    if(productModel->rowCount() == 0) {
        return 50000;
    } else {
        /*productModel의 마지막 데이터의 0번째 인덱스(ID)에서 +1한 ID 번호 자동 부여*/
        auto id = productModel->data(productModel->index(productModel->rowCount()-1, 0)).toInt(); /*clientList의 마지막 키의 ID에서 +1한 ID 번호 자동 부여*/
        return ++id;
    }
}

/*상품 데이터에서 선택한 상품 정보를 삭제하는 함수*/
void ProductManagerForm::removeItem()
{
    /*테이블 뷰에서 현재 클릭한 데이터의 인덱스 반환*/
    QModelIndex idx = ui->tableView->currentIndex();

    /*테이블 뷰에서 클릭한 데이터의 고객 ID값 반환*/
    int ID = idx.sibling(idx.row(), 0).data().toInt();

    if(idx.isValid()) {
        /*테이블뷰에서 클릭한 정보의 행 번호를 index에 저장*/
        int index = ui->tableView->currentIndex().row();

        /*현재 클릭한 상품ID에 해당하는 상품 데이터 삭제*/
        product_query->prepare("DELETE FROM Product WHERE p_id = ?;");
        product_query->addBindValue(ID);

        /*해당 쿼리문 실행*/
        product_query->exec();

        /*해당 데이터 모델 업데이트*/
        productModel->select();

        /*Shopmanagerform에서의 상품 정보 콤보박스에서도 삭제가 적용되기 위해 시그널 전송*/
        emit productremoved(index);
    }

    /*상품 데이터를 삭제하고 난 이후에 모든 입력란을 clear 해줌*/
    ui->idLineEdit->clear();
    ui->productnameLineEdit->clear();
    ui->priceLineEdit->clear();
    ui->categoryLineEdit->clear();
    ui->quantityLineEdit->clear();

}

/*테이블 뷰에서 마우스 오른쪽 버튼의 위치에 해당하는 행위를 인식하기 위한 함수*/
void ProductManagerForm::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = ui->tableView->mapToGlobal(pos);
        if(ui->tableView->indexAt(pos).isValid())
            menu->exec(globalPos);
}

/*Search 버튼을 눌렀을 때 데이터 검색 기능이 수행되는 함수*/
void ProductManagerForm::on_searchPushButton_clicked()
{
    /*검색 모델 초기화*/
    searchModel->clear();

    /*검색 콤보박스에서의 인덱스 (ID(0), 상품명(1), 상품가격(2), 카테고리(3))*/
    int i = ui->searchComboBox->currentIndex();

    //MatchCaseSensitive : 대소문자 구별, MatchContains : 검색하는게 항목에 포함되어 있는지 확인
    /*검색 기능에서 상품 ID나 가격을 통해 검색 시 정확한 ID나 정확한 가격을 입력할 시에만
    검색이 되도록 구현*/
    auto flag = (i==0 || i==2) ? Qt::MatchCaseSensitive:
                                 Qt::MatchCaseSensitive | Qt::MatchContains;

    /*검색창에 입력된 텍스트를 통해 상품 정보 트리위젯에서 flag 조건을 통해 검색,
          i를 통해 어떤 항목으로 검색할지 결정*/
    QModelIndexList indexes = productModel->match(productModel->index(0, i), Qt::EditRole, ui->searchLineEdit->text(), -1, Qt::MatchFlags(flag));

    foreach(auto ix, indexes) {
        /*검색창에서 입력한 값으로 찾은 데이터를 통해 상품 데이터들을 반환*/
        int id = productModel->data(ix.siblingAtColumn(0)).toInt();
        QString productname = productModel->data(ix.siblingAtColumn(1)).toString();
        int price = productModel->data(ix.siblingAtColumn(2)).toInt();
        QString Category = productModel->data(ix.siblingAtColumn(3)).toString();
        int quantity = productModel->data(ix.siblingAtColumn(4)).toInt();

        /*찾은 고객 데이터를 stringList에 저장*/
        QStringList strings;
        strings << QString::number(id) << productname
                << QString::number(price) << Category << QString::number(quantity);

        /*stringList에 저장한 데이터를 QStandardItem에 추가*/
        QList<QStandardItem *> items;
        for (int i = 0; i < 5; ++i) {
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

/*Modify 버튼 클릭시 상품 정보가 변경되는 것을 실행하는 함수*/
void ProductManagerForm::on_modifyPushButton_clicked()
{
    /*트리위젯에서 클릭한 상품 정보를 item에 저장*/
    QModelIndex idx = ui->tableView->currentIndex();

    if(idx.isValid()) {

        QString productname, Category;
        int price, quantity;

        /*현재 클릭한 고객 데이터의 텍스트 값을 반환*/
        int index = ui->tableView->currentIndex().row();       
        int ID = ui->idLineEdit->text().toInt();
        productname = ui->productnameLineEdit->text();
        price = ui->priceLineEdit->text().toInt();
        Category = ui->categoryLineEdit->text();
        quantity = ui->quantityLineEdit->text().toInt();

        /*클릭한 상품 데이터에 해당하는 데이터 값을 변경*/
        product_query->prepare("UPDATE Product SET p_name = ?, p_price = ?, "
                               "p_category = ?, p_quantity = ? WHERE p_id = ?;");
        product_query->bindValue(0, productname);
        product_query->bindValue(1, price);
        product_query->bindValue(2, Category);
        product_query->bindValue(3, quantity);
        product_query->bindValue(4, ID);

        /*해당 쿼리문 실행 및 업데이트*/
        product_query->exec();
        productModel->select();

        /*shopmanagerform의 상품 정보 콤보박스에 변경된 값 전달하는 시그널*/
        emit productModified(productname, index);
    }

    /*상품 데이터를 변경하고 난 이후에 모든 입력란을 clear 해준다.*/
    ui->idLineEdit->clear();
    ui->productnameLineEdit->clear();
    ui->priceLineEdit->clear();
    ui->categoryLineEdit->clear();
    ui->quantityLineEdit->clear();
}

/*Add 버튼 클릭시 상품 정보가 추가되는 것을 실행하는 함수*/
void ProductManagerForm::on_addPushButton_clicked()
{
    QString productname, Category;
    int price, quantity;

    /*ID는 자동 생성*/
    int id = makeId();
    ui->idLineEdit->setText(QString::number(id));

    /*상품명, 상품가격, 상품 카테고리는 사용자가 입력해줌*/
    productname = ui->productnameLineEdit->text();
    price = ui->priceLineEdit->text().toInt();
    Category = ui->categoryLineEdit->text();
    quantity = ui->quantityLineEdit->text().toInt();

    /*입력란에 데이터를 입력하지 않고 Add버튼 클릭시 경고창 띄워주는 예외처리*/
    if(productname == "" || ui->priceLineEdit->text() == ""
            || Category == "" || ui->quantityLineEdit->text() == "")
    {
        QMessageBox::warning(this, tr("Error"), tr("정보를 모두 입력해주세요"));
        return;
    }

    /*DB가 오픈되어있고 고객명(NOT NULL)이 입력되면 Product 테이블에 데이터 추가 및 업데이트*/
    QSqlDatabase db = QSqlDatabase::database("productConnection");
    if(db.isOpen() && productname.length()) {
        product_query->exec(QString("INSERT INTO Product VALUES(%1, '%2', %3, '%4', %5)").arg(id)
                            .arg(productname).arg(price).arg(Category).arg(quantity));
        productModel->select();

        /*상품 정보가 추가되면 shopmanagerform에서 콤보박스에 해당 상품 정보 추가되는 시그널*/
        emit productAdded(id, productname);
    }

    /*고객 데이터를 추가하고 난 이후에 모든 입력란을 clear 해준다.*/
    ui->idLineEdit->clear();
    ui->productnameLineEdit->clear();
    ui->priceLineEdit->clear();
    ui->categoryLineEdit->clear();
    ui->quantityLineEdit->clear();
}

/*Shopmanagerform에서 상품정보 콤보박스를 클릭 시 하단 트리위젯에
클릭한 상품의 정보들의 보여질 수 있도록 하기위한 시그널 함수*/
void ProductManagerForm::SearchProductInfo(QString productname)
{
    auto flag = Qt::MatchCaseSensitive|Qt::MatchContains;
    /*shopmanagerform에서 받아온 상품명을 통해 해당 상품 정보를 검색*/
    QModelIndexList indexes = productModel->match(productModel->index(0, 1), Qt::EditRole, productname, -1, Qt::MatchFlags(flag));

    foreach(auto ix, indexes) {

        /*클릭한 데이터의 이름, 가격, 카테고리를 반환*/
        QString productname = productModel->data(ix.siblingAtColumn(1)).toString();
        QString price = productModel->data(ix.siblingAtColumn(2)).toString();
        QString Category = productModel->data(ix.siblingAtColumn(3)).toString();

        /*고객 데이터를 StringList에 저장*/
        QStringList strings;
        strings << productname << price << Category;

        /*Shopmanagerform에서 받아온 상품명을 통해 상품 정보를 찾아 해당 정보를 넘겨줌*/
        emit ProductInfoSended(strings);
    }
}

/*재고 수량과 주문 수량의 관계 : 주문 수량에 따라 재고 수량이 변경되도록 구현된 함수*/
void ProductManagerForm::SendQuantity(int quantity, QString productname)
{
    auto flag = Qt::MatchCaseSensitive|Qt::MatchContains;

    /*테이블뷰에서 클릭한 데이터의 상품명을 통해 데이터 모델에서 상품 데이터를 찾음*/
    QModelIndexList indexes = productModel->match(productModel->index(0, 1),
                                                  Qt::EditRole, productname, -1, Qt::MatchFlags(flag));

    foreach(auto ix, indexes) {

        /*데이터의 상품ID, 재고 수량을 반환*/
        int ID = productModel->data(ix.siblingAtColumn(0)).toInt();
        int original_quantity = productModel->data(ix.siblingAtColumn(4)).toInt();

        /*재고수량이 주문수량보다 작으면 0을 ShoplistmanagerForm으로 시그널 전송*/
        if(original_quantity - quantity < 0)
        {
            emit quantityInformed(0);
            return;
        }

        /*해당 상품ID의 재고 수량을 주문 수량을 뺀 만큼 변경*/
        product_query->prepare("UPDATE Product SET p_quantity = ? WHERE p_id = ?;");
        product_query->bindValue(0, original_quantity - quantity);
        product_query->bindValue(1, ID);

        /*해당 쿼리문 실행 후 모델 업데이트*/
        product_query->exec();
        productModel->select();

        /*재고수량이 주문수량보다 작으면 1을 ShoplistmanagerForm으로 시그널 전송*/
        emit quantityInformed(1);
    }
}

/*상품 정보를 담고 있는 테이블 뷰에서 해당 상품을 클릭했을 경우
입력란의 line edit에 해당 상품에 대한 텍스트가 보여질 수 있도록 구현*/
void ProductManagerForm::on_tableView_clicked(const QModelIndex &idx)
{
    /*클릭한 데이터의 ID, 상품명, 가격, 카테고리, 재고 수량을 반환*/
    QString ID = idx.sibling(idx.row(), 0).data().toString();
    QString productname = idx.sibling(idx.row(), 1).data().toString();
    QString price = idx.sibling(idx.row(), 2).data().toString();
    QString Category = idx.sibling(idx.row(), 3).data().toString();
    QString quantity = idx.sibling(idx.row(), 4).data().toString();

    /*테이블 뷰에서 클릭한 데이터를 line Edit에 설정에서 보여줌*/
    ui->idLineEdit->setText(ID);
    ui->productnameLineEdit->setText(productname);
    ui->priceLineEdit->setText(price);
    ui->categoryLineEdit->setText(Category);
    ui->quantityLineEdit->setText(quantity);
}

