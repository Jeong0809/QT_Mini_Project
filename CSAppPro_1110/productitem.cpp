#include "productitem.h"

ProductItem::ProductItem(int ID, QString ProductName, int Price, QString Category)
{
    setText(0, QString::number(ID));    //첫 번째 열에는 ID 입력
    setText(1, ProductName);            //두 번째 열에는 상품명 입력
    setText(2, QString::number(Price)); //세 번째 열에는 상품가격 입력
    setText(3, Category);               //네 번째 열에는 카테고리 입력
}

//상품 ID를 불러오는 getter
int ProductItem::ID() const
{
    return text(0).toInt();
}

//상품명을 불러오는 getter
QString ProductItem::getProductName() const
{
    return text(1);
}

//상품명을 설정하는 setter
void ProductItem::setProductName(QString& ProductName)
{
    setText(1, ProductName);
}

//상품 가격을 불러오는 getter
int ProductItem::getPrice() const
{
    return text(2).toInt();
}

//상품 가격을 설정하는 setter
void ProductItem::setPrice(int& Price)
{
    setText(2, QString::number(Price)); //int -> QString
}

//상품 카테고리를 불러오는 getter
QString ProductItem::getCategory() const
{
    return text(3);
}

//상품 카테고리를 설정하는 setter
void ProductItem::setCategory(QString& Category)
{
    setText(3, Category);
}
