#include "SetConfCommand.h"
#include "utils/StringUtil.h"

using namespace Tor;

SetConfCommand::SetConfCommand()
	: TorControlCommand("SETCONF")
{
}

QByteArray SetConfCommand::build(const QByteArray &key, const QByteArray &value)
{
	return QByteArray("SETCONF ") + key + "=" + quotedString(value) + "\r\n";
}

QByteArray SetConfCommand::build(const QList<QPair<QByteArray, QByteArray> > &data)
{
	if (data.isEmpty())
		return QByteArray();

	QByteArray out("SETCONF");
	for (int i = 0; i < data.size(); ++i)
	{
		out.append(' ');
		out.append(data[i].first);
		out.append('=');
		out.append(quotedString(data[i].second));
	}

	out.append("\r\n");
	return out;
}

void SetConfCommand::handleReply(int code, QByteArray &data, bool end)
{
	Q_UNUSED(code);

	if (end)
	{
		statusMessage = data;

		if (code == 250)
			emit setConfSucceeded();
		else
			emit setConfFailed(code);
	}
}
