#ifndef SHOPLISTMANAGERFORM_H
#define SHOPLISTMANAGERFORM_H

#include <QWidget>
#include <QHash>
#include <QCalendarWidget>
#include <QMessageBox>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QStandardItemModel>

class ShopItem;
class QMenu;
class QTreeWidgetItem;
class QDate;

namespace Ui {
class ShoplistManagerForm;
}

class ShoplistManagerForm : public QWidget
{
    Q_OBJECT

public:
    explicit ShoplistManagerForm(QWidget *parent = nullptr);        /*Shoplistmanager 생성자*/
    ~ShoplistManagerForm();                                         /*Shoplistmanager 소멸자*/
    void loadData();                                                /*주문내역 데이터 불러오는 함수*/

public slots:
    void addClient(int, QString);                       /*Clientmanagerform에서 고객 정보 추가시 콤보 박스에 추가되는 고객 정보를 처리*/
    void modifyClient(int, QString, int);               /*Clientmanagerform에서 고객 정보 변경시 콤보 박스에서 변경되는 고객 정보를 처리*/
    void removeClient(int);                             /*Clientmanagerform에서 고객 정보 삭제시 콤보 박스에 삭제되는 고객 정보를 처리*/

    void addProduct(int ProductID, QString productname);    /*Productmanagerform에서 상품 정보 추가시 콤보 박스에 추가되는 상품 정보를 처리*/
    void modifyProduct(QString productname, int index);     /*Productmanagerform에서 상품 정보 변경시 콤보 박스에서 변경되는 상품 정보를 처리*/
    void removeProduct(int index);                          /*Productmanagerform에서 상품 정보 삭제시 콤보 박스에서 삭제되는 상품 정보를 처리*/

    void SendCustomerInfo(QStringList strings);               /*Clientmanager에서 받아온 고객 정보 중 필요한 데이터만 뽑아서 트리위젯에 추가하는 슬롯*/
    void SendProductInfo(QStringList strings);               /*Productmanager에서 받아온 상품 정보 중 필요한 데이터만 뽑아서 트리위젯에 추가하는 슬롯*/


private slots:
    /* QTreeWidget을 위한 슬롯 */
    void showContextMenu(const QPoint &);           /*마우스 오른족 버튼 클릭시 위치에 해당하는 정보를 제공하는 함수*/
    void removeItem();                              /*QAction을 위한 슬롯,  주문내역 삭제 함수*/
    void on_addPushButton_clicked();                /*주문내역 추가 함수*/
    void on_modifyPushButton_clicked();             /*주문내역 정보 변경 함수*/
    void on_searchPushButton_clicked();             /*검색 버튼 클릭시 입력한 정보로 검색한 결과를 나타내주는 함수*/
    void on_CustomerInfocomboBox_textActivated(const QString &arg1);    /*고객 정보 콤보박스의 데이터가 선택될 때 실행되는 함수*/
    void on_ProductInfocomboBox_textActivated(const QString &arg1);     /*상품 정보 콤보박스의 데이터가 선택될 때 실행되는 함수*/

    void on_tableView_clicked(const QModelIndex &index);
    void Informquantity(bool temp);

signals:
    void CustomerInfoSearched(int);         /*고객 콤보박스에서 고객 ID를 뽑아 Clientmanagerform으로 전송하는 시그널*/
    void ProductInfoSearched(QString);      /*상품 콤보박스에서 상품명을 뽑아 Productmanagerform으로 전송하는 시그널*/
    void QuantitySended(int, QString);          /*주문 정보에서 수량을 입력하면 상품란에서 해당 수량만큼 제거해주는 시그널*/

private:
    int makeId();                                   /*주문번호(ID)를 자동생성해주는 함수*/
    QMap<int, ShopItem*> shopList;                  /*주문내역 정보들을 저장하는 리스트*/
    Ui::ShoplistManagerForm *ui;                    /*ShoplistManagerForm의 UI*/
    QMenu* menu;
    QDate* date;
    QSqlTableModel *shopModel;
    QSqlQuery* shop_query;
    bool orderQuantity;                             /*주문 수량이 재고 수량보다 적거나 같은지 확인하는 변수*/
    QStandardItemModel* searchModel;
    QStandardItemModel* clientModel;
    QStandardItemModel* productModel;
};

#endif // SHOPLISTMANAGERFORM_H
