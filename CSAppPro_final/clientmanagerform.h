#ifndef CLIENTMANAGERFORM_H
#define CLIENTMANAGERFORM_H

#include <QWidget>
#include <QHash>
#include <QTableView>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QStandardItemModel>

class ClientItem;
class QMenu;
class QTreeWidgetItem;

namespace Ui {
class ClientManagerForm;
}

class ClientManagerForm : public QWidget
{
    Q_OBJECT

public:
    explicit ClientManagerForm(QWidget *parent = nullptr);       /*Clientmanager 생성자*/
    ~ClientManagerForm();                                        /*Clientmanager 소멸자*/
    void loadData();                                             /*고객 데이터 불러오는 함수*/

private slots:
    /* QTreeWidget을 위한 슬롯 */
    void showContextMenu(const QPoint &);                        /*마우스 오른족 버튼 클릭시 위치에 해당하는 정보를 제공하는 함수*/
    void on_tableView_clicked(const QModelIndex &index);         /*테이블 뷰 클릭 시 입력란에 보여질 수 있도록 하는 함수*/

    void removeItem();                                           /*QAction을 위한 슬롯,  고객 삭제 함수*/
    void on_addPushButton_clicked();                             /*고객 추가 함수*/
    void on_modifyPushButton_clicked();                          /*고객 정보 변경 함수*/
    void on_searchPushButton_clicked();                          /*검색 버튼 클릭시 입력한 정보로 검색한 결과를 나타내주는 함수*/
    void SearchCustomerInfo(int);                                /*Shopmanagerform에서 보내준 아이디를 통해 해당 고객의
                                                                   정보를 보내주는 시그널 함수*/
signals:
    void clientAdded(int, QString);                              /*Shopmanagerform의 고객 정보 콤보박스에 추가되는 사항 전달*/
    void clientModified(int, QString, int);                      /*Shopmanagerform의 고객 정보 콤보박스에 변경되는 사항 전달*/
    void clientremoved(int);                                     /*Shopmanagerform의 고객 정보 콤보박스에 삭제되는 사항 전달*/
    void CustomerInfoSended(QStringList);                        /*Shopmanagerform에서 ID를 받아와 해당 ID의 고객 정보를 전달*/

private:
    int makeId();                                                /*고객 ID를 자동생성해주는 함수*/
    Ui::ClientManagerForm *ui;
    QMenu* menu;
    QSqlTableModel *clientModel;                                 /*고객 데이터를 저장하는 데이터 모델*/
    QSqlQuery* client_query;                                     /*Client SQL 쿼리*/
    QStandardItemModel* searchModel;                             /*검색 데이터를 임시로 저장하는 데이터 모델*/
};
#endif // CLIENTMANAGERFORM_H
