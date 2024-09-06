#include "tooltipeventfilter.h"

TooltipEventFilter::TooltipEventFilter(QTreeView *view)
    : QObject(view), view(view)
{

}

bool TooltipEventFilter::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseMove) {
        //tooltip
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        QModelIndex index = view->indexAt(mouseEvent->pos());
        if (index.isValid()) {
            QString data = index.data().toString();
            //                qDebug() << "pos:"<< mouseEvent->globalPos() <<" data" << data;
            QToolTip::showText(mouseEvent->globalPos(), data, view);
        } else {
            QToolTip::hideText();
        }
    }
    if(event->type() ==QEvent::KeyPress){
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        qDebug() << "keyEvent:" << keyEvent->key() ;

        if(keyEvent->key() == Qt::Key_C && keyEvent->modifiers().testFlag(Qt::ControlModifier)){ //ctrl+c
            emit doCopy();
        }
        if(keyEvent->key() == Qt::Key_C && keyEvent->modifiers().testFlag(Qt::ControlModifier)){  //ctrl+v
            emit doPaste();
        }
        if(keyEvent->key() == Qt::Key_Delete){       //del key
            qDebug() << "doDelete" ;
            emit doDelete();
        }
        // if(keyEvent->key() == Qt::Key_F5){       //F5
        //     emit doRefresh();
        // }
    }
    return QObject::eventFilter(obj, event);
}
