#ifndef PRODUCTMANAGERFORM_H
#define PRODUCTMANAGERFORM_H

#include <QWidget>
#include <QHash>
#include <QTableView>
#include <QSqlTableModel>
#include <QSqlQuery>

class ProductItem;
class QMenu;
class QTreeWidgetItem;

namespace Ui {
class ProductManagerForm;
}

class ProductManagerForm : public QWidget
{
    Q_OBJECT

public:
    explicit ProductManagerForm(QWidget *parent = nullptr);     /*Productmanager 생성자*/
    ~ProductManagerForm();                                      /*Productmanager 소멸자*/
    void loadData();                                            /*상품 데이터 불러오는 함수*/

private slots:
    /* QTreeWidget을 위한 슬롯 */
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);  /*트리위젯의 상품 정보 클릭시 동작하는 함수*/
    void showContextMenu(const QPoint &);                       /*마우스 오른족 버튼 클릭시 위치에 해당하는 정보를 제공하는 함수*/
    void showContextMenu_Table(const QPoint &);                       /*마우스 오른족 버튼 클릭시 위치에 해당하는 정보를 제공하는 함수*/
    void removeItem();                                          /*QAction을 위한 슬롯,  상품 삭제 함수*/
    void on_addPushButton_clicked();                            /*상품 추가 함수*/
    void on_modifyPushButton_clicked();                         /*상품 정보 변경 함수*/
    void on_searchPushButton_clicked();                         /*검색 버튼 클릭시 입력한 정보로 검색한 결과를 나타내주는 함수*/
    void SearchProductInfo(QString productname);                /*Shopmanagerform에서 보내준 상품명을 통해 해당 상품의
                                                                   정보를 보내주는 시그널 함수*/

    void on_tableView_clicked(const QModelIndex &index);

signals:
    void productAdded(int, QString);                            /*Shopmanagerform의 상품 정보 콤보박스에 추가되는 사항 전달*/
    void productModified(QString, int);                         /*Shopmanagerform의 상품 정보 콤보박스에 변경되는 사항 전달*/
    void productremoved(int);                                   /*Shopmanagerform의 상품 정보 콤보박스에 삭제되는 사항 전달*/
    void ProductInfoSended(ProductItem*);                       /*Shopmanagerform에서 상품명을 받아와 해당 ID의 상품 정보를 전달*/

private:
    int makeId();                                               /*고객 ID를 자동생성해주는 함수*/
    QMap<int, ProductItem*> productList;                        /*상품 정보들을 저장하는 리스트*/
    Ui::ProductManagerForm *ui;                                 /*ProductManagerForm의 UI*/
    QMenu* menu;

    bool createConnection();                                    /*Product 테이블 생성을 위한 함수*/
    QSqlTableModel *productModel;
    QSqlQuery* product_query;
};

#endif // PRODUCTMANAGERFORM_H
