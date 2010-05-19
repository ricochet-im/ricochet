#ifndef FANCYTEXTEDIT_H
#define FANCYTEXTEDIT_H

#include <QTextEdit>

class FancyTextEdit : public QTextEdit
{
    Q_OBJECT
    Q_DISABLE_COPY(FancyTextEdit)

public:
    explicit FancyTextEdit(QWidget *parent = 0);

    void setPlaceholderText(const QString &placeholderText);
    const QString &placeholderText() const;

    bool isPlaceholderActive() const { return m_placeholderActive; }
    bool isTextEmpty() const;

protected:
    virtual void focusInEvent(QFocusEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);

private:
    QColor m_storedColor;
    QString m_placeholderText;
    bool m_placeholderActive;
};

#endif // FANCYTEXTEDIT_H
