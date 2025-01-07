#ifndef SHOWCUSTOMTOOLTIP_H
#define SHOWCUSTOMTOOLTIP_H

#include <QWidget>
#include <QToolTip>
#include <QString>
#include <QPoint>

// Function to show a tooltip
void showCustomToolTip(QWidget *widget, const QString &message, int delay=10*1000) {
    QPoint globalPos = widget->mapToGlobal(widget->rect().center());
    QToolTip::showText(globalPos, message, widget, {}, delay);
}
#endif // SHOWCUSTOMTOOLTIP_H
