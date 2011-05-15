#include "UIHelper.h"
#include "ui/ChatTextWidget.h"
#include <QTextEdit>
#include <QDeclarativeItem>

UIHelper::UIHelper(QObject *parent)
    : QObject(parent)
{
}

ChatTextWidget *UIHelper::createChatArea(ContactUser *user, QDeclarativeItem *proxyItem)
{
    ChatTextWidget *text = new ChatTextWidget(user);
    text->setFrameStyle(QFrame::NoFrame);

    DeclarativeProxiedProxyWidget *w = new DeclarativeProxiedProxyWidget(proxyItem, text);
    Q_UNUSED(w);

    return text;
}
