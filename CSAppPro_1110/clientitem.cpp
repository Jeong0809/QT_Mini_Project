#include "clientitem.h"

ClientItem::ClientItem(int id, QString name, QString phoneNumber, QString address)
{
    //setText(0, QString::number(id));    //첫 번째 열에는 ID 입력
    setText(0, QString::number(id).rightJustified(5, '0'));
    setText(1, name);                   //두 번째 열에는 이름 입력
    setText(2, phoneNumber);            //세 번째 열에는 휴대폰 번호 입력
    setText(3, address);                //네 번째 열에는 주소 입력
}

//고객명을 불러오는 getter
QString ClientItem::getName() const
{
    return text(1);
}

//고객명을 설정하는 setter
void ClientItem::setName(QString& name)
{
    setText(1, name);
}

//휴대폰 번호를 불러오는 getter
QString ClientItem::getPhoneNumber() const
{
    return text(2);
}

//휴대폰 번호를 설정하는 setter
void ClientItem::setPhoneNumber(QString& phoneNumber)
{
    setText(2, phoneNumber);    // c_str() --> const char*
}

//고객 주소를 불러오는 getter
QString ClientItem::getAddress() const
{
    return text(3);
}

//고객 주소를 설정하는 setter
void ClientItem::setAddress(QString& address)
{
    setText(3, address);
}

//고객 ID를 불러오는 getter
int ClientItem::ID() const
{
    return text(0).toInt();
}
