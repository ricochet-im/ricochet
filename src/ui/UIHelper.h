#ifndef UIHELPER_H
#define UIHELPER_H

#include <QObject>
#include <QGraphicsProxyWidget>
#include <QDeclarativeItem>

class QTextEdit;
class QDeclarativeItem;

class UIHelper : public QObject
{
    Q_OBJECT

public:
    explicit UIHelper(QObject *parent = 0);

    Q_INVOKABLE QTextEdit *createTextEdit(QDeclarativeItem *proxyItem);
};

class DeclarativeProxiedProxyWidget : public QGraphicsProxyWidget
{
    Q_OBJECT

public:
    DeclarativeProxiedProxyWidget(QDeclarativeItem *proxyItem, QWidget *widget)
        : QGraphicsProxyWidget(proxyItem), proxyItem(proxyItem)
    {
        setWidget(widget);
        updateWidgetGeometry();

        connect(proxyItem, SIGNAL(widthChanged()), SLOT(updateWidgetGeometry()));
        connect(proxyItem, SIGNAL(heightChanged()), SLOT(updateWidgetGeometry()));
    }

public slots:
    void updateWidgetGeometry()
    {
        setGeometry(QRectF(0, 0, proxyItem->width(), proxyItem->height()));
    }

private:
    QDeclarativeItem *proxyItem;
};

#endif // UIHELPER_H
