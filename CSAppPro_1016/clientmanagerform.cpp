#include "clientmanagerform.h"
#include "ui_clientmanagerform.h"
#include "clientitem.h"

#include <QFile>
#include <QMenu>
#include <QMessageBox>

ClientManagerForm::ClientManagerForm(QWidget *parent)
    : QWidget(parent), ui(new Ui::ClientManagerForm)
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

    //컬럼명 공간 조절
    ui->treeWidget->setColumnWidth(0, 60);
    ui->treeWidget->setColumnWidth(3, 180);

    connect(ui->treeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));
    connect(ui->searchLineEdit, SIGNAL(returnPressed()),
            this, SLOT(on_searchPushButton_clicked()));
}

/*clientlist.txt 파일에서 저장되어 있었던 고객 데이터를 불러오는 함수*/
void ClientManagerForm::loadData()
{
    QFile file("clientlist.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    /*in이 파일의 끝이 아니라면 줄마다 라인별로 읽어온다.*/
    while (!in.atEnd()) {
        QString line = in.readLine();
        QList<QString> row = line.split(", ");
        if(row.size()) {
            int id = row[0].toInt();
            ClientItem* c = new ClientItem(id, row[1], row[2], row[3]);
            ui->treeWidget->addTopLevelItem(c);
            clientList.insert(id, c);

            emit clientAdded(id, row[1]);
        }
    }
    file.close( );
}

/*clientlist.txt 파일에 고객 정보의 데이터를 ", "로 구분해서 저장해준다*/
ClientManagerForm::~ClientManagerForm()
{
    delete ui;

    QFile file("clientlist.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    //ID, 고객명, 휴대폰번호, 주소를 ,로 구분해서 저장한다.
    QTextStream out(&file);
    for (const auto& v : clientList) {
        ClientItem* c = v;
        out << c->ID() << ", " << c->getName() << ", ";
        out << c->getPhoneNumber() << ", ";
        out << c->getAddress() << "\n";
    }
    file.close( );
}

/*100번부터 고객 ID가 자동으로 부여될 수 있도록 설정하였다.*/
int ClientManagerForm::makeId( )
{
    if(clientList.size( ) == 0) {
        return 100;
    } else {
        auto id = clientList.lastKey();
        return ++id;
    }
}

void ClientManagerForm::removeItem()
{
    /*고객 삭제 같은 경우 삭제하고자 하는 고객을 클릭하고 마우스 오른쪽 버튼을 눌렀을 때
    remove action을 통해 삭제하기 때문에 해당 인덱스를 SIGNAL로 보내준다*/
    QTreeWidgetItem* item = ui->treeWidget->currentItem();
    if(item != nullptr) {
        int index = ui->treeWidget->currentIndex().row();
        clientList.remove(item->text(0).toInt());
        ui->treeWidget->takeTopLevelItem(ui->treeWidget->indexOfTopLevelItem(item));
        ui->treeWidget->update();
        emit clientremoved(index);
    }

    //고객 데이터를 삭제하고 난 이후에 입력란을 clear 해준다.
    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->phoneNumberLineEdit->clear();
    ui->addressLineEdit->clear();
}

void ClientManagerForm::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = ui->treeWidget->mapToGlobal(pos);
    menu->exec(globalPos);
}

void ClientManagerForm::on_searchPushButton_clicked()
{
    ui->searchTreeWidget->clear();
    int i = ui->searchComboBox->currentIndex();
    auto flag = (i)? Qt::MatchCaseSensitive|Qt::MatchContains
                   : Qt::MatchCaseSensitive;
    {
        auto items = ui->treeWidget->findItems(ui->searchLineEdit->text(), flag, i);

        foreach(auto i, items) {
            ClientItem* c = static_cast<ClientItem*>(i);
            int id = c->ID();
            QString name = c->getName();
            QString number = c->getPhoneNumber();
            QString address = c->getAddress();
            ClientItem* item = new ClientItem(id, name, number, address);
            ui->searchTreeWidget->addTopLevelItem(item);
        }
    }
}

void ClientManagerForm::on_modifyPushButton_clicked()
{
    QTreeWidgetItem* item = ui->treeWidget->currentItem();   

    if(item != nullptr) {
        int index = ui->treeWidget->currentIndex().row();
        int key = item->text(0).toInt();
        ClientItem* c = clientList[key];
        QString name, number, address;
        name = ui->nameLineEdit->text();
        number = ui->phoneNumberLineEdit->text();
        address = ui->addressLineEdit->text();
        c->setName(name);
        c->setPhoneNumber(number);
        c->setAddress(address);
        clientList[key] = c;
        ui->treeWidget->update();
        emit clientModified(key, name, index);
    }

    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->phoneNumberLineEdit->clear();
    ui->addressLineEdit->clear();
}

void ClientManagerForm::on_addPushButton_clicked()
{
    QString name, number, address;
    int id = makeId( );
    name = ui->nameLineEdit->text();
    number = ui->phoneNumberLineEdit->text();
    address = ui->addressLineEdit->text();

    if(name == "" || number == "" || address == "")
    {
        QMessageBox::warning(this, tr("Error"), tr("모두 입력해주세요"));
        return;
    }

    if(name.length()) {
        ClientItem* c = new ClientItem(id, name, number, address);
        clientList.insert(id, c);
        ui->treeWidget->addTopLevelItem(c);
        ui->treeWidget->update();
        emit clientAdded(id, name);
    }

    ui->idLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->phoneNumberLineEdit->clear();
    ui->addressLineEdit->clear();
}

/*고객 정보를 담고 있는 트리위젯에서 해당 고객을 클릭했을 경우
입력란의 lineedit에 해당 고객에 대한 텍스트가 보여질 수 있도록 구현하였다.*/
void ClientManagerForm::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    ui->idLineEdit->setText(item->text(0));
    ui->nameLineEdit->setText(item->text(1));
    ui->phoneNumberLineEdit->setText(item->text(2));
    ui->addressLineEdit->setText(item->text(3));
    ui->toolBox->setCurrentIndex(0);
}

/*Shopmanagerform에서 고객정보 콤보박스를 클릭 시 하단 트리위젯에
클릭한 고객의 정보들의 보여질 수 있도록 하기위한 시그널 함수*/
void ClientManagerForm::SearchCustomerInfo(int ID)
{
    ClientItem* c = clientList[ID];
    emit CustomerInfoSended(c);
}

