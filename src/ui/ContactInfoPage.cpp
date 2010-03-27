#include "ContactInfoPage.h"
#include "core/ContactUser.h"
#include <QBoxLayout>
#include <QLabel>
#include <QTextEdit>

ContactInfoPage::ContactInfoPage(ContactUser *u, QWidget *parent)
	: QWidget(parent), user(u)
{
	QBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);

	QBoxLayout *infoLayout = new QHBoxLayout;
	infoLayout->setMargin(0);
	mainLayout->addLayout(infoLayout);

	createAvatar();
	infoLayout->addWidget(avatar);

	createNickname();
	infoLayout->addWidget(nickname);
	infoLayout->addStretch();

	createNotes();
	mainLayout->addWidget(notesEdit);
}

ContactInfoPage::~ContactInfoPage()
{
	saveNotes();
}

void ContactInfoPage::createAvatar()
{
	avatar = new QLabel;
	avatar->setPixmap(user->avatar(ContactUser::FullAvatar));
	avatar->setFrameStyle(QFrame::Panel | QFrame::Sunken);
}

void ContactInfoPage::createNickname()
{
	nickname = new QLabel;
	nickname->setTextFormat(Qt::PlainText);
	nickname->setText(user->nickname());
	nickname->setAlignment(Qt::AlignLeft | Qt::AlignTop);

	QFont font = nickname->font();
	font.setPointSize(13);
	nickname->setFont(font);
}

void ContactInfoPage::createNotes()
{
	notesEdit = new QTextEdit;
	notesEdit->insertPlainText(user->notesText());
	notesEdit->setFont(QFont("Helvetica", 9));
}

void ContactInfoPage::saveNotes()
{
	if (!notesEdit->document()->isModified())
		return;

	user->setNotesText(notesEdit->document()->toPlainText());
	notesEdit->document()->setModified(false);
}

void ContactInfoPage::hideEvent(QHideEvent *ev)
{
	saveNotes();
	QWidget::hideEvent(ev);
}
