#ifndef EXPANDINGTEXTEDIT_H
#define EXPANDINGTEXTEDIT_H

#include <QTextEdit>

class ExpandingTextEdit : public QTextEdit
{
    Q_OBJECT

    Q_PROPERTY(int maxLength READ maxLength WRITE setMaxLength)
    Q_PROPERTY(int maxLines READ maxLines WRITE setMaxLines)

public:
    explicit ExpandingTextEdit(QWidget *parent = 0);

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    int characterCount() const { return m_currentLength; }
    int lineCount() const { return m_currentLines; }

    int maxLength() const { return m_maxLength; }
    int maxLines() const { return m_maxLines; }

    bool canInput() const;

public slots:
    void setMaxLength(int maxLength);
    void setMaxLines(int maxLines);

private slots:
    void documentChanged();

protected:
    virtual void resizeEvent(QResizeEvent *e);

private:
    int m_maxLength, m_maxLines;
    int m_currentLength, m_currentLines;
};

#endif // EXPANDINGTEXTEDIT_H
