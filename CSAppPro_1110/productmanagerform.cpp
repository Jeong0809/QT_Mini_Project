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
    /*Product 테이블 뷰의 컬럼명 공간 조절*/
    ui->tableView->resizeColumnsToContents();

    /*Remove가 트리거되면 removeItem 함수 실행*/
    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));

    menu = new QMenu;
    /*메뉴 생성 후 Remove 추가*/
    menu->addAction(removeAction);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    /*Productmanager 테이블 뷰에서 마우스 오른쪽 버튼 클릭시 해당 위치에서 Remove 창이 나오도록 연결*/
    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));

    /*검색 시 lineedit에 검색할 단어 입력 후 search 버튼 누르면 검색된 결과가 출력되도록 연결*/
    connect(ui->searchLineEdit, SIGNAL(returnPressed()),
            this, SLOT(on_searchPushButton_clicked()));
}

/*productlist.db 파일에서 저장되어 있었던 상품 데이터를 불러오는 함수*/
void ProductManagerForm::loadData()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "productConnection");
    db.setDatabaseName("productlist.db");

    if(db.open()){

        product_query = new QSqlQuery(db);
        product_query->exec("CREATE  TABLE IF NOT EXISTS Product ( p_id INTEGER PRIMARY KEY,"
                            "p_name VARCHAR2(100) NOT NULL,"
                            "p_price INTEGER,"
                            "p_category VARCHAR2(100),"
                            "p_quantity INTEGER );");

        productModel = new QSqlTableModel(this, db);
        productModel->setTable("Product");
        productModel->select();

        productModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
        productModel->setHeaderData(1, Qt::Horizontal, tr("Product Name"));
        productModel->setHeaderData(2, Qt::Horizontal, tr("Price"));
        productModel->setHeaderData(3, Qt::Horizontal, tr("Category"));
        productModel->setHeaderData(4, Qt::Horizontal, tr("Quantity"));

        ui->tableView->setModel(productModel);
    }

    for(int i = 0; i < productModel->rowCount(); i++) {
        int id = productModel->data(productModel->index(i, 0)).toInt();
        QString name = productModel->data(productModel->index(i, 1)).toString();
        //clientList.insert(id, clientModel->index(i, 0));
        emit productAdded(id, name);
    }
}

/*productlist.db 파일에 상품 정보의 데이터를 저장*/
ProductManagerForm::~ProductManagerForm()
{
    delete ui;
    QSqlDatabase db = QSqlDatabase::database("clientConnection");
    if(db.isOpen()) {
        productModel->submitAll();
        delete productModel;
        db.commit();
        db.close();
    }
}

/*10000번부터 상품 ID가 자동으로 부여될 수 있도록 설정하였다.*/
int ProductManagerForm::makeId( )
{
    /*productList의 사이즈가 0이면 데이터가 없는 것이므로 ID가 10000번부터 시작*/
    if(productModel->rowCount() == 0) {
        return 50000;
    } else {
        /*productList의 마지막 키의 ID에서 +1한 ID 번호 자동 부여*/
        auto id = productModel->data(productModel->index(productModel->rowCount()-1, 0)).toInt(); /*clientList의 마지막 키의 ID에서 +1한 ID 번호 자동 부여*/
        return ++id;
    }
}

/*상품 데이터에서 선택한 상품 정보를 삭제하는 함수*/
void ProductManagerForm::removeItem()
{
    /*상품 삭제 같은 경우 삭제하고자 하는 상품을 클릭하고 마우스 오른쪽 버튼을 눌렀을 때
    remove action을 통해 삭제하기 때문에 해당 인덱스를 SIGNAL로 보내준다*/


    QModelIndex idx = ui->tableView->currentIndex();
    int ID = idx.sibling(idx.row(), 0).data().toInt();

    if(idx.isValid()) {

        /*테이블뷰에서 클릭한 정보의 행 번호를 index에 저장*/
        int index = ui->tableView->currentIndex().row();
        product_query->prepare("DELETE FROM Product WHERE p_id = ?;");
        product_query->addBindValue(ID);
        product_query->exec();
        productModel->select();

        /*Shopmanagerform에서의 상품 정보 콤보박스에서도 삭제가 적용되기 위해 시그널 전송*/
        emit productremoved(index);
    }

    /*상품 데이터를 삭제하고 난 이후에 모든 입력란을 clear 해준다.*/
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

    /*검색창에 입력된 텍스트를 통해 상품 정보 트리위젯에서 flag 조건을 통해 검색,
          i를 통해 어떤 항목으로 검색할지 결정*/
    QModelIndexList indexes = productModel->match(productModel->index(0, i), Qt::EditRole, ui->searchLineEdit->text(), -1, Qt::MatchFlags(flag));

    foreach(auto ix, indexes) {
        int id = productModel->data(ix.siblingAtColumn(0)).toInt(); //c->id();
        QString productname = productModel->data(ix.siblingAtColumn(1)).toString();
        int price = productModel->data(ix.siblingAtColumn(2)).toInt();
        QString Category = productModel->data(ix.siblingAtColumn(3)).toString();
        int quantity = productModel->data(ix.siblingAtColumn(4)).toInt();
        QStringList strings;
        strings << QString::number(id) << productname
                << QString::number(price) << Category << QString::number(quantity);
        new QTreeWidgetItem(ui->searchTreeWidget, strings);
        for(int i = 0; i < ui->searchTreeWidget->columnCount(); i++)
            ui->searchTreeWidget->resizeColumnToContents(i);
    }
}

/*Modify 버튼 클릭시 상품 정보가 변경되는 것을 실행하는 함수*/
void ProductManagerForm::on_modifyPushButton_clicked()
{
    /*트리위젯에서 클릭한 상품 정보를 item에 저장*/
    QModelIndex idx = ui->tableView->currentIndex();

    if(idx.isValid()) {

        QString productname, Category;
        int index = ui->tableView->currentIndex().row();
        int price, quantity;
        int ID = ui->idLineEdit->text().toInt();
        productname = ui->productnameLineEdit->text();
        price = ui->priceLineEdit->text().toInt();
        Category = ui->categoryLineEdit->text();
        quantity = ui->quantityLineEdit->text().toInt();

        product_query->prepare("UPDATE Product SET p_name = ?, p_price = ?, "
                               "p_category = ?, p_quantity = ? WHERE p_id = ?;");
        product_query->bindValue(0, productname);
        product_query->bindValue(1, price);
        product_query->bindValue(2, Category);
        product_query->bindValue(3, quantity);
        product_query->bindValue(4, ID);
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
    ui->tableView->resizeColumnsToContents();
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
        QString productname = productModel->data(ix.siblingAtColumn(1)).toString();
        QString price = productModel->data(ix.siblingAtColumn(2)).toString();
        QString Category = productModel->data(ix.siblingAtColumn(3)).toString();
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
    QModelIndexList indexes = productModel->match(productModel->index(0, 1), Qt::EditRole, productname, -1, Qt::MatchFlags(flag));

    foreach(auto ix, indexes) {
        int ID = productModel->data(ix.siblingAtColumn(0)).toInt();
        int original_quantity = productModel->data(ix.siblingAtColumn(4)).toInt();

        if(original_quantity - quantity < 0)
        {
            emit quantityInformed(0);
            return;
        }

        product_query->prepare("UPDATE Product SET p_quantity = ? WHERE p_id = ?;");
        product_query->bindValue(0, original_quantity - quantity);
        product_query->bindValue(1, ID);
        product_query->exec();
        productModel->select();
        emit quantityInformed(1);
    }
}

/*상품 정보를 담고 있는 테이블 뷰에서 해당 상품을 클릭했을 경우
입력란의 lineedit에 해당 상품에 대한 텍스트가 보여질 수 있도록 구현하였다.*/
void ProductManagerForm::on_tableView_clicked(const QModelIndex &idx)
{
    QString ID = idx.sibling(idx.row(), 0).data().toString();
    QString productname = idx.sibling(idx.row(), 1).data().toString();
    QString price = idx.sibling(idx.row(), 2).data().toString();
    QString Category = idx.sibling(idx.row(), 3).data().toString();
    QString quantity = idx.sibling(idx.row(), 4).data().toString();

    ui->idLineEdit->setText(ID);
    ui->productnameLineEdit->setText(productname);
    ui->priceLineEdit->setText(price);
    ui->categoryLineEdit->setText(Category);
    ui->quantityLineEdit->setText(quantity);
}

