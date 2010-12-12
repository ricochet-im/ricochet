#ifndef CHATTEXTINPUT_H
#define CHATTEXTINPUT_H

#include "ExpandingTextEdit.h"

class ChatTextInput : public ExpandingTextEdit
{
    Q_OBJECT

    Q_PROPERTY(QString text READ text WRITE setText)

public:
    explicit ChatTextInput(QWidget *parent = 0);

    QString text() const { return toPlainText(); }

public slots:
    void setText(const QString &text) { setPlainText(text); }

signals:
    void returnPressed();
    void textSubmitted(const QString &text);

protected:
    virtual void keyPressEvent(QKeyEvent *e);

private:
    /* Prevent the use of setDocument */
    using QTextEdit::setDocument;
};

#endif // CHATTEXTINPUT_H
