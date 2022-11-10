#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ClientManagerForm;
class ProductManagerForm;
class ShoplistManagerForm;
class ChatClientForm;
class ChatServerForm;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);  /*MainWindow의 생성자*/
    ~MainWindow();                          /*MainWindow의 소멸자*/

private slots:
    void on_actionClient_triggered();   /*Chat 버튼 클릭시 동작되는 함수*/
    void on_actionChat_triggered();     /*Client 버튼 클릭시 동작되는 함수*/

private slots:
    void on_actionProduct_triggered();

    void on_actionShoplist_triggered();

private:
    Ui::MainWindow *ui;                 /*MainWindow의 UI*/
    ClientManagerForm *clientForm;      /*ClientManagerForm의 객체 생성*/
    ProductManagerForm *productForm;    /*ProductManagerForm의 객체 생성*/
    ShoplistManagerForm* orderForm;     /*ShoplistManagerForm의 객체 생성*/
    ChatClientForm* chatClientForm;     /*ChatClientForm의 객체 생성*/
    ChatServerForm* chatServerForm;     /*ChatServerForm의 객체 생성*/
};
#endif // MAINWINDOW_H
