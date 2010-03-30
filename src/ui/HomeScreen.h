#ifndef HOMESCREEN_H
#define HOMESCREEN_H

#include <QWidget>

class HomeScreen : public QWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(HomeScreen)

public:
	explicit HomeScreen(QWidget *parent = 0);

private:
	class QLabel *avatar;

	void createAvatar();
	class QLayout *createButtons();
};

#endif // HOMESCREEN_H
