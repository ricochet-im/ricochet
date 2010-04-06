#ifndef HOMECONTACTWIDGET_H
#define HOMECONTACTWIDGET_H

#include <QWidget>

class HomeContactWidget : public QWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(HomeContactWidget)

public:
	explicit HomeContactWidget(QWidget *parent = 0);

	bool isSelected() const { return pSelected; }

	virtual QSize sizeHint() const;

public slots:
	void setSelected(bool selected = true);
	void clearSelected() { setSelected(false); }

signals:
	void selectionChanged(bool selected);
	void selected();
	void deselected();

protected:
	virtual void paintEvent(QPaintEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void enterEvent(QEvent *event);
	virtual void leaveEvent(QEvent *event);

private:
	bool pSelected;
};

#endif // HOMECONTACTWIDGET_H
