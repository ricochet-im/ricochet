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

    explicit ChatWidget(ContactUser *user, QWidget *parent = 0);

private slots:
	void sendInputMessage();

private:
	class QTextEdit *textArea;
	class QLineEdit *textInput;

	void createTextArea();
	void createTextInput();

	void appendChatMessage(const QDateTime &when, ContactUser *user, const QString &text);
	void scrollToBottom();
};

#endif // CHATWIDGET_H
