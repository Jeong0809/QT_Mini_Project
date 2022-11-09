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
    //메인 윈도우 창 사이즈 1400 X 800
    this->resize(1400, 800);

    //Shopmanagerform 객체 생성
    orderForm = new ShoplistManagerForm(this);
    connect(orderForm, SIGNAL(destroyed()), orderForm, SLOT(deleteLater()));

    //Shopmanagerform의 기존 데이터 불러오기
    orderForm->loadData();

    //Shopmanagerform의 윈도우 제목은 Order Info로 설정
    orderForm->setWindowTitle(tr("Order Info"));

    //ClientManagerForm 객체 생성
    clientForm = new ClientManagerForm(this);

    //ClientManagerForm의 윈도우 제목은 Client Info로 설정
    clientForm->setWindowTitle(tr("Client Info"));

    connect(clientForm, SIGNAL(destroyed()),
            clientForm, SLOT(deleteLater()));

    //clientForm에서 고객 데이터가 추가되면 orderForm의 고객 정보 콤보박스에서도 고객 데이터 추가
    connect(clientForm, SIGNAL(clientAdded(int, QString)), //고객 ID, 고객명 전달
            orderForm, SLOT(addClient(int, QString)));

    //clientForm에서 고객 데이터가 변경되면 orderForm의 고객 정보 콤보박스에서도 고객 데이터 변경
    connect(clientForm, SIGNAL(clientModified(int, QString, int)),
            orderForm, SLOT(modifyClient(int, QString, int))); //고객 ID, 고객명, 트리위젯에서 선택된 인덱스값 전달

    //clientForm에서 고객 데이터가 삭제되면 orderForm의 고객 정보 콤보박스에서도 고객 데이터 삭제
    connect(clientForm, SIGNAL(clientremoved(int)),
            orderForm, SLOT(removeClient(int))); //트리위젯에서 선택된 인덱스 값 전달


    //ProductManagerForm 객체 생성
    productForm = new ProductManagerForm(this);
    connect(productForm, SIGNAL(destroyed()),
            productForm, SLOT(deleteLater()));

    //productForm에서 상품 데이터가 추가되면 orderForm의 상품 정보 콤보박스에서도 상품 데이터 추가
    connect(productForm, SIGNAL(productAdded(int,QString)), //상품 ID, 상품명 전달
            orderForm, SLOT(addProduct(int,QString)));

    //productForm에서 상품 데이터가 변경되면 orderForm의 상품 정보 콤보박스에서도 상품 데이터 변경
    connect(productForm, SIGNAL(productModified(QString, int)), //상품명, 트리위젯에서 선택된 인덱스 값 전달
            orderForm, SLOT(modifyProduct(QString, int)));

    //productForm에서 상품 데이터가 삭제되면 orderForm의 상품 정보 콤보박스에서도 상품 데이터 삭제
    connect(productForm, SIGNAL(productremoved(int)),
            orderForm, SLOT(removeProduct(int))); //트리위젯에서 선택된 인덱스 값 전달



    //ProductManagerForm의 윈도우 제목은 Product Info로 설정
    productForm->setWindowTitle(tr("Product Info"));


    /*Shopmanagerfoem에 고객 정보 콤보박스에서 고객을 선택했을 때
    해당 고객에 대한 정보가 고객 정보 treeWidget에 보여지도록 하기 위한 연결 과정*/
    connect(orderForm, SIGNAL(CustomerInfoSearched(int)),
            clientForm, SLOT(SearchCustomerInfo(int)));

    /*orderForm에서 고객 ID를 clientForm으로 전달한 후 그에 해당하는 고객 데이터 객체를
      다시 orderForm으로 보내어 필요한 고객 데이터를 사용*/
    connect(clientForm, SIGNAL(CustomerInfoSended(ClientItem*)),
            orderForm, SLOT(SendCustomerInfo(ClientItem*)));

    /*Shopmanagerfoem에서 상품 정보 콤보박스에서 상품을 선택했을 때
    해당 상품 정보가 상품 정보 treeWidget에 보여지도록 하기 위한 연결 과정*/
    connect(orderForm, SIGNAL(ProductInfoSearched(QString)),
            productForm, SLOT(SearchProductInfo(QString)));

    /*orderForm에서 상품명을 productForm으로 전달한 후 그에 해당하는 상품 데이터 객체를
      다시 orderForm으로 보내어 필요한 상품 데이터를 사용*/
    connect(productForm, SIGNAL(ProductInfoSended(ProductItem*)),
            orderForm, SLOT(SendProductInfo(ProductItem*)));

    //ClientForm, ProductForm, OrderForm을 각각 mdiArea로 추가
    QMdiSubWindow *cw = ui->mdiArea->addSubWindow(clientForm);
    ui->mdiArea->addSubWindow(productForm);
    ui->mdiArea->addSubWindow(orderForm);
    ui->mdiArea->setActiveSubWindow(cw);

    //ChatServerForm 객체 생성 후 윈도우 타이틀을 Chat Server로 설정
    chatServerForm = new ChatServerForm(this);
    chatServerForm->setWindowTitle(tr("Chat Server"));

    //mdiArea로 chatServerform을 추가
    ui->mdiArea->addSubWindow(chatServerForm);

    /*채팅 서버에서 고객 리스트가 보여질 때 고객 탭에서 고객이 추가되거나 삭제되거나
    변경될 경우 즉각 채팅 서버에도 반영될 수 있도록 하기 위해 SIGNAL, SLOT을 이용해
    고객 정보에 대한 추가, 삭제, 변경 사항을 전송*/
    connect(clientForm, SIGNAL(clientAdded(int, QString)),
            chatServerForm, SLOT(addChatClient(int, QString)));
    connect(clientForm, SIGNAL(clientModified(int, QString, int)),
            chatServerForm, SLOT(modifyChatClient(int, QString, int)));
    connect(clientForm, SIGNAL(clientremoved(int)),
            chatServerForm, SLOT(removeChatClient(int)));

    clientForm->loadData();
    //ProductManagerForm 기존 데이터 불러오기
    productForm->loadData();


}

MainWindow::~MainWindow()
{
    delete ui;
    delete chatServerForm;
    delete clientForm;
    delete productForm;
    delete orderForm;

    QStringList list = QSqlDatabase::connectionNames();
    for(int i = 0; i < list.count(); ++i) {
        QSqlDatabase::removeDatabase(list[i]);
    }
}

/*메인 윈도우에서 Client 버튼 클릭시 동작되는 함수*/
void MainWindow::on_actionClient_triggered()
{
    if(clientForm != nullptr) {
        clientForm->setFocus();
    }
}

/*메인 윈도우에서 Chat 버튼 클릭시 동작되는 함수*/
void MainWindow::on_actionChat_triggered()
{
    //Chat 버튼 클릭 시 ChatClientForm 객체 생성
    chatClientForm = new ChatClientForm();
    chatClientForm->show();

    /*클라이언트에서 입력받은 고객명을 서버단으로 전송한 뒤 서버단에서 clientList내에 해당 고객이 있는지
      확인 후 유무에 따라 0, 1값을 다시 클라이언트로 보내주는 역할을 수행*/
    connect(chatClientForm, SIGNAL(LogInChecked(QString)),
            chatServerForm, SLOT(CheckLogIn(QString)));
    connect(chatServerForm, SIGNAL(SendLogInChecked(int)),
            chatClientForm, SLOT(LogInCheckSended(int)));
}

