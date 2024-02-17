#ifndef FORMENDPOINTS_H
#define FORMENDPOINTS_H

#include <QWidget>

namespace Ui {
class FormEndPoints;
}

// class to show endpoint list
class FormEndPoints : public QWidget
{
    Q_OBJECT

public:
    explicit FormEndPoints(QWidget *parent = nullptr);
    ~FormEndPoints();

private:
    Ui::FormEndPoints *ui;
};

#endif // FORMENDPOINTS_H
