#ifndef HOMESCREEN_H
#define HOMESCREEN_H

#include <QWidget>

class HomeScreen : public QWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(HomeScreen)

public:
	explicit HomeScreen(QWidget *parent = 0);

private slots:
	void updateTorStatus();

	void startTorConfig();

private:
	class QLabel *avatar;

	QList<QAction*> buttonActions;
	class QAction *actAddContact, *actChangeAvatar, *actOpenDownloads, *actTestConnection, *actOptions;
	class QAction *actTorConfig;

	QLabel *torStatus, *torInfo;

	void createAvatar();
	void createActions();
	class QLayout *createButtons();
	class QLayout *createStatus();
};

#endif // HOMESCREEN_H
