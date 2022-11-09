#ifndef SHOPITEM_H
#define SHOPITEM_H

#include <QTreeWidgetItem>

class ShopItem : public QTreeWidgetItem
{
public:
    //ShopItem의 생성자
    explicit ShopItem(int ID, QString Date, QString CustomerInfo, QString ProductInfo,
                      int Quantity, QString Address, int Price, int TotalPrice);

    int ID() const;                     //주문 번호를 불러오는 getter

    QString getDate() const;            //주문 날짜를 불러오는 getter
    void setDate(QString&);             //주문 날짜를 설정하는 setter

    QString getCustomerInfo() const;    //고객 정보를 불러오는 getter
    void setCustomerInfo(QString&);     //고객 정보를 설정하는 setter

    QString getProductInfo() const;     //상품 정보를 불러오는 getter
    void setProductInfo(QString&);      //상품 정보를 설정하는 setter

    int getQuantity() const;            //구매 수량을 불러오는 getter
    void setQuantity(int&);             //구매 수량을 설정하는 setter

    QString getAddress() const;         //주소를 불러오는 getter
    void setAddress(QString&);          //주소를 설정하는 setter

    int getPrice() const;               //가격을 불러오는 getter
    void setPrice(int&);                //가격을 설정하는 setter

    int getTotalPrice() const;          //총 가격을 불러오는 getter
    void setTotalPrice(int&);           //총 가격을 설정하는 setter
};

#endif // SHOPITEM_H
