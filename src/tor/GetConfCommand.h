#ifndef GETCONFCOMMAND_H
#define GETCONFCOMMAND_H

#include "TorControlCommand.h"
#include <QList>
#include <QMultiHash>

namespace Tor
{

class GetConfCommand : public TorControlCommand
{
	Q_OBJECT
	Q_DISABLE_COPY(GetConfCommand)

public:
    GetConfCommand();

	QByteArray build(const QByteArray &key);
	QByteArray build(const QList<QByteArray> &keys);

	const QMultiHash<QByteArray,QByteArray> &results() const { return pResults; }
	bool get(const QByteArray &key, QByteArray &value) const;
	QList<QByteArray> getList(const QByteArray &key) const;

protected:
	virtual void handleReply(int code, QByteArray &data, bool end);

private:
	QMultiHash<QByteArray,QByteArray> pResults;
};

}

#endif // GETCONFCOMMAND_H
