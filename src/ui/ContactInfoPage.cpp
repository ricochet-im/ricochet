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

void ContactInfoPage::createAvatar()
{
	avatar = new QLabel;
	avatar->setPixmap(user->avatar(ContactUser::FullAvatar));
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
}
