//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>

QHBoxLayout *getLayoutUnitless(QWidget *parent, const char *prefix, QWidget *widget)
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(new QLabel(prefix, parent));
    layout->addWidget(widget);
    return layout;
}