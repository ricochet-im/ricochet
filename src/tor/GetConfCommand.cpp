#include "GetConfCommand.h"
#include "utils/StringUtil.h"

using namespace Tor;

GetConfCommand::GetConfCommand()
	: TorControlCommand("GETCONF")
{
}

QByteArray GetConfCommand::build(const QByteArray &key)
{
	return QByteArray("GETCONF ") + key + "\r\n";
}

QByteArray GetConfCommand::build(const QList<QByteArray> &keys)
{
	QByteArray out("GETCONF");
	for (QList<QByteArray>::ConstIterator it = keys.begin(); it != keys.end(); ++it)
	{
		out.append(' ');
		out.append(*it);
	}

	out.append("\r\n");
	return out;
}

void GetConfCommand::handleReply(int code, QByteArray &data, bool end)
{
	if (code != 250)
		return;

	int kep = data.indexOf('=');
	pResults.insertMulti(data.mid(0, kep), (kep >= 0) ? data.mid(kep+1) : QByteArray());
}

bool GetConfCommand::get(const QByteArray &key, QByteArray &value) const
{
	QMultiHash<QByteArray,QByteArray>::ConstIterator it = pResults.find(key);
	if (it == pResults.end())
		return false;

	value = *it;
	return true;
}

QList<QByteArray> GetConfCommand::getList(const QByteArray &key) const
{
	/* QHash returns values from the most recent to the least recent, but Tor sends its values
	 * from first to last, and order may be relevant. So, reverse the list */

	QList<QByteArray> values = pResults.values(key);
	QList<QByteArray> out;

	QList<QByteArray>::Iterator it = values.end();
	while (it != values.begin())
	{
		--it;
		out.append(*it);
	}

	return out;
}
