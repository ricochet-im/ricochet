#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>

class ChatWidget : public QWidget
{
Q_OBJECT
public:
    explicit ChatWidget(QWidget *parent = 0);

private:
	class QTextEdit *textArea;
	class QLineEdit *textInput;

	void createTextArea();
	void createTextInput();
};

#endif // CHATWIDGET_H
