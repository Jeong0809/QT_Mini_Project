#ifndef SHOPITEM_H
#define SHOPITEM_H

#include <QTreeWidgetItem>

class ShopItem : public QTreeWidgetItem
{
public:
    explicit ShopItem(int ID, QString Date, QString CustomerInfo, QString ProductInfo,
                      int Quantity, QString Address, int Price, int TotalPrice);

    int ID() const;

    QString getDate() const;
    void setDate(QString&);

    QString getCustomerInfo() const;
    void setCustomerInfo(QString&);

    QString getProductInfo() const;
    void setProductInfo(QString&);

    int getQuantity() const;
    void setQuantity(int&);

    QString getAddress() const;
    void setAddress(QString&);

    int getPrice() const;
    void setPrice(int&);

    int getTotalPrice() const;
    void setTotalPrice(int&);

};

#endif // SHOPITEM_H
