#ifndef PAGESWITCHERBASE_H
#define PAGESWITCHERBASE_H

#include <QDeclarativeItem>
#include <QtDeclarative>

class PageSwitcherProps : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isCurrentItem READ isCurrentItem WRITE setIsCurrentItem NOTIFY isCurrentItemChanged)

public:
    PageSwitcherProps(QObject *parent = 0)
        : QObject(parent), m_isCurrentItem(true)
    {
    }

    bool isCurrentItem() const { return m_isCurrentItem; }

public slots:
    void setIsCurrentItem(bool v)
    {
        if (m_isCurrentItem == v)
            return;
        m_isCurrentItem = v;
        emit isCurrentItemChanged(v);
    }


signals:
    void isCurrentItemChanged(bool isCurrentItem);

private:
    bool m_isCurrentItem;
};

class PageSwitcherBase : public QDeclarativeItem
{
    Q_OBJECT

public:
    PageSwitcherBase(QDeclarativeItem *parent = 0)
        : QDeclarativeItem(parent)
    {
    }

    /* Workaround for a QML bug */
    Q_INVOKABLE void setAttachedProperty(QObject *object, const QString &property, const QVariant &value)
    {
        qmlAttachedPropertiesObject<PageSwitcherBase>(object)->setProperty(qPrintable(property), value);
    }

    static PageSwitcherProps *qmlAttachedProperties(QObject *object)
    {
        return new PageSwitcherProps(object);
    }
};

QML_DECLARE_TYPEINFO(PageSwitcherBase, QML_HAS_ATTACHED_PROPERTIES)

#endif // PAGESWITCHERBASE_H
