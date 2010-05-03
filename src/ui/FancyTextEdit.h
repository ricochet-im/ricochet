#ifndef FANCYTEXTEDIT_H
#define FANCYTEXTEDIT_H

#include <QTextEdit>

class FancyTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit FancyTextEdit(QWidget *parent = 0);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);

    void setPlaceholderText(const QString &placeholderText);
    const QString &placeholderText() const;
private:
    QColor m_storedColor;
    QString m_placeholderText;
};

#endif // FANCYTEXTEDIT_H
