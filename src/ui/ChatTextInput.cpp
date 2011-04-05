#include "ChatTextInput.h"
#include <QKeyEvent>

ChatTextInput::ChatTextInput(QWidget *parent)
    : ExpandingTextEdit(parent)
{
    setTabChangesFocus(true);
    setAcceptRichText(false);
}

void ChatTextInput::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (!(e->modifiers() & Qt::ShiftModifier))
        {
            emit returnPressed();
            if (receivers(SIGNAL(textSubmitted(QString))))
            {
                QString t = text();
                if (!t.isEmpty())
                    emit textSubmitted(t);
                clear();
            }
            e->accept();
            return;
        }
        e->setModifiers(e->modifiers() & ~Qt::ShiftModifier);
        break;
    }

    ExpandingTextEdit::keyPressEvent(e);
}
