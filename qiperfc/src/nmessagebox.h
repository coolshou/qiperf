#ifndef NMESSAGEBOX_H
#define NMESSAGEBOX_H

#include <QMessageBox>
#include <QWidget>
#include <QIcon>

class NMessageBox : public QMessageBox
{
public:
    NMessageBox(Icon icon, const QString &title, const QString &text,
                bool bYesNo=false, QWidget *parent=nullptr);
};

#endif // NMESSAGEBOX_H
