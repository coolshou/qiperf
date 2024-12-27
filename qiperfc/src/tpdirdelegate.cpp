#include "tpdirdelegate.h"
#include "qpainter.h"
#include "tp.h"
TPDirDelegate::TPDirDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{

}

void TPDirDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid() && index.column() == TP::cols::dir) {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        // Custom painting for the background and selection highlights
        painter->save();

        // Draw background
        if (opt.state & QStyle::State_Selected) {
            painter->fillRect(opt.rect, opt.palette.highlight());
        } else {
            painter->fillRect(opt.rect, opt.palette.base());
        }

        // Draw custom decorations (e.g., icons, borders, etc.)
        if (!opt.icon.isNull()) {
            QRect iconRect = QStyle::alignedRect(
                opt.direction,
                Qt::AlignLeft,
                opt.icon.actualSize(opt.decorationSize),
                opt.rect
                );
            opt.icon.paint(painter, iconRect, Qt::AlignLeft);
        }
        QVariant data = index.data(); // Retrieve data associated with the index
        if (data.isValid()) {
            QString imagePath = ":/"+data.toString();// Assumes you store the image path in Qt::UserRole
            // qDebug() << "imagePath:" << imagePath;
            QImage image(imagePath);
            if (!image.isNull()) {
                QPixmap pixmap = QPixmap::fromImage(image);
                QRect rect = option.rect;
                // rect.setWidth((option.rect.width()/3)*2); // adjust size
                painter->drawPixmap(rect, pixmap);
            }
        }
        painter->restore();

        if (0){
            // const QModelIndex itm;// = index;
            // // QModelIndex itm = index;
            // // QMap<int, QVariant> rolesData;
            // // rolesData[Qt::DisplayRole] = "";
            // // itm.model()->setItemData(itm, rolesData);
            // // qDebug() << "option:" << option.rect << " text:" << option.text;
            // qDebug() << " text:" << index.data(Qt::DisplayRole) ;//<< " new:" << itm.data(Qt::DisplayRole);
            // QStyledItemDelegate::paint(painter, option, itm); //this will draw all include text string
            // Assuming you want to display images in column::dir
            QVariant data = index.data(); // Retrieve data associated with the index
            if (data.isValid()) {
                QString imagePath = ":/"+data.toString();// Assumes you store the image path in Qt::UserRole
                QImage image(imagePath);
                if (!image.isNull()) {
                    QPixmap pixmap = QPixmap::fromImage(image);
                    QRect rect = option.rect;
                    rect.setWidth(option.rect.width()/2);
                    painter->drawPixmap(rect, pixmap);
                    // painter->drawPixmap(option.rect, pixmap);
                }else{
                    //                qDebug() << "No image: " << imagePath
                }
            } else {
                qDebug() << "data.not Valid: " << data;
            }
        }
    }
    else {
        QStyledItemDelegate::paint(painter, option, index); //this is original paint, it will draw all include text
    }
}
