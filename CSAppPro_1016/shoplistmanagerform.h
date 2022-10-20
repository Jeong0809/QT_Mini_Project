#ifndef SHOPLISTMANAGERFORM_H
#define SHOPLISTMANAGERFORM_H

#include <QWidget>
#include <QHash>
#include <QCalendarWidget>
#include <QMessageBox>
#include "productitem.h"
#include "clientitem.h"

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
    explicit ShoplistManagerForm(QWidget *parent = nullptr);
    ~ShoplistManagerForm();
    void loadData();

public slots:
    void addClient(int, QString);
    void modifyClient(int, QString, int);
    void removeClient(int);

    void addProduct(int ProductID, QString productname);
    void modifyProduct(QString productname, int index);
    void removeProduct(int index);

    void SendCustomerInfo(ClientItem* c);
    void SendProductInfo(ProductItem* c);


private slots:
    /* QTreeWidget을 위한 슬롯 */
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void showContextMenu(const QPoint &);
    void removeItem();              /* QAction을 위한 슬롯 */
    void on_addPushButton_clicked();
    void on_modifyPushButton_clicked();
    void on_searchPushButton_clicked();
    void on_CustomerInfocomboBox_textActivated(const QString &arg1);

    void on_ProductInfocomboBox_textActivated(const QString &arg1);

signals:
    void CustomerInfoSearched(int);
    void ProductInfoSearched(QString);

private:
    Ui::ShoplistManagerForm *ui;
    QMap<int, ShopItem*> shopList;
    QMenu* menu;
    QDate* date;
    int makeId();
};

#endif // SHOPLISTMANAGERFORM_H
