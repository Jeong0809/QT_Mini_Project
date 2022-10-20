#include "shopitem.h"

ShopItem::ShopItem(int ID, QString Date, QString CustomerInfo, QString ProductInfo,
                   int Quantity, QString Address, int Price, int TotalPrice)
{
    setText(0, QString::number(ID));
    setText(1, Date);
    setText(2, CustomerInfo);
    setText(3, ProductInfo);
    setText(4, QString::number(Quantity));
    setText(5, Address);
    setText(6, QString::number(Price));
    setText(7, QString::number(TotalPrice));
}

int ShopItem::ID() const
{
    return text(0).toInt();
}

QString ShopItem::getDate() const
{
    return text(1);
}

void ShopItem::setDate(QString& Date)
{
    setText(1, Date);
}

QString ShopItem::getCustomerInfo() const
{
    return text(2);
}

void ShopItem::setCustomerInfo(QString& CustomerInfo)
{
    setText(2, CustomerInfo);
}

QString ShopItem::getProductInfo() const
{
    return text(3);
}

void ShopItem::setProductInfo(QString& ProductInfo)
{
    setText(3, ProductInfo);
}

int ShopItem::getQuantity() const
{
    return text(4).toInt();
}

void ShopItem::setQuantity(int& Quantity)
{
    setText(4, QString::number(Quantity));
}

QString ShopItem::getAddress() const
{
    return text(5);
}

void ShopItem::setAddress(QString& Address)
{
    setText(5, Address);
}

int ShopItem::getPrice() const
{
    return text(6).toInt();
}

void ShopItem::setPrice(int& Price)
{
    setText(6, QString::number(Price));
}

int ShopItem::getTotalPrice() const
{
    return text(7).toInt();
}

void ShopItem::setTotalPrice(int& TotalPrice)
{
    setText(7, QString::number(TotalPrice));
}
