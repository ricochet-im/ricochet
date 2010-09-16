/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * Torsion is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Torsion. If not, see http://www.gnu.org/licenses/
 */

#include "ContactInfoPage.h"
#include "core/ContactUser.h"
#include "ui/EditableLabel.h"
#include "ui/ContactIDWidget.h"
#include "utils/DateUtil.h"
#include "utils/PaintUtil.h"
#include "core/NicknameValidator.h"
#include "core/OutgoingContactRequest.h"
#include "ui/MainWindow.h"
#include <QBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QAction>
#include <QToolButton>
#include <QApplication>
#include <QTextStream>
#include <QDateTime>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>

ContactInfoPage::ContactInfoPage(ContactUser *u, QWidget *parent)
    : QWidget(parent), user(u)
{
    connect(user, SIGNAL(contactDeleted(ContactUser*)), SLOT(deleteLater()));

    createActions();

    QBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    QBoxLayout *infoLayout = new QHBoxLayout;
    infoLayout->setMargin(0);
    mainLayout->addLayout(infoLayout);

    infoLayout->addStrut(120);

    createAvatar();
    infoLayout->addWidget(avatar, Qt::AlignTop | Qt::AlignLeft);

    infoLayout->addLayout(createInfo(), 1);
    infoLayout->addStretch();
    infoLayout->addLayout(createButtons());

    if (user->isContactRequest())
        mainLayout->addLayout(createRequestInfo());

    /* Notes */
    createNotes(mainLayout);
}

ContactInfoPage::~ContactInfoPage()
{
    saveNotes();
}

void ContactInfoPage::createActions()
{
    renameAction = new QAction(QIcon(QLatin1String(":/icons/user--pencil.png")), tr("Change Nickname"), this);
    renameAction->setIconVisibleInMenu(false);

    /* Show a grayscale icon normally, and the full color (red) icon when hovered */
    QIcon deleteIcon;
    deleteIcon.addFile(QLatin1String(":/icons/cross.png"), QSize(), QIcon::Active);
    deleteIcon.addPixmap(deleteIcon.pixmap(24, QIcon::Disabled));

    deleteAction = new QAction(deleteIcon, tr("Remove Contact"), this);
    deleteAction->setIconVisibleInMenu(false);
    connect(deleteAction, SIGNAL(triggered()), SLOT(deleteContact()));
}

void ContactInfoPage::createAvatar()
{
    QPixmap image = user->avatar(FullAvatar);

    avatar = new QLabel;
    avatar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    if (!image.isNull())
        avatar->setPixmap(shadowedAvatar(image));
    else
        avatar->setPixmap(QPixmap(QLatin1String(":/graphics/avatar-placeholder.png")));
}

QLayout *ContactInfoPage::createInfo()
{
    QGridLayout *layout = new QGridLayout;
    layout->setMargin(0);
    layout->setVerticalSpacing(0);
    layout->setHorizontalSpacing(5);

    int row = 0;

    /* Nickname */
    nickname = new EditableLabel;
    nickname->setText(user->nickname());
    nickname->addAction(renameAction);
    nickname->setContextMenuPolicy(Qt::ActionsContextMenu);
    NicknameValidator *validator = new NicknameValidator(nickname);
    validator->setWidget(nickname);
    validator->setValidateUnique(true, user);
    nickname->setValidator(validator);

    QFont font = nickname->font();
    font.setPointSize(11);
    nickname->setFont(font);

    QPalette p = nickname->palette();
    p.setColor(QPalette::WindowText, QColor(0x00, 0x66, 0xaa));
    p.setColor(QPalette::Text, QColor(0x00, 0x66, 0xaa));
    nickname->setPalette(p);

    connect(renameAction, SIGNAL(triggered()), nickname, SLOT(startEditing()));
    connect(renameAction, SIGNAL(triggered()), nickname, SLOT(setFocus()));
    connect(renameAction, SIGNAL(triggered()), nickname, SLOT(selectAll()));
    connect(nickname, SIGNAL(editingFinished()), SLOT(setNickname()));

    layout->addWidget(nickname, row++, 0, 1, 2);

    layout->setRowMinimumHeight(row++, 6);

    /* ID */
    p.setColor(QPalette::WindowText, QColor(0x80, 0x80, 0x80));

    QLabel *label = new QLabel(tr("ID:"));
    label->setContentsMargins(0, 0, 0, 0);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    label->setPalette(p);
    layout->addWidget(label, row, 0);

    QLineEdit *id = new ContactIDWidget;
    id->setText(user->contactID());
    id->setFrame(false);
    id->setTextMargins(-2, 0, 0, 0);

    QPalette idPalette = id->palette();
    idPalette.setBrush(QPalette::Base, idPalette.window());
    idPalette.setBrush(QPalette::Text, idPalette.windowText());
    id->setPalette(idPalette);

    layout->addWidget(id, row++, 1);

    /* Contact Request */
    if (user->isContactRequest())
    {
        label = new QLabel(tr("Added:"));
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        label->setPalette(p);
        layout->addWidget(label, row, 0);

        QLabel *reqStatus = new QLabel;
        layout->addWidget(reqStatus, row++, 1);

        QString startDate = timeDifferenceString(user->readSetting("whenCreated").toDateTime(), QDateTime::currentDateTime());
        reqStatus->setText(startDate);
    }

    /* Connected date */
    if (!user->isContactRequest())
    {
        label = new QLabel(tr("Connected:"));
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        label->setPalette(p);
        layout->addWidget(label, row, 0);

        QLabel *connected = new QLabel;
        layout->addWidget(connected, row++, 1);

        QDateTime lastConnect = user->readSetting(QLatin1String("lastConnected")).toDateTime();
        if (user->isConnected())
            connected->setText(tr("Yes"));
        else if (lastConnect.isNull())
            connected->setText(tr("Never"));
        else
        {
            connected->setText(timeDifferenceString(lastConnect, QDateTime::currentDateTime()));
            connected->setToolTip(lastConnect.toString(Qt::DefaultLocaleLongDate));
        }
    }

    layout->setRowStretch(row++, 1);
    return layout;
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
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn->setAutoRaise(true);
    btn->setDefaultAction(renameAction);
    btn->setPalette(p);
    layout->addWidget(btn);

    btn = new QToolButton;
    btn->setFixedHeight(23);
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn->setAutoRaise(true);
    btn->setDefaultAction(deleteAction);
    btn->setPalette(p);
    layout->addWidget(btn);

    layout->addStretch();
    return layout;
}

QLayout *ContactInfoPage::createRequestInfo()
{
    QBoxLayout *reqLayout = new QHBoxLayout;
    reqLayout->setSpacing(4);
    reqLayout->addStretch();

    OutgoingContactRequest *request = OutgoingContactRequest::requestForUser(user);
    Q_ASSERT(request);

    /* Icon */
    QLabel *iconLabel = new QLabel;
    iconLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    reqLayout->addWidget(iconLabel);

    /* Text */
    QLabel *textLabel = new QLabel;
    textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QFont f = textLabel->font();
    f.setPointSize(10);
    textLabel->setFont(f);

    reqLayout->addWidget(textLabel);

    QString text, iconPath;
    QColor textColor = QColor(0x00, 0x66, 0xaa);
    switch (request->status())
    {
    case OutgoingContactRequest::Pending:
        iconPath = QLatin1String(":/icons/information.png");
        text = tr("Your contact request will be sent when <b>%1</b> is online").arg(Qt::escape(user->nickname()));
        break;
    case OutgoingContactRequest::Acknowledged:
        iconPath = QLatin1String(":/icons/information.png");
        text = tr("Waiting for <b>%1</b> to accept your contact request").arg(Qt::escape(user->nickname()));
        break;
    case OutgoingContactRequest::Accepted:
        iconPath = QLatin1String(":/icons/tick-circle.png");
        text = tr("<b>%1</b> accepted your contact request").arg(Qt::escape(user->nickname()));
        break;
    case OutgoingContactRequest::Rejected:
        iconPath = QLatin1String(":/icons/cross-circle.png");
        textColor = QColor(0x9f, 0x00, 0x00);
        text = tr("Your contact request was rejected, and you cannot try again");
        break;
    case OutgoingContactRequest::Error:
        iconPath = QLatin1String(":/icons/exclamation-red.png");
        textColor = QColor(0x9f, 0x00, 0x00);
        text = tr("An error occurred with the contact request");
        break;
    }

    iconLabel->setPixmap(QPixmap(iconPath));
    textLabel->setText(text);

    QPalette p = textLabel->palette();
    p.setColor(QPalette::WindowText, textColor);
    textLabel->setPalette(p);

    reqLayout->addStretch();
    return reqLayout;
}

void ContactInfoPage::createNotes(QBoxLayout *layout)
{
    /* Header */
    QLabel *notesHeader = new QLabel(tr("Private notes:"));
    notesHeader->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    notesHeader->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    notesHeader->setContentsMargins(1, 0, 0, 0);

    QFont font = notesHeader->font();
    font.setPointSize(8);
    font.setBold(true);
    notesHeader->setFont(font);

    QPalette p = notesHeader->palette();
    p.setColor(QPalette::WindowText, Qt::darkGray);
    notesHeader->setPalette(p);

    layout->addWidget(notesHeader);

    /* Edit */
    notesEdit = new QTextEdit;
    notesEdit->insertPlainText(user->notesText());
    QFont notesFont;
    notesFont.setStyleHint(QFont::SansSerif);
    notesFont.setPointSize(9);
    notesEdit->setFont(notesFont);
    layout->addWidget(notesEdit, 1);
}

void ContactInfoPage::saveNotes()
{
    if (!notesEdit->document()->isModified())
        return;

    user->setNotesText(notesEdit->document()->toPlainText());
    notesEdit->document()->setModified(false);
}

void ContactInfoPage::setNickname()
{
    if (nickname->hasAcceptableInput())
        user->setNickname(nickname->text());
}

void ContactInfoPage::deleteContact()
{
    uiMain->uiRemoveContact(user);
}

void ContactInfoPage::hideEvent(QHideEvent *ev)
{
    saveNotes();
    QWidget::hideEvent(ev);
}
