#include "UIHelper.h"
#include <QTextEdit>
#include <QDeclarativeItem>

UIHelper::UIHelper(QObject *parent)
    : QObject(parent)
{
}

QTextEdit *UIHelper::createTextEdit(QDeclarativeItem *proxyItem)
{
    QTextEdit *textEdit = new QTextEdit;
    textEdit->setFrameStyle(QFrame::NoFrame);

    DeclarativeProxiedProxyWidget *w = new DeclarativeProxiedProxyWidget(proxyItem, textEdit);
    Q_UNUSED(w);

    return textEdit;
}
