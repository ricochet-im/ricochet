#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>

class QDateTime;
class ContactUser;

class ChatWidget : public QWidget
{
Q_OBJECT
public:
	ContactUser * const user;

	static ChatWidget *widgetForUser(ContactUser *user, bool create = true);

	~ChatWidget();

	void receiveMessage(const QDateTime &when, const QString &text);

	int unreadMessages() const { return pUnread; }

public slots:
	void clearUnreadMessages();

signals:
	void messageReceived();
	void unreadMessagesChanged(int unread);

private slots:
	void sendInputMessage();
	void messageReply();

	void scrollToBottom();

	void showOfflineNotice();
	void clearOfflineNotice();
	void clearOfflineNoticeInstantly();

protected:
	virtual bool event(QEvent *event);

private:
	static QHash<ContactUser*,ChatWidget*> userMap;

	class QTextEdit *textArea;
	class QLineEdit *textInput;
	class QWidget *offlineNotice;

	int pUnread;

	explicit ChatWidget(ContactUser *user);

	void createTextArea();
	void createTextInput();

	void addChatMessage(ContactUser *user, const QDateTime &when, const QString &text, int identifier = 0);
	bool findBlockIdentifier(int identifier, class QTextBlock &block);
};

#endif // CHATWIDGET_H
