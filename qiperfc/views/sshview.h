#ifndef SSHVIEW_H
#define SSHVIEW_H

#include <QWidget>
#include "../views/abstractview.h"

namespace Ui {
class SSHView;
}

class SSHView : public AbstractView
{
    Q_OBJECT

public:
    explicit SSHView(QWidget *parent = nullptr);
    ~SSHView();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SSHView *ui;
};

#endif // SSHVIEW_H
