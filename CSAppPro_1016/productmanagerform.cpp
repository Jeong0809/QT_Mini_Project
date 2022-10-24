#include "productmanagerform.h"
#include "ui_productmanagerform.h"
#include "productitem.h"

#include <QFile>
#include <QMenu>
#include <QMessageBox>

ProductManagerForm::ProductManagerForm(QWidget *parent):QWidget(parent), ui(new Ui::ProductManagerForm)
{
    ui->setupUi(this);

    QList<int> sizes;
    sizes << 560 << 400;
    ui->splitter->setSizes(sizes);

    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));

    menu = new QMenu;
    menu->addAction(removeAction);
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeWidget->setColumnWidth(0, 60);
    ui->treeWidget->setColumnWidth(1, 150);

    connect(ui->treeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));
    connect(ui->searchLineEdit, SIGNAL(returnPressed()),
            this, SLOT(on_searchPushButton_clicked()));
}

void ProductManagerForm::loadData()
{
    QFile file("productlist.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QList<QString> row = line.split(", ");
        if(row.size()) {
            int id = row[0].toInt();
            int price = row[2].toInt();
            ProductItem* c = new ProductItem(id, row[1], price, row[3]);
            ui->treeWidget->addTopLevelItem(c);
            productList.insert(id, c);

            emit productAdded(id, row[1]);
        }
    }
    file.close( );
}

ProductManagerForm::~ProductManagerForm()
{
    delete ui;

    QFile file("productlist.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    for (const auto& v : productList) {
        ProductItem* c = v;
        out << c->ID() << ", " << c->getProductName() << ", ";
        out << c->getPrice() << ", ";
        out << c->getCategory() << "\n";
    }
    file.close( );
}

int ProductManagerForm::makeId( )
{
    if(productList.size( ) == 0) {
        return 10000;
    } else {
        auto id = productList.lastKey();
        return ++id;
    }
}

void ProductManagerForm::removeItem()
{
    QTreeWidgetItem* item = ui->treeWidget->currentItem();
    if(item != nullptr) {
        int index = ui->treeWidget->currentIndex().row();
        productList.remove(item->text(0).toInt());
        ui->treeWidget->takeTopLevelItem(ui->treeWidget->indexOfTopLevelItem(item));
        delete item;
        ui->treeWidget->update();
        emit productremoved(index);
    }

    ui->idLineEdit->clear();
    ui->productnameLineEdit->clear();
    ui->priceLineEdit->clear();
    ui->categoryLineEdit->clear();
}

void ProductManagerForm::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = ui->treeWidget->mapToGlobal(pos);
    menu->exec(globalPos);
}

void ProductManagerForm::on_searchPushButton_clicked()
{
    ui->searchTreeWidget->clear();
    int i = ui->searchComboBox->currentIndex();

    //MatchCaseSensitive : 대소문자 구별, MatchContains : 검색하는게 항목에 포함되어 있는지 확인
    /*검색 기능에서 상품 ID나 가격을 통해 검색 시 정확한 ID나 정확한 가격을 입력할 시에만
    검색이 되도록 구현해주었다.*/
    auto flag = (i==0 || i==2) ? Qt::MatchCaseSensitive:
                                 Qt::MatchCaseSensitive | Qt::MatchContains;

    {
        auto items = ui->treeWidget->findItems(ui->searchLineEdit->text(), flag, i);

        foreach(auto i, items) {
            ProductItem* c = static_cast<ProductItem*>(i);
            int id = c->ID();
            QString productname = c->getProductName();
            QString Category = c->getCategory();
            int price = c->getPrice();
            ProductItem* item = new ProductItem(id, productname, price, Category);
            ui->searchTreeWidget->addTopLevelItem(item);
        }
    }
}

void ProductManagerForm::on_modifyPushButton_clicked()
{
    QTreeWidgetItem* item = ui->treeWidget->currentItem();
    if(item != nullptr) {
        int index = ui->treeWidget->currentIndex().row();
        int key = item->text(0).toInt();
        ProductItem* c = productList[key];
        QString productname, Category;
        int price;
        productname = ui->productnameLineEdit->text();
        price = ui->priceLineEdit->text().toInt();
        Category = ui->categoryLineEdit->text();
        c->setProductName(productname);
        c->setPrice(price);
        c->setCategory(Category);
        productList[key] = c;
        emit productModified(productname, index);
    }

    ui->idLineEdit->clear();
    ui->productnameLineEdit->clear();
    ui->priceLineEdit->clear();
    ui->categoryLineEdit->clear();
}

void ProductManagerForm::on_addPushButton_clicked()
{
    QString productname, Category;
    int price;
    int id = makeId();
    productname = ui->productnameLineEdit->text();
    price = ui->priceLineEdit->text().toInt();
    Category = ui->categoryLineEdit->text();


    if(productname == "" || ui->priceLineEdit->text() == "" || Category == "")
    {
        QMessageBox::warning(this, tr("Error"), tr("모두 입력해주세요"));
        return;
    }

    if(productname.length()) {
        ProductItem* c = new ProductItem(id, productname, price, Category);
        productList.insert(id, c);
        ui->treeWidget->addTopLevelItem(c);
        emit productAdded(id, productname);
    }

    ui->idLineEdit->clear();
    ui->productnameLineEdit->clear();
    ui->priceLineEdit->clear();
    ui->categoryLineEdit->clear();
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
        auto items = ui->treeWidget->findItems(productname, flag, i);
        foreach(auto i, items) {
            ProductItem* c = static_cast<ProductItem*>(i);
            int id = c->ID();
            QString name = c->getProductName();
            QString category = c->getCategory();
            int price = c->getPrice();
            emit ProductInfoSended(c);
        }
    }

}
