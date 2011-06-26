#include "TopLevelWindow.h"

TopLevelWindow::TopLevelWindow(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
      m_window(0),
      m_rootItem(0),
      m_isResizing(false)
{
    static qreal sceneOffset = 0;

    sceneOffset += 20000;
    m_sceneOffset = sceneOffset;
}

void TopLevelWindow::setOpen(bool open)
{
    if (open == isOpen())
        return;

    if (open)
    {
        if (!scene())
        {
            Q_ASSERT(false);
            return;
        }

        m_window = new PopoutWindow;
        m_window->setScene(scene());
        QSize itemSize;
        if (m_rootItem)
            itemSize = QSize(m_rootItem->width(), m_rootItem->height());
        m_window->setSceneRect(m_sceneOffset, m_sceneOffset, itemSize.width(), itemSize.height());
        if (!itemSize.isEmpty())
            m_window->resize(itemSize);
        m_window->show();

        connect(m_window, SIGNAL(widthChanged(int)), SLOT(windowSizeChanged()));
        connect(m_window, SIGNAL(heightChanged(int)), SLOT(windowSizeChanged()));
    }
    else
    {
        m_window->close();
        m_window->deleteLater();
        m_window = 0;
    }

    Q_ASSERT(isOpen() == open);
    emit openChanged();
}

void TopLevelWindow::setRootItem(QDeclarativeItem *item)
{
    if (m_rootItem == item)
        return;

    m_rootItem = item;
    m_rootItem->setParentItem(this);
    m_rootItem->setX(m_sceneOffset);
    m_rootItem->setY(m_sceneOffset);

    connect(m_rootItem, SIGNAL(widthChanged()), SLOT(itemSizeChanged()));
    connect(m_rootItem, SIGNAL(heightChanged()), SLOT(itemSizeChanged()));
}

void TopLevelWindow::windowSizeChanged()
{
    if (m_isResizing) return;
    m_isResizing = true;
    m_rootItem->setWidth(m_window->width());
    m_rootItem->setHeight(m_window->height());
    m_isResizing = false;
}

void TopLevelWindow::itemSizeChanged()
{
    if (m_isResizing) return;
    m_isResizing = true;
    m_window->resize(m_rootItem->width(), m_rootItem->height());
    m_isResizing = false;
}
