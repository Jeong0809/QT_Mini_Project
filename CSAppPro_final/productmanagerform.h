#ifndef PRODUCTMANAGERFORM_H
#define PRODUCTMANAGERFORM_H

#include <QWidget>
#include <QHash>
#include <QTableView>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QStandardItemModel>

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
    void showContextMenu(const QPoint &);                       /*마우스 오른족 버튼 클릭시 위치에 해당하는 정보를 제공하는 함수*/
    void removeItem();                                          /*QAction을 위한 슬롯,  상품 삭제 함수*/
    void on_addPushButton_clicked();                            /*상품 추가 함수*/
    void on_modifyPushButton_clicked();                         /*상품 정보 변경 함수*/
    void on_searchPushButton_clicked();                         /*검색 버튼 클릭시 입력한 정보로 검색한 결과를 나타내주는 함수*/
    void SearchProductInfo(QString productname);                /*Shopmanagerform에서 보내준 상품명을 통해 해당 상품의
                                                                   정보를 보내주는 시그널 함수*/
    void on_tableView_clicked(const QModelIndex &index);        /*테이블 뷰 클릭 시 입력란에 보여질 수 있도록 하는 함수*/
    void SendQuantity(int quantity, QString productname);       /*주문 내역을 통해 변경된 수량을 조절하는 함수*/

signals:
    void productAdded(int, QString);                            /*Shopmanagerform의 상품 정보 콤보박스에 추가되는 사항 전달*/
    void productModified(QString, int);                         /*Shopmanagerform의 상품 정보 콤보박스에 변경되는 사항 전달*/
    void productremoved(int);                                   /*Shopmanagerform의 상품 정보 콤보박스에 삭제되는 사항 전달*/
    void ProductInfoSended(QStringList);                        /*Shopmanagerform에서 상품명을 받아와 해당 ID의 상품 정보를 전달*/
    void quantityInformed(bool);                                /*Shopmanagerform에서 주문한 구매 수량이 재고 수량보다 적은지 확인*/

private:
    int makeId();                                               /*상품 ID를 자동 생성해주는 함수*/
    Ui::ProductManagerForm *ui;                                 /*ProductManagerForm의 UI*/
    QMenu* menu;
    QSqlTableModel *productModel;                               /*상품 데이터를 저장하는 데이터 모델*/
    QSqlQuery* product_query;                                   /*product SQL 쿼리*/
    QStandardItemModel* searchModel;                            /*검색 데이터를 임시로 저장하는 데이터 모델*/
};

#endif // PRODUCTMANAGERFORM_H
