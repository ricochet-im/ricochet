#ifndef COMMANDDATAPARSER_H
#define COMMANDDATAPARSER_H

#include <QByteArray>

class QString;

class CommandDataParser
{
public:
	enum ParserMode
	{
		Read,
		Write
	} mode;

	CommandDataParser(QByteArray *data, ParserMode mode);

	QByteArray *data() { return d; }
	void setData(QByteArray *data);

	int pos() const { return p; }
	void setPos(int pos);

	/* Input */
	CommandDataParser &operator<<(bool boolean);
	CommandDataParser &operator<<(qint8 byte);
	CommandDataParser &operator<<(quint8 byte);
	CommandDataParser &operator<<(qint16 value);
	CommandDataParser &operator<<(quint16 value);
	CommandDataParser &operator<<(qint32 value);
	CommandDataParser &operator<<(quint32 value);
	CommandDataParser &operator<<(qint64 value);
	CommandDataParser &operator<<(quint64 value);
	CommandDataParser &operator<<(const QString &string);

	/* Output */
	CommandDataParser &operator>>(bool &boolean);
	CommandDataParser &operator>>(qint8 &byte);
	CommandDataParser &operator>>(quint8 &byte);
	CommandDataParser &operator>>(qint16 &value);
	CommandDataParser &operator>>(quint16 &value);
	CommandDataParser &operator>>(qint32 &value);
	CommandDataParser &operator>>(quint32 &value);
	CommandDataParser &operator>>(qint64 &value);
	CommandDataParser &operator>>(quint64 &value);
	CommandDataParser &operator>>(QString &string);

private:
	QByteArray *d;
	int p;

	template<typename T> bool appendInt(T value);
	template<typename T> bool takeInt(T &value);
};

#endif // COMMANDDATAPARSER_H
