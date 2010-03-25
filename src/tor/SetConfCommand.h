#ifndef SETCONFCOMMAND_H
#define SETCONFCOMMAND_H

#include "TorControlCommand.h"
#include <QList>
#include <QPair>

namespace Tor
{

class SetConfCommand : public TorControlCommand
{
public:
	QByteArray statusMessage;

    SetConfCommand();

	QByteArray build(const QByteArray &key, const QByteArray &value);
	QByteArray build(const QList<QPair<QByteArray,QByteArray> > &data);

protected:
	virtual void handleReply(int code, QByteArray &data, bool end);
};

}

#endif // SETCONFCOMMAND_H
