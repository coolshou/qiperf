#include "tpdirdelegate.h"
#include "qpainter.h"
#include "tp.h"
TPDirDelegate::TPDirDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{

}

void TPDirDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //TODO: when column is selected, the bg color did not fit to selected
    if (index.isValid() && index.column() == TP::cols::dir) { // Assuming you want to display images in column::dir
//        QVariant data = index.data(Qt::UserRole); // Retrieve data associated with the index
        QVariant data = index.data(); // Retrieve data associated with the index
        if (data.isValid()) {
            QString imagePath = ":/"+data.toString();// Assumes you store the image path in Qt::UserRole
            QImage image(imagePath);
            if (!image.isNull()) {
                QPixmap pixmap = QPixmap::fromImage(image);
                painter->drawPixmap(option.rect, pixmap);
            }else{
//                qDebug() << "No image: " << imagePath << ::Qt::endl;
            }
        } else {
            qDebug() << "data.isValid: " << data << ::Qt::endl;
        }
    }
    else {
        QStyledItemDelegate::paint(painter, option, index); //this will draw text too
    }
}
