#ifndef CLIENTITEM_H
#define CLIENTITEM_H

#include <QTreeWidgetItem>

class ClientItem : public QTreeWidgetItem
{
public:
    /*고객 데이터 생성자*/
    explicit ClientItem(int id = 0, QString = "", QString = "", QString = "");

    QString getName() const;        //고객명을 불러오는 getter
    void setName(QString&);         //고객명을 설정하는 setter

    QString getPhoneNumber() const; //휴대폰 번호를 불러오는 getter
    void setPhoneNumber(QString&);  //휴대폰 번호를 설정하는 setter

    QString getAddress() const;     //고객 주소를 불러오는 getter
    void setAddress(QString&);      //고객 주소를 설정하는 setter

    int ID() const;                 //고객 ID를 불러오는 getter
};

#endif // CLIENTITEM_H
