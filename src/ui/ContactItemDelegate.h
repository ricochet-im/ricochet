#ifndef CONTACTITEMDELEGATE_H
#define CONTACTITEMDELEGATE_H

#include <QStyledItemDelegate>

class ContactItemDelegate : public QStyledItemDelegate
{
Q_OBJECT
public:
    explicit ContactItemDelegate(QObject *parent = 0);

	virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // CONTACTITEMDELEGATE_H
