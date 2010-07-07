/* Torsion - http://github.com/special/torsion
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

#ifndef EDITABLELABEL_H
#define EDITABLELABEL_H

#include <QLineEdit>

class EditableLabel : public QLineEdit
{
    Q_OBJECT
    Q_DISABLE_COPY(EditableLabel)

public:
    explicit EditableLabel(QWidget *parent = 0);

    bool isEditing() const { return !isReadOnly(); }

    const QPalette &palette() const { return originalPalette; }
    void setPalette(const QPalette &palette);

public slots:
    void startEditing();
    void stopEditing();

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *ev);

private:
    QMargins originalMargins;
    QPalette originalPalette;
};

#endif // EDITABLELABEL_H
