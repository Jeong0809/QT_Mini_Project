#include "shoplistmanagerform.h"
#include "ui_shoplistmanagerform.h"
#include "shopitem.h"
#include "clientitem.h"
#include "productitem.h"

#include <QFile>
#include <QMenu>

ShoplistManagerForm::ShoplistManagerForm(QWidget *parent) :
    QWidget(parent), ui(new Ui::ShoplistManagerForm)
{
    ui->setupUi(this);

    QList<int> sizes;
    sizes << 790 << 450;
    ui->splitter->setSizes(sizes);

    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));

    menu = new QMenu;
    menu->addAction(removeAction);
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeWidget->setColumnWidth(0, 60);
    ui->treeWidget->setColumnWidth(1, 80);
    ui->treeWidget->setColumnWidth(2, 130);
    ui->treeWidget->setColumnWidth(3, 130);
    ui->treeWidget->setColumnWidth(4, 60);
    ui->treeWidget->setColumnWidth(5, 150);
    ui->treeWidget->setColumnWidth(6, 60);
    ui->CustomerInfotreeWidget->setColumnWidth(0, 70);
    ui->ProductInfotreeWidget->setColumnWidth(0, 130);

    date = new QDate();
    ui->dateEdit->setDate(date->currentDate());

    connect(ui->treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(ui->searchLineEdit, SIGNAL(returnPressed()),
            this, SLOT(on_searchPushButton_clicked()));
}

void ShoplistManagerForm::loadData()
{
    QFile file("shoplist.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QList<QString> row = line.split(", ");
        if(row.size()) {
            int id = row[0].toInt();
            int Quantity = row[4].toInt();
            int Price = row[6].toInt();
            int TotalPrice = row[7].toInt();
            ShopItem* c = new ShopItem(id, row[1], row[2], row[3], Quantity,
                                        row[5], Price, TotalPrice);
            ui->treeWidget->addTopLevelItem(c);
            shopList.insert(id, c);

            //emit shopAdded(row[1]);
        }
    }
    file.close( );
}

ShoplistManagerForm::~ShoplistManagerForm()
{
    delete ui;

    QFile file("shoplist.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    for (const auto& v : shopList) {
        ShopItem* c = v;
        out << c->ID() << ", " << c->getDate() << ", ";
        out << c->getCustomerInfo() << ", " << c->getProductInfo() << ", ";
        out << c->getQuantity() << ", " << c->getAddress() << ", ";
        out << c->getPrice() << ", " << c->getTotalPrice() << "\n";
    }
    file.close( );
}

int ShoplistManagerForm::makeId( )
{
    if(shopList.size( ) == 0) {
        return 100;
    }
    else {
        auto id = shopList.lastKey();
        return ++id;
    }
}

void ShoplistManagerForm::removeItem()
{
    QTreeWidgetItem* item = ui->treeWidget->currentItem();
    if(item != nullptr) {
        shopList.remove(item->text(0).toInt());
        ui->treeWidget->takeTopLevelItem(ui->treeWidget->indexOfTopLevelItem(item));
        delete item;
        ui->treeWidget->update();
    }
}

void ShoplistManagerForm::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = ui->treeWidget->mapToGlobal(pos);
    menu->exec(globalPos);
}

void ShoplistManagerForm::on_searchPushButton_clicked()
{
    ui->searchTreeWidget->clear();
    //    for(int i = 0; i < ui->treeWidget->columnCount(); i++)
    int i = ui->searchComboBox->currentIndex();

    auto flag = (i==0 || i==4) ? Qt::MatchCaseSensitive :
                                 Qt::MatchCaseSensitive | Qt::MatchContains;
    {
        auto items = ui->treeWidget->findItems(ui->searchLineEdit->text(), flag, i);

        foreach(auto i, items) {
            ShopItem* c = static_cast<ShopItem*>(i);
            int id = c->ID();
            QString Date = c->getDate();
            QString CustomerInfo = c->getCustomerInfo();
            QString ProductInfo = c->getProductInfo();
            int Quantity = c->getQuantity();
            QString Address = c->getAddress();
            int Price = c->getPrice();
            int TotalPrice = c->getTotalPrice();

            ShopItem* item = new ShopItem(id, Date, CustomerInfo, ProductInfo,
                                          Quantity, Address, Price, TotalPrice);
            ui->searchTreeWidget->addTopLevelItem(item);
        }
    }
}

void ShoplistManagerForm::on_modifyPushButton_clicked()
{
    QTreeWidgetItem* item = ui->treeWidget->currentItem();

    if(item != nullptr) {
        int key = item->text(0).toInt();
        ShopItem* c = shopList[key];

        QString Date, CustomerInfo, ProductInfo, Address;
        int Quantity, TotalPrice, Price;
        Date = ui->dateEdit->date().toString("yyyy-MM-dd");
        CustomerInfo = ui->CustomerInfocomboBox->currentText();
        ProductInfo = ui->ProductInfocomboBox->currentText();
        Quantity = ui->QuantityLineEdit->text().toInt();
        Address = ui->CustomerInfotreeWidget->topLevelItem(0)->text(2);
        Price = ui->ProductInfotreeWidget->topLevelItem(0)->text(1).toInt();
        TotalPrice = Quantity * Price;

        c->setDate(Date);
        c->setCustomerInfo(CustomerInfo);
        c->setProductInfo(ProductInfo);
        c->setQuantity(Quantity);
        c->setAddress(Address);
        c->setPrice(Price);
        c->setTotalPrice(TotalPrice);

        shopList[key] = c;
    }
}

void ShoplistManagerForm::on_addPushButton_clicked()
{
    QString Date, CustomerInfo, ProductInfo, address;
    int Quantity, Price, TotalPrice;

    int ID = makeId();
    Date = ui->dateEdit->date().toString("yyyy-MM-dd");
    CustomerInfo = ui->CustomerInfocomboBox->currentText();
    ProductInfo = ui->ProductInfocomboBox->currentText();
    Quantity = ui->QuantityLineEdit->text().toInt();

    if(ui->CustomerInfotreeWidget->topLevelItemCount() == 0
            || ui->ProductInfotreeWidget->topLevelItemCount() == 0 || Quantity == 0)
    {
        QMessageBox::warning(this, tr("Error"), \
                              tr("모두 입력해주세요"));
        return;
    }

    address = ui->CustomerInfotreeWidget->topLevelItem(0)->text(2);
    Price = ui->ProductInfotreeWidget->topLevelItem(0)->text(1).toInt();
    TotalPrice = Price * Quantity;

    if(Date.length()) {
        qDebug() << address;
        ShopItem* c = new ShopItem(ID, Date, CustomerInfo, ProductInfo, Quantity, address, Price, TotalPrice);
        shopList.insert(ID, c);
        ui->treeWidget->addTopLevelItem(c);
        //emit shopAdded(Date);
    }
}

//고객 정보 추가시 콤보 박스에 추가되는 고객 정보를 처리
void ShoplistManagerForm::addClient(int CustomerID, QString name)
{
    ui->CustomerInfocomboBox->addItem(name + " ( ID : " + QString::number(CustomerID) + " )" );
}

//고객 정보 변경시 콤보 박스에서 변경되는 고객 정보를 처리
void ShoplistManagerForm::modifyClient(int CustomerID, QString name, int index)
{    
    ui->CustomerInfocomboBox->setItemText(index, name + " ( ID : " + QString::number(CustomerID) + " )");
}

//고객 정보 삭제시 콤보 박스에 삭제되는 고객 정보를 처리
void ShoplistManagerForm::removeClient(int index)
{
    ui->CustomerInfocomboBox->removeItem(index);
}

//상품 정보 추가시 콤보 박스에 추가되는 상품 정보를 처리
void ShoplistManagerForm::addProduct(int ProductID, QString productname)
{
    ui->ProductInfocomboBox->addItem(productname);
}

//상품 정보 변경시 콤보 박스에서 변경되는 상품 정보를 처리
void ShoplistManagerForm::modifyProduct(QString productname, int index)
{
    ui->ProductInfocomboBox->setItemText(index, productname);
}

//상품 정보 삭제시 콤보 박스에서 삭제되는 상품 정보를 처리
void ShoplistManagerForm::removeProduct(int index)
{
    ui->ProductInfocomboBox->removeItem(index);
}

void ShoplistManagerForm::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    ui->idLineEdit->setText(item->text(0));
    ui->dateEdit->setDate(QDate::fromString(item->text(1), "yyyy-MM-dd"));
    ui->CustomerInfocomboBox->setCurrentText(item->text(2));
    ui->ProductInfocomboBox->setCurrentText(item->text(3));
    ui->QuantityLineEdit->setText(item->text(4));

    emit CustomerInfoSearched(item->text(2).right(5).left(3).toInt());
    emit ProductInfoSearched(item->text(3));
    ui->toolBox->setCurrentIndex(0);
}


void ShoplistManagerForm::on_CustomerInfocomboBox_textActivated(const QString &temp)
{
    int ID =  temp.right(5).left(3).toInt();
    emit CustomerInfoSearched(ID);
}

void ShoplistManagerForm::SendCustomerInfo(ClientItem* c)
{
    ui->CustomerInfotreeWidget->clear();
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, c->getName());
    item->setText(1, c->getPhoneNumber());
    item->setText(2, c->getAddress());
    ui->CustomerInfotreeWidget->addTopLevelItem(item);
}

void ShoplistManagerForm::SendProductInfo(ProductItem* c)
{
    ui->ProductInfotreeWidget->clear();
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, c->getProductName());
    item->setText(1, QString::number(c->getPrice()));
    item->setText(2, c->getCategory());
    ui->ProductInfotreeWidget->addTopLevelItem(item);
}

void ShoplistManagerForm::on_ProductInfocomboBox_textActivated(const QString &productname)
{
    emit ProductInfoSearched(productname);
}

//void ShoplistManagerForm::SendCustomerInfo(QString name, QString phonenumber, QString address)
//{
//    //QTreeWidgetItem *item = static_cast<QTreeWidgetItem*>(c);
//    //ui->CustomerInfotreeWidget->addTopLevelItem(item);
//    ui->CustomerInfotreeWidget->clear();
//    QTreeWidgetItem *item = new QTreeWidgetItem;
//    item->setText(0, name);
//    item->setText(1, phonenumber);
//    item->setText(2, address);

//    ui->CustomerInfotreeWidget->addTopLevelItem(item);
//}




