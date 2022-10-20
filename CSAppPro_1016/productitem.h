#ifndef PRODUCTITEM_H
#define PRODUCTITEM_H

#include <QTreeWidgetItem>

class ProductItem : public QTreeWidgetItem
{
public:
    explicit ProductItem(int ID, QString ProductName, int Price, QString Category);

    int ID() const;                                                     //상품 ID를 반환하는 getter
    QString getProductName() const;                                      //상품명을 반환하는 getter
    void setProductName(QString&);                                       //상품명을 입력받는 setter
    int getPrice() const;                                               //상품 가격을 반환하는 getter
    void setPrice(int&);                                                //상품 가격을 입력받는 setter
    QString getCategory() const;                                         //상품 카테고리를 반환하는 getter
    void setCategory(QString&);                                          //상품 카테고리를 입력받는 setter

};

#endif // PRODUCTITEM_H
