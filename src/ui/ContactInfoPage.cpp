#include "ContactInfoPage.h"
#include "core/ContactUser.h"
#include <QBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QAction>
#include <QToolButton>

ContactInfoPage::ContactInfoPage(ContactUser *u, QWidget *parent)
	: QWidget(parent), user(u)
{
	createActions();

	QBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);

	QBoxLayout *infoLayout = new QHBoxLayout;
	infoLayout->setMargin(0);
	mainLayout->addLayout(infoLayout);

	createAvatar();
	infoLayout->addWidget(avatar, Qt::AlignTop | Qt::AlignLeft);

	createNickname();
	infoLayout->addWidget(nickname);
	infoLayout->addStretch();

	infoLayout->addLayout(createButtons());

	/* Notes */
	createNotes(mainLayout);
}

ContactInfoPage::~ContactInfoPage()
{
	saveNotes();
}

void ContactInfoPage::createActions()
{
	renameAction = new QAction(QIcon(":/icons/image--pencil.png"), tr("Change Nickname"), this);
	deleteAction = new QAction(QIcon(":/icons/user--plus.png"), tr("Delete Contact"), this);
}

#include <QPainter>

void ContactInfoPage::createAvatar()
{
	avatar = new QLabel;
	avatar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	QPixmap image = user->avatar(ContactUser::FullAvatar);
	if (!image.isNull())
	{
		/* Shadow data (temporary) */
		static const quint32 topCornerData[] =
		{
			0x07000000u, 0x04000000u, 0x02000000u, 0x01000000u,
			0x14000000u, 0x0c000000u, 0x06000000u, 0x02000000u,
			0x28000000u, 0x18000000u, 0x0c000000u, 0x04000000u,
			0x42000000u, 0x28000000u, 0x14000000u, 0x07000000u,
			0x55000000u, 0x33000000u, 0x1a000000u, 0x09000000u,
			0x62000000u, 0x3b000000u, 0x1e000000u, 0x0a000000u
		};
		QImage topCorner(reinterpret_cast<const uchar*>(topCornerData), 4, 6, QImage::Format_ARGB32);

		static const quint32 bottomCornerData[] =
		{
			0x00000000u, 0x00000000u, 0x62000000u, 0x3b000000u, 0x1e000000u, 0x0a000000u,
			0x80000000u, 0x6f000000u, 0x55000000u, 0x33000000u, 0x1a000000u, 0x09000000u,
			0x62000000u, 0x55000000u, 0x42000000u, 0x28000000u, 0x14000000u, 0x07000000u,
			0x3b000000u, 0x33000000u, 0x28000000u, 0x18000000u, 0x0c000000u, 0x04000000u,
			0x1e000000u, 0x1a000000u, 0x14000000u, 0x0c000000u, 0x06000000u, 0x02000000u,
			0x0a000000u, 0x09000000u, 0x07000000u, 0x04000000u, 0x02000000u, 0x01000000u
		};
		QImage bottomCorner(reinterpret_cast<const uchar*>(bottomCornerData), 6, 6, QImage::Format_ARGB32);

		static const quint32 rightData[] = { 0x69000000u, 0x3f000000u, 0x20000000u, 0x0b000000u };
		QImage right(reinterpret_cast<const uchar*>(rightData), 4, 1, QImage::Format_ARGB32);

		static const quint32 bottomData[] = { 0x88000000u, 0x69000000u, 0x3f000000u, 0x20000000u, 0x0b000000u };
		QImage bottom(reinterpret_cast<const uchar*>(bottomData), 1, 5, QImage::Format_ARGB32);

		/* Shadowed image */
		QImage shadowAvatar(image.width() + 4, image.height() + 6, QImage::Format_ARGB32_Premultiplied);
		shadowAvatar.fill(qRgb(240, 240, 240));
		QPainter p(&shadowAvatar);

		/* Draw avatar */
		p.drawPixmap(0, 1, image);

		/* Draw top-right corner */
		p.drawImage(image.width(), 0, topCorner);

		/* Draw right-side repeat */
		p.drawTiledPixmap(QRect(image.width(), 6, 4, image.height() - 7), QPixmap::fromImage(right));

		/* Draw bottom-right corner */
		p.drawImage(image.width() - 2, image.height() - 1, bottomCorner);

		/* Draw bottom repeat */
		p.drawTiledPixmap(QRect(0, image.height(), image.width() - 2, 5), QPixmap::fromImage(bottom));
		p.end();

		avatar->setPixmap(QPixmap::fromImage(shadowAvatar));
	}
	else
		avatar->setPixmap(QPixmap(":/graphics/avatar-placeholder.png"));
}

void ContactInfoPage::createNickname()
{
	nickname = new QLabel;
	nickname->setTextFormat(Qt::PlainText);
	nickname->setText(user->nickname());
	nickname->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	nickname->addAction(renameAction);
	nickname->setContextMenuPolicy(Qt::ActionsContextMenu);

	QFont font("Candara", 12, QFont::Bold);
	nickname->setFont(font);

	QPalette p = nickname->palette();
	p.setColor(QPalette::WindowText, QColor(28, 128, 205));
	nickname->setPalette(p);
}

QLayout *ContactInfoPage::createButtons()
{
	QBoxLayout *layout = new QVBoxLayout;
	layout->setMargin(0);
	layout->setSpacing(0);

	QToolButton *btn = new QToolButton;
	btn->setFixedHeight(23);
	btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	btn->setAutoRaise(true);
	btn->setDefaultAction(renameAction);
	layout->addWidget(btn);

	btn = new QToolButton;
	btn->setFixedHeight(23);
	btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	btn->setAutoRaise(true);
	btn->setDefaultAction(deleteAction);
	layout->addWidget(btn);

	layout->addStretch();
	return layout;
}

void ContactInfoPage::createNotes(QBoxLayout *layout)
{
	/* Header */
	QLabel *notesHeader = new QLabel(tr("Private notes:"));
	notesHeader->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
	notesHeader->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	notesHeader->setContentsMargins(1, 0, 0, 0);

	QFont font = notesHeader->font();
	font.setBold(true);
	notesHeader->setFont(font);

	QPalette p = notesHeader->palette();
	p.setColor(QPalette::WindowText, Qt::darkGray);
	notesHeader->setPalette(p);

	layout->addWidget(notesHeader);

	/* Edit */
	notesEdit = new QTextEdit;
	notesEdit->insertPlainText(user->notesText());
	notesEdit->setFont(QFont("Helvetica", 9));
	layout->addWidget(notesEdit, 1);
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
