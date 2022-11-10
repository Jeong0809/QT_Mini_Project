#include "shopitem.h"

ShopItem::ShopItem(int ID, QString Date, QString CustomerInfo, QString ProductInfo,
                   int Quantity, QString Address, int Price, int TotalPrice)
{
    setText(0, QString::number(ID));        //첫 번째 열에는 주문번호 입력
    setText(1, Date);                       //두 번째 열에는 주문날짜 입력
    setText(2, CustomerInfo);               //세 번째 열에는 고객정보 입력
    setText(3, ProductInfo);                //네 번째 열에는 상품정보 입력
    setText(4, QString::number(Quantity));  //다섯 번째 열에는 수량 입력
    setText(5, Address);                    //여섯 번째 열에는 주소 입력
    setText(6, QString::number(Price));     //일곱 번째 열에는 상품 가격 입력
    setText(7, QString::number(TotalPrice));    //여덟 번째 열에는 총 가격 입력
}

//주문 번호를 불러오는 getter
int ShopItem::ID() const
{
    return text(0).toInt();
}

//주문 날짜를 불러오는 getter
QString ShopItem::getDate() const
{
    return text(1);
}

//주문 날짜를 설정하는 setter
void ShopItem::setDate(QString& Date)
{
    setText(1, Date);
}

//고객 정보를 불러오는 getter
QString ShopItem::getCustomerInfo() const
{
    return text(2);
}

//고객 정보를 설정하는 setter
void ShopItem::setCustomerInfo(QString& CustomerInfo)
{
    setText(2, CustomerInfo);
}

//상품 정보를 불러오는 getter
QString ShopItem::getProductInfo() const
{
    return text(3);
}

//상품 정보를 설정하는 setter
void ShopItem::setProductInfo(QString& ProductInfo)
{
    setText(3, ProductInfo);
}

//구매 수량을 불러오는 getter
int ShopItem::getQuantity() const
{
    return text(4).toInt();
}

//구매 수량을 설정하는 setter
void ShopItem::setQuantity(int& Quantity)
{
    setText(4, QString::number(Quantity));
}

//주소를 불러오는 getter
QString ShopItem::getAddress() const
{
    return text(5);
}

//주소를 설정하는 setter
void ShopItem::setAddress(QString& Address)
{
    setText(5, Address);
}

//가격을 불러오는 getter
int ShopItem::getPrice() const
{
    return text(6).toInt();
}

//가격을 설정하는 setter
void ShopItem::setPrice(int& Price)
{
    setText(6, QString::number(Price));
}

//총 가격을 불러오는 getter
int ShopItem::getTotalPrice() const
{
    return text(7).toInt();
}

//총 가격을 설정하는 setter
void ShopItem::setTotalPrice(int& TotalPrice)
{
    setText(7, QString::number(TotalPrice));
}
