#include "productitem.h"

ProductItem::ProductItem(int ID, QString ProductName, int Price, QString Category)
{
    setText(0, QString::number(ID));
    setText(1, ProductName);
    setText(2, QString::number(Price));
    setText(3, Category);
}

int ProductItem::ID() const
{
    return text(0).toInt();
}

QString ProductItem::getProductName() const
{
    return text(1);
}

void ProductItem::setProductName(QString& ProductName)
{
    setText(1, ProductName);
}

int ProductItem::getPrice() const
{
    return text(2).toInt();
}

void ProductItem::setPrice(int& Price)
{
    setText(2, QString::number(Price));
}

QString ProductItem::getCategory() const
{
    return text(3);
}

void ProductItem::setCategory(QString& Category)
{
    setText(3, Category);
}
