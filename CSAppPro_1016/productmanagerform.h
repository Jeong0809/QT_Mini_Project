#ifndef PRODUCTMANAGERFORM_H
#define PRODUCTMANAGERFORM_H

#include <QWidget>
#include <QHash>

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
    explicit ProductManagerForm(QWidget *parent = nullptr);
    ~ProductManagerForm();
    void loadData();

public slots:
    void SearchProductInfo(QString productname);

private slots:
    /* QTreeWidget을 위한 슬롯 */
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void showContextMenu(const QPoint &);
    void removeItem();              /* QAction을 위한 슬롯 */
    void on_addPushButton_clicked();
    void on_modifyPushButton_clicked();
    void on_searchPushButton_clicked();

signals:
    void productAdded(int, QString);
    void productModified(QString, int);
    void productremoved(int);
    void ProductInfoSended(ProductItem*);

private:
    Ui::ProductManagerForm *ui;    
    QMap<int, ProductItem*> productList;
    QMenu* menu;
    int makeId();
};

#endif // PRODUCTMANAGERFORM_H
