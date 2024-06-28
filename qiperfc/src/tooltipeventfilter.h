#ifndef TOOLTIPEVENTFILTER_H
#define TOOLTIPEVENTFILTER_H

#include <QObject>
#include <QTreeView>
#include <QToolTip>
#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>

#include <QDebug>

class TooltipEventFilter : public QObject
{
    Q_OBJECT
public:
    TooltipEventFilter(QTreeView* view);// : QObject(view), view(view); {}
signals:
    void doCopy();
    void doPaste();
    void doDelete();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QTreeView* view;
};

#endif // TOOLTIPEVENTFILTER_H
