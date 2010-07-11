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

#ifndef OSUTIL_H
#define OSUTIL_H

#include <QtGlobal>
class QString;

/* Return the PID written to the given file, or -1 */
qint64 readPidFile(const QString &path);
bool isProcessRunning(qint64 pid);
/* Attempt to kill a process, gracefully or forcefully. Process may not exit immediately. */
bool killProcess(qint64 pid, bool forceful = false);

#endif // OSUTIL_H
