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

    /*Remove가 트리거되면 removeItem 함수 실행*/
    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));

    menu = new QMenu;
    /*메뉴 생성 후 Remove 추가*/
    menu->addAction(removeAction);
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    /*Shoplist 트리 위젯 컬럼명 공간 조절*/
    ui->treeWidget->setColumnWidth(0, 60);
    ui->treeWidget->setColumnWidth(1, 80);
    ui->treeWidget->setColumnWidth(2, 130);
    ui->treeWidget->setColumnWidth(3, 130);
    ui->treeWidget->setColumnWidth(4, 60);
    ui->treeWidget->setColumnWidth(5, 150);
    ui->treeWidget->setColumnWidth(6, 60);

    /*고객 정보 트리 위젯 컬럼명 공간 조절*/
    ui->CustomerInfotreeWidget->setColumnWidth(0, 70);

    /*상품 정보 트리 위젯 컬럼명 공간 조절*/
    ui->ProductInfotreeWidget->setColumnWidth(0, 130);

    /*처음 실행 시 오늘 날짜가 선택되어 있도록 초깃값을 설정해주었다*/
    date = new QDate();
    ui->dateEdit->setDate(date->currentDate());

    /*Shoplistmanager 트리 위젯에서 마우스 오른쪽 버튼 클릭시 해당 위치에서 Remove 창이 나오도록 연결*/
    connect(ui->treeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));

    /*검색 시 lineedit에 검색할 단어 입력 후 search 버튼 누르면 검색된 결과가 출력되도록 연결*/
    connect(ui->searchLineEdit, SIGNAL(returnPressed()),
            this, SLOT(on_searchPushButton_clicked()));
}

/*shoplist.txt 파일에서 저장되어 있었던 주문내역 데이터를 불러오는 함수*/
void ShoplistManagerForm::loadData()
{
    QFile file("shoplist.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);

    /*in이 파일의 끝이 아니라면 줄마다 라인별로 읽어온다.*/
    while (!in.atEnd()) {
        QString line = in.readLine();

        /*주문번호(ID), 날짜, 고객 정보, 상품 정보, 수량, 주소, 가격, 총가격을 ", "로 구분*/
        QList<QString> row = line.split(", ");
        if(row.size()) {
            int id = row[0].toInt();        //주문 내역 추출
            int Quantity = row[4].toInt();  //주문 수량 추출
            int Price = row[6].toInt();     //상품 가격 추출
            int TotalPrice = row[7].toInt();    //총 가격 추출
            ShopItem* c = new ShopItem(id, row[1], row[2], row[3], Quantity,
                                        row[5], Price, TotalPrice);
            ui->treeWidget->addTopLevelItem(c);     /*txt파일에 저장되어 있던 주문내역 정보를 트리위젯에 추가*/
            shopList.insert(id, c);                 /*shoplist에도 불러온 내용 추가*/
        }
    }
    file.close( );
}

/*shoplist.txt 파일에 주문내역 정보의 데이터를 ", "로 구분해서 저장*/
ShoplistManagerForm::~ShoplistManagerForm()
{
    delete ui;

    QFile file("shoplist.txt");

    /*파일이 쓰기 전용 텍스트 파일 일때만 저장*/
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

     /*주문번호(ID), 날짜, 고객 정보, 상품 정보, 수량, 주소, 가격, 총가격을 ", "로 구분해서 저장*/
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

/*50000번부터 상품 ID가 자동으로 부여될 수 있도록 설정하였다.*/
int ShoplistManagerForm::makeId( )
{
    /*shopList의 사이즈가 0이면 데이터가 없는 것이므로 ID가 50000번부터 시작*/
    if(shopList.size( ) == 0) {
        return 1;
    }
    else {
        /*shopList의 마지막 키의 ID에서 +1한 ID 번호 자동 부여*/
        auto id = shopList.lastKey();
        return ++id;
    }
}

/*주문내역 데이터에서 선택한 주문내역 정보를 삭제하는 함수*/
void ShoplistManagerForm::removeItem()
{
    /*트리위젯에서 선택된 주문내역 데이터*/
    QTreeWidgetItem* item = ui->treeWidget->currentItem();
    if(item != nullptr) {

        /*shopList에서 현재 클릭한 item의 ID에 해당하는 주문내역 정보를 삭제*/
        shopList.remove(item->text(0).toInt());

        /*트리 위젯에서도 해당 item의 주문내역 정보 제거*/
        ui->treeWidget->takeTopLevelItem(ui->treeWidget->indexOfTopLevelItem(item));
        delete item;
        ui->treeWidget->update();
    }
}

/*트리위젯에서 마우스 오른쪽 버튼의 위치에 해당하는 행위를 인식하기 위한 함수*/
void ShoplistManagerForm::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = ui->treeWidget->mapToGlobal(pos);
    menu->exec(globalPos);
}

/*Search 버튼을 눌렀을 때 검색 기능이 수행되는 함수*/
void ShoplistManagerForm::on_searchPushButton_clicked()
{
    ui->searchTreeWidget->clear();

    /*현재 검색 콤보박스에서 선택된 인덱스, 어떤 인자로 검색할 것인지 나타냄*/
    int i = ui->searchComboBox->currentIndex();

    /*MatchCaseSensitive : 대소문자 구별, MatchContains : 검색하는게 항목에 포함되어 있는지 확인
    검색 기능에서 상품 ID나 수량을 통해 검색 시 정확한 ID나 정확한 수량을 입력할 시에만
    검색이 되도록 구현*/
    auto flag = (i==0 || i==4) ? Qt::MatchCaseSensitive :
                                 Qt::MatchCaseSensitive | Qt::MatchContains;
    {
        /*검색창에 입력된 텍스트를 통해 상품 정보 트리위젯에서 flag 조건을 통해 검색,
          i를 통해 어떤 항목으로 검색할지 결정*/
        auto items = ui->treeWidget->findItems(ui->searchLineEdit->text(), flag, i);

        foreach(auto i, items) {
            /*i의 자료형을 ShopItem 형으로 변환 후 고정*/
            ShopItem* c = static_cast<ShopItem*>(i);

            /*주문 내역에 대한 정보를 객체에서 불러옴*/
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

            /*검색을 통해 찾은 item 정보를 search 트리위젯에 추가*/
            ui->searchTreeWidget->addTopLevelItem(item);
        }
    }
}

/*Modify 버튼 클릭시 주문내역 정보가 변경되는 것을 실행하는 함수*/
void ShoplistManagerForm::on_modifyPushButton_clicked()
{
    /*트리위젯에서 클릭한 주문내역 정보를 item에 저장*/
    QTreeWidgetItem* item = ui->treeWidget->currentItem();

    if(item != nullptr) {

        /*트리위젯에서 선택한 주문내역 정보의 ID 저장*/
        int key = item->text(0).toInt();

        /*ID에 해당하는 주문내역 정보 객체*/
        ShopItem* c = shopList[key];

        QString Date, CustomerInfo, ProductInfo, Address;
        int Quantity, TotalPrice, Price;

        /*변경할 주문 내역 데이터 작성*/
        Date = ui->dateEdit->date().toString("yyyy-MM-dd");
        CustomerInfo = ui->CustomerInfocomboBox->currentText().right(7).left(5);
        ProductInfo = ui->ProductInfocomboBox->currentText();
        Quantity = ui->QuantityLineEdit->text().toInt();
        Address = ui->CustomerInfotreeWidget->topLevelItem(0)->text(2);
        Price = ui->ProductInfotreeWidget->topLevelItem(0)->text(1).toInt();
        TotalPrice = Quantity * Price;

        /*사용자가 입력한 값으로 데이터 변경*/
        c->setDate(Date);
        c->setCustomerInfo(CustomerInfo);
        c->setProductInfo(ProductInfo);
        c->setQuantity(Quantity);
        c->setAddress(Address);
        c->setPrice(Price);
        c->setTotalPrice(TotalPrice);

        /*변경한 데이터의 객체(정보)를 shopList에 저장*/
        shopList[key] = c;
    }
}

/*Add 버튼 클릭시 주문내역 정보가 추가되는 것을 실행하는 함수*/
void ShoplistManagerForm::on_addPushButton_clicked()
{
    QString Date, CustomerInfo, ProductInfo, address;
    int Quantity, Price, TotalPrice;

    /*ID는 자동 생성*/
    int ID = makeId();

    /*날짜, 고객정보, 상품정보, 수량은 사용자가 입력해줌*/
    Date = ui->dateEdit->date().toString("yyyy-MM-dd");
    CustomerInfo = ui->CustomerInfocomboBox->currentText().right(7).left(5);
    ProductInfo = ui->ProductInfocomboBox->currentText();
    Quantity = ui->QuantityLineEdit->text().toInt();

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

    /*날짜가 입력되면 해당 정보들을 객체로 shopList에 추가*/
    if(Date.length()) {
        qDebug() << address;
        ShopItem* c = new ShopItem(ID, Date, CustomerInfo, ProductInfo, Quantity, address, Price, TotalPrice);
        shopList.insert(ID, c);

        /*트리위젯에 해당 정보 추가*/
        ui->treeWidget->addTopLevelItem(c);
        ui->treeWidget->update();
    }
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
void ShoplistManagerForm::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    /*트리위젯에서 선택한 데이터가 입력란에 보여질 수 있도록 설정*/
    ui->idLineEdit->setText(item->text(0));
    ui->dateEdit->setDate(QDate::fromString(item->text(1), "yyyy-MM-dd"));
    ui->CustomerInfocomboBox->setCurrentIndex(ui->CustomerInfocomboBox->findText(item->text(2), Qt::MatchContains));
    ui->ProductInfocomboBox->setCurrentText(item->text(3));
    ui->QuantityLineEdit->setText(item->text(4));

    /*트리위젯에서 데이터를 선택했을 때 고객 ID와 상품명 정보를 통해 오른쪽에 고객정보 트리위젯과
      상품 정보 트리위젯에 그에 해당하는 정보가 보여질 수 있도록 시그널 전송*/
    emit CustomerInfoSearched(item->text(2).toInt());
    emit ProductInfoSearched(item->text(3));

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
void ShoplistManagerForm::SendCustomerInfo(ClientItem* c)
{
    ui->CustomerInfotreeWidget->clear();
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, c->getName());         //이름 정보 추출 후 설정
    item->setText(1, c->getPhoneNumber());  //휴대폰 번호 정보 추출 후 설정
    item->setText(2, c->getAddress());      //주소 정보 추출 후 설정
    ui->CustomerInfotreeWidget->addTopLevelItem(item);  //트리 위젯에 추가
}

/*Shopmanagerform에서 보낸 싱품명을 통해 Productmanagerform에서 해당 상품의 정보를 전체 전송한 후
필요한 정보만 뽑아서 주문내역 트리위젯에 추가해주는 방식이다.*/
void ShoplistManagerForm::SendProductInfo(ProductItem* c)
{
    ui->ProductInfotreeWidget->clear();
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, c->getProductName());              //상품명 정보 추출 후 설정
    item->setText(1, QString::number(c->getPrice()));   //가격 정보 추출 후 설정
    item->setText(2, c->getCategory());                 //카테고리 정보 추출 후 설정
    ui->ProductInfotreeWidget->addTopLevelItem(item);   //트리 위젯에 추가
}

/*상품 콤보박스의 경우 해당 상품명을 productmanagerform으로 보내준 다음 해당 상품의 정보를 불러온다.*/
void ShoplistManagerForm::on_ProductInfocomboBox_textActivated(const QString &productname)
{
    emit ProductInfoSearched(productname);
}
