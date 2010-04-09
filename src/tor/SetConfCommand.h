#ifndef SETCONFCOMMAND_H
#define SETCONFCOMMAND_H

#include "TorControlCommand.h"
#include <QList>
#include <QPair>

namespace Tor
{

class SetConfCommand : public TorControlCommand
{
	Q_OBJECT
	Q_DISABLE_COPY(SetConfCommand)

public:
	QByteArray statusMessage;

    SetConfCommand();

	QByteArray build(const QByteArray &key, const QByteArray &value);
	QByteArray build(const QList<QPair<QByteArray,QByteArray> > &data);

signals:
	void setConfSucceeded();
	void setConfFailed(int code);

protected:
	virtual void handleReply(int code, QByteArray &data, bool end);
};

}

#endif // SETCONFCOMMAND_H
