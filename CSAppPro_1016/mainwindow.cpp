#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "clientmanagerform.h"
#include "productmanagerform.h"
#include "shoplistmanagerform.h"
#include "chatserverform.h"
#include "chatclientform.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //    ClientManagerForm *clientForm1 = new ClientManagerForm(0);
    //    clientForm1->show();

    this->resize(1400, 800);
    orderForm = new ShoplistManagerForm(this);
    connect(orderForm, SIGNAL(destroyed()),
            orderForm, SLOT(deleteLater()));
    orderForm->loadData();
    orderForm->setWindowTitle(tr("Order Info"));

    clientForm = new ClientManagerForm(this);
    connect(clientForm, SIGNAL(destroyed()),
            clientForm, SLOT(deleteLater()));

    connect(clientForm, SIGNAL(clientAdded(int, QString)),
            orderForm, SLOT(addClient(int, QString)));
    connect(clientForm, SIGNAL(clientModified(int, QString, int)),
            orderForm, SLOT(modifyClient(int, QString, int)));
    connect(clientForm, SIGNAL(clientremoved(int)),
            orderForm, SLOT(removeClient(int)));

    clientForm->setWindowTitle(tr("Client Info"));
    //    ui->tabWidget->addTab(clientForm, "&Client Info");

    productForm = new ProductManagerForm(this);
    connect(productForm, SIGNAL(destroyed()),
            productForm, SLOT(deleteLater()));
    connect(productForm, SIGNAL(productAdded(int,QString)),
            orderForm, SLOT(addProduct(int,QString)));
    connect(productForm, SIGNAL(productModified(QString, int)),
            orderForm, SLOT(modifyProduct(QString, int)));
    connect(productForm, SIGNAL(productremoved(int)),
            orderForm, SLOT(removeProduct(int)));

    productForm->loadData();
    productForm->setWindowTitle(tr("Product Info"));


    //Shopmanagerfoem에서 고객을 선택했을 때 해당 고객에 대한 정보가 treeWidget에 보여지도록
    //하기 위한 연결 과정
    connect(orderForm, SIGNAL(CustomerInfoSearched(int)),
            clientForm, SLOT(SearchCustomerInfo(int)));
    connect(clientForm, SIGNAL(CustomerInfoSended(ClientItem*)),
            orderForm, SLOT(SendCustomerInfo(ClientItem*)));


    //Shopmanagerfoem에서 상품을 선택했을 때 해당 상품 정보가 treeWidget에 보여지도록
    //하기 위한 연결 과정
    connect(orderForm, SIGNAL(ProductInfoSearched(QString)),
            productForm, SLOT(SearchProductInfo(QString)));
    connect(productForm, SIGNAL(ProductInfoSended(ProductItem*)),
            orderForm, SLOT(SendProductInfo(ProductItem*)));


    QMdiSubWindow *cw = ui->mdiArea->addSubWindow(clientForm);
    ui->mdiArea->addSubWindow(productForm);
    ui->mdiArea->addSubWindow(orderForm);
    ui->mdiArea->setActiveSubWindow(cw);


    chatServerForm = new ChatServerForm(this);
    chatServerForm->setWindowTitle(tr("Chat Server"));
    ui->mdiArea->addSubWindow(chatServerForm);
    connect(clientForm, SIGNAL(clientAdded(int, QString)),
            chatServerForm, SLOT(addChatClient(int, QString)));

    connect(clientForm, SIGNAL(clientModified(int, QString, int)),
            chatServerForm, SLOT(modifyChatClient(int, QString, int)));
    connect(clientForm, SIGNAL(clientremoved(int)),
            chatServerForm, SLOT(removeChatClient(int)));

    clientForm->loadData();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionClient_triggered()
{
    if(clientForm != nullptr) {
        clientForm->setFocus();
    }
}

void MainWindow::on_actionProduct_triggered()
{
    if(productForm != nullptr) {
        productForm->setFocus();
    }
}

void MainWindow::on_actionOrder_triggered()
{
    if(orderForm != nullptr) {
        orderForm->setFocus();
    }
}

void MainWindow::on_actionChat_triggered()
{
    chatClientForm = new ChatClientForm();
    chatClientForm->show();
}

