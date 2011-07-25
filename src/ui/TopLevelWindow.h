#ifndef TOPLEVELWINDOW_H
#define TOPLEVELWINDOW_H

#include <QDeclarativeItem>
#include "PopoutManager.h"

class TopLevelWindow : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(bool open READ isOpen WRITE setOpen NOTIFY openChanged)
    Q_PROPERTY(QDeclarativeItem* rootItem READ rootItem WRITE setRootItem NOTIFY rootItemChanged)
    Q_CLASSINFO("DefaultProperty", "rootItem")

    Q_PROPERTY(bool sheet READ isSheet WRITE setSheet)

public:
    explicit TopLevelWindow(QDeclarativeItem *parent = 0);

    bool isOpen() const { return m_window; }
    bool isSheet() const { return m_winFlags & Qt::Sheet; }
    QDeclarativeItem *rootItem() const { return m_rootItem; }
    PopoutWindow *window() const { return m_window; }

public slots:
    void setOpen(bool open);
    void setSheet(bool sheet);
    void setRootItem(QDeclarativeItem *rootItem);
    void open() { setOpen(true); }
    void show() { setOpen(true); }
    void close() { setOpen(false); }

signals:
    void openChanged();
    void opened();
    void closed();
    void rootItemChanged();

private slots:
    void windowSizeChanged();
    void itemSizeChanged();

private:
    PopoutWindow *m_window;
    QDeclarativeItem *m_rootItem;
    qreal m_sceneOffset;
    bool m_isResizing;
    Qt::WindowFlags m_winFlags;
};

#endif // TOPLEVELWINDOW_H
