#include "ContactInfoPage.h"
#include "core/ContactUser.h"
#include <QBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QAction>
#include <QToolButton>
#include <QApplication>

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

/* 6x6 */
static const quint32 topLeftData[] =
{
	0x1000000u, 0x2000000u, 0x4000000u, 0x7000000u, 0x9000000u, 0xa000000u,
	0x2000000u, 0x6000000u, 0x0u, 0x0u, 0x0u, 0x0u,
	0x4000000u, 0xc000000u, 0x0u, 0x0u, 0x0u, 0x0u,
	0x7000000u, 0x14000000u, 0x0u, 0x0u, 0x0u, 0x0u,
	0x9000000u, 0x1a000000u, 0x0u, 0x0u, 0x0u, 0x0u,
	0xa000000u, 0x1e000000u, 0x0u, 0x0u, 0x0u, 0x0u
};
/* 6x6 */
static const quint32 topRightData[] =
{
	0xa000000u, 0x9000000u, 0x7000000u, 0x4000000u, 0x2000000u, 0x1000000u,
	0x0u, 0x0u, 0x14000000u, 0xc000000u, 0x6000000u, 0x2000000u,
	0x0u, 0x0u, 0x28000000u, 0x18000000u, 0xc000000u, 0x4000000u,
	0x0u, 0x0u, 0x42000000u, 0x28000000u, 0x14000000u, 0x7000000u,
	0x0u, 0x0u, 0x55000000u, 0x33000000u, 0x1a000000u, 0x9000000u,
	0x0u, 0x0u, 0x62000000u, 0x3b000000u, 0x1e000000u, 0xa000000u
};
/* 6x6 */
static const quint32 bottomRightData[] =
{
	0x0u, 0x0u, 0x62000000u, 0x3b000000u, 0x1e000000u, 0xa000000u,
	0x80000000u, 0x6f000000u, 0x55000000u, 0x33000000u, 0x1a000000u, 0x9000000u,
	0x62000000u, 0x55000000u, 0x42000000u, 0x28000000u, 0x14000000u, 0x7000000u,
	0x3b000000u, 0x33000000u, 0x28000000u, 0x18000000u, 0xc000000u, 0x4000000u,
	0x1e000000u, 0x1a000000u, 0x14000000u, 0xc000000u, 0x6000000u, 0x2000000u,
	0xa000000u, 0x9000000u, 0x7000000u, 0x4000000u, 0x2000000u, 0x1000000u
};
/* 6x6 */
static const quint32 bottomLeftData[] =
{
	0xa000000u, 0x1e000000u, 0x0u, 0x0u, 0x0u, 0x0u,
	0x9000000u, 0x1a000000u, 0x33000000u, 0x55000000u, 0x6f000000u, 0x80000000u,
	0x7000000u, 0x14000000u, 0x28000000u, 0x42000000u, 0x55000000u, 0x62000000u,
	0x4000000u, 0xc000000u, 0x18000000u, 0x28000000u, 0x33000000u, 0x3b000000u,
	0x2000000u, 0x6000000u, 0xc000000u, 0x14000000u, 0x1a000000u, 0x1e000000u,
	0x1000000u, 0x2000000u, 0x4000000u, 0x7000000u, 0x9000000u, 0xa000000u
};
/* 1x5 */
static const quint32 bottomData[] =
{
	0x88000000u,
	0x69000000u,
	0x3f000000u,
	0x20000000u,
	0xb000000u
};

void ContactInfoPage::createAvatar()
{
	avatar = new QLabel;
	avatar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	QPixmap image = user->avatar(ContactUser::FullAvatar);
	if (!image.isNull())
	{
		QImage topLeft(reinterpret_cast<const uchar*>(topLeftData), 6, 6, QImage::Format_ARGB32);
		QImage topRight(reinterpret_cast<const uchar*>(topRightData), 6, 6, QImage::Format_ARGB32);
		QImage bottomRight(reinterpret_cast<const uchar*>(bottomRightData), 6, 6, QImage::Format_ARGB32);
		QImage bottomLeft(reinterpret_cast<const uchar*>(bottomLeftData), 6, 6, QImage::Format_ARGB32);

		static const quint32 rightData[] = { 0x69000000u, 0x3f000000u, 0x20000000u, 0x0b000000u };
		QImage right(reinterpret_cast<const uchar*>(rightData), 4, 1, QImage::Format_ARGB32);

		QImage bottom(reinterpret_cast<const uchar*>(bottomData), 1, 5, QImage::Format_ARGB32);

		/* Shadowed image */
		QImage shadowAvatar(image.width() + 6, image.height() + 6, QImage::Format_ARGB32_Premultiplied);
		shadowAvatar.fill(qRgb(240, 240, 240));
		QPainter p(&shadowAvatar);

		QRect imageRect(2, 1, image.width(), image.height());

		/* Draw avatar */
		p.drawPixmap(imageRect.topLeft(), image);

		/* Draw top-left corner */
		p.drawImage(0, 0, topLeft);

		/* Draw top */
		p.setPen(QColor(0, 0, 0, 11));
		p.drawLine(6, 0, imageRect.right() - 2, 0);

		/* Draw left */
		p.drawLine(0, 6, 0, imageRect.bottom() - 1);

		p.setPen(QColor(0, 0, 0, 32));
		p.drawLine(1, 6, 1, imageRect.bottom() - 1);

		/* Draw top-right corner */
		p.drawImage(imageRect.right() - 1, 0, topRight);

		/* Draw right-side repeat */
		p.drawTiledPixmap(QRect(imageRect.right()+1, 6, 4, imageRect.height() - 6), QPixmap::fromImage(right));

		/* Draw bottom-right corner */
		p.drawImage(imageRect.right() - 1, imageRect.bottom(), bottomRight);

		/* Draw bottom repeat */
		p.drawTiledPixmap(QRect(6, imageRect.bottom()+1, image.width() - 6, 5), QPixmap::fromImage(bottom));

		/* Draw bottom-left corner */
		p.drawImage(0, imageRect.bottom(), bottomLeft);

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

	QPalette p = QApplication::palette();
	p.setColor(QPalette::ButtonText, QColor(104, 104, 104));

	QToolButton *btn = new QToolButton;
	btn->setFixedHeight(23);
	btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	btn->setAutoRaise(true);
	btn->setDefaultAction(renameAction);
	btn->setPalette(p);
	layout->addWidget(btn);

	btn = new QToolButton;
	btn->setFixedHeight(23);
	btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	btn->setAutoRaise(true);
	btn->setDefaultAction(deleteAction);
	btn->setPalette(p);
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
