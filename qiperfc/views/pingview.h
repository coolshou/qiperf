#ifndef PINGVIEW_H
#define PINGVIEW_H

#include <QWidget>
#include "abstractview.h"

namespace Ui {
class PingView;
}

class PingView : public AbstractView
{
    Q_OBJECT

public:
    explicit PingView(QWidget *parent = nullptr);
    ~PingView();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::PingView *ui;
};

#endif // PINGVIEW_H
