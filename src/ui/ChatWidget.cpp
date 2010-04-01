#include "ChatWidget.h"
#include "core/ContactUser.h"
#include "protocol/ChatMessageCommand.h"
#include "ui/MainWindow.h"
#include <QBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QDateTime>
#include <QTextDocument>
#include <QTextBlock>
#include <QLabel>
#include <QScrollBar>
#include <QApplication>
#include <QPropertyAnimation>

QHash<ContactUser*,ChatWidget*> ChatWidget::userMap;

ChatWidget *ChatWidget::widgetForUser(ContactUser *u)
{
	ChatWidget *widget = userMap.value(u);
	if (!widget)
	{
		widget = new ChatWidget(u);
		userMap.insert(u, widget);
		uiMain->addChatWidget(widget);
	}

	return widget;
}

ChatWidget::ChatWidget(ContactUser *u)
	: user(u), offlineNotice(0)
{
	Q_ASSERT(user);
	connect(user, SIGNAL(connected()), this, SLOT(clearOfflineNotice()));
	connect(user, SIGNAL(disconnected()), this, SLOT(showOfflineNotice()));

	QBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);

	createTextArea();
	layout->addWidget(textArea);

	createTextInput();
//	layout->addWidget(textInput);

	QBoxLayout *testLayout = new QHBoxLayout;
	layout->addLayout(testLayout);

	testLayout->addStretch();

	layout->addWidget(textInput);

	if (!user->isConnected())
		showOfflineNotice();
}

ChatWidget::~ChatWidget()
{
	QHash<ContactUser*,ChatWidget*>::Iterator it = userMap.find(user);
	if (*it == this)
		userMap.erase(it);
}

void ChatWidget::createTextArea()
{
	textArea = new QTextEdit;
	textArea->setReadOnly(true);
	textArea->setFont(QFont("Calibri", 10));

	connect(textArea->verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(scrollToBottom()));
}

void ChatWidget::createTextInput()
{
	textInput = new QLineEdit;
	textInput->setFont(QFont("Calibri", 10));
	textInput->setMaxLength(512);

	connect(textInput, SIGNAL(returnPressed()), this, SLOT(sendInputMessage()));
}

void ChatWidget::sendInputMessage()
{
	QString text = textInput->text();
	if (text.isEmpty())
		return;
	textInput->clear();

	QDateTime when = QDateTime::currentDateTime();

	ChatMessageCommand *command = new ChatMessageCommand;
	connect(command, SIGNAL(commandFinished()), this, SLOT(messageReply()));
	command->send(user->conn(), when, text);

	addChatMessage(NULL, when, text, command->identifier());
}

void ChatWidget::receiveMessage(const QDateTime &when, const QString &text)
{
	addChatMessage(user, when, text);

	if (!isVisible() || !window()->isActiveWindow() || window()->isMinimized())
	{
		qApp->alert(window(), 0);
	}
}

void ChatWidget::messageReply()
{
	ChatMessageCommand *command = qobject_cast<ChatMessageCommand*>(sender());
	if (!command)
		return;

	QTextBlock block;
	if (findBlockIdentifier(command->identifier(), block))
	{
		block.setUserState(0);

		/* Loop through the fragments in this block, and see if any have alternate text colors.
		 * If they do, set that color. */
		QTextCursor cursor(textArea->document());
		for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it)
		{
			QTextFragment fragment = it.fragment();
			if (fragment.isValid())
			{
				QTextCharFormat format = fragment.charFormat();
				if (!format.hasProperty(QTextFormat::UserProperty))
					continue;

				format.setForeground(QColor(format.property(QTextFormat::UserProperty).value<QRgb>()));

				cursor.setPosition(fragment.position());
				cursor.setPosition(fragment.position() + fragment.length(), QTextCursor::KeepAnchor);
				cursor.setCharFormat(format);
			}
		}
	}
}

void ChatWidget::addChatMessage(ContactUser *from, const QDateTime &when, const QString &text, int identifier)
{
	QTextCursor cursor(textArea->document());
	cursor.movePosition(QTextCursor::End);

	if (!cursor.atBlockStart())
		cursor.insertBlock();

	if (identifier)
		cursor.block().setUserState(identifier);

	/* These are the colors used for the timestamp, nickname, and text. Each has two colors;
	 * the full color, and the 'faded' color for when the message hasn't been received yet. */
	static const QRgb tsColor[] = { qRgb(160, 160, 164), qRgb(200, 200, 202) };
	static const QRgb myNickColor[] = { qRgb(0, 94, 173), qRgb(135, 185, 227) };
	static const QRgb nickColor[] = { qRgb(174, 0, 0), qRgb(227, 135, 135) };
	static const QRgb textColor[] = { qRgb(0, 0, 0), qRgb(135, 135, 135) };
	
	bool light = (from == 0);

	/* Timestamp */
	QTextCharFormat tsFormat;
	tsFormat.setForeground(QColor(tsColor[light]));
	if (light)
		tsFormat.setProperty(QTextFormat::UserProperty, tsColor[!light]);

	cursor.insertText(when.time().toString(QString("(HH:mm:ss) ")), tsFormat);

	/* Nickname */
	QTextCharFormat nickFormat;
	nickFormat.setFontWeight(QFont::Bold);
	if (!from)
	{
		nickFormat.setForeground(QColor(myNickColor[light]));
		if (light)
			nickFormat.setProperty(QTextFormat::UserProperty, myNickColor[!light]);
	}
	else
	{
		nickFormat.setForeground(QColor(nickColor[light]));
		if (light)
			nickFormat.setProperty(QTextFormat::UserProperty, nickColor[!light]);
	}

	QString nickname = from ? from->nickname() : tr("Me");

	cursor.insertText(nickname + QString(": "), nickFormat);

	/* Text */
	QTextCharFormat textFormat;
	textFormat.setForeground(QColor(textColor[light]));
	if (light)
		textFormat.setProperty(QTextFormat::UserProperty, textColor[!light]);

	cursor.insertText(text, textFormat);

	scrollToBottom();
}

void ChatWidget::scrollToBottom()
{
	textArea->verticalScrollBar()->setValue(textArea->verticalScrollBar()->maximum());
}

bool ChatWidget::findBlockIdentifier(int identifier, QTextBlock &dst)
{
	if (!identifier)
		return false;

	QTextBlock b = textArea->document()->lastBlock();
	for (;;)
	{
		if (b.userState() == identifier)
		{
			dst = b;
			return true;
		}

		if (b == textArea->document()->begin())
			break;

		b = b.previous();
	}

	return false;
}

void ChatWidget::showOfflineNotice()
{
	if (offlineNotice)
		return;

	/* Create widget */
	offlineNotice = new QWidget;
	offlineNotice->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QBoxLayout *layout = new QHBoxLayout(offlineNotice);
	layout->setMargin(0);
	layout->setSpacing(3);
	layout->addStretch();

	QLabel *icon = new QLabel;
	icon->setPixmap(QPixmap(":/icons/information.png"));
	icon->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	layout->addWidget(icon);

	QLabel *message = new QLabel;
	message->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	message->setText(tr("<b>%1</b> is not online. Your messages will be delivered as soon as possible.")
					 .arg(Qt::escape(user->nickname())));

	QPalette p = message->palette();
	p.setColor(QPalette::WindowText, Qt::darkGray);
	message->setPalette(p);

	layout->addWidget(message);
	layout->addStretch();

	/* Create the size animation */
	offlineNotice->setMaximumHeight(0);

	QPropertyAnimation *ani = new QPropertyAnimation(offlineNotice, QByteArray("maximumHeight"), offlineNotice);
	ani->setEndValue(offlineNotice->sizeHint().height());
	ani->setDuration(300);

	/* Add to the window */
	QBoxLayout *windowLayout = qobject_cast<QBoxLayout*>(this->layout());
	Q_ASSERT(windowLayout);
	if (!windowLayout)
	{
		delete offlineNotice;
		offlineNotice = 0;
		return;
	}

	windowLayout->insertWidget(1, offlineNotice);
	ani->start(QAbstractAnimation::DeleteWhenStopped);
}

void ChatWidget::clearOfflineNotice()
{
	if (!offlineNotice)
		return;

	/* Animate out */
	QPropertyAnimation *ani = new QPropertyAnimation(offlineNotice, QByteArray("maximumHeight"), offlineNotice);
	ani->setEndValue(0);
	ani->setDuration(300);

	connect(ani, SIGNAL(finished()), this, SLOT(clearOfflineNoticeInstantly()));

	ani->start(QAbstractAnimation::DeleteWhenStopped);
}

void ChatWidget::clearOfflineNoticeInstantly()
{
	if (!offlineNotice)
		return;

	offlineNotice->hide();
	offlineNotice->deleteLater();
	offlineNotice = 0;
}
