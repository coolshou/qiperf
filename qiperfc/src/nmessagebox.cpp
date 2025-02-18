#include "nmessagebox.h"
#include <QStyle>
#include <QApplication>

NMessageBox::NMessageBox(Icon icon, const QString &title, const QString &text, QWidget *parent)
    :QMessageBox(parent)
{
    setAttribute(Qt::WA_DeleteOnClose); //makes sure the msgbox is deleted automatically when closed
    setStandardButtons(QMessageBox::Ok);
    setWindowTitle(title);
    setText(text);
    setIcon(icon);
    setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
    setModal( false ); // if you want it non-modal
}
