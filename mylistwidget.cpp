#include "mylistwidget.h"

MyListWidget::MyListWidget(QWidget *parent)
    :QListWidget(parent)
{

}

MyListWidget::~MyListWidget()
{

}


void MyListWidget::Update(FindTerminal *r)
{
    const QMap<int, chat_host_t>& h = r->GetMap();
    QList<chat_host_t> hosts = h.values();
    this->clear();
    foreach (chat_host_t host, hosts) {
       this->addItem(host.hostname+"("+host.address+")");
    }
}
