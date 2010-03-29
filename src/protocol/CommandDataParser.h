#ifndef COMMANDDATAPARSER_H
#define COMMANDDATAPARSER_H

#include <QByteArray>

class QString;

class CommandDataParser
{
public:
	static const int maxCommandSize = 65535;

	CommandDataParser(QByteArray *data);
	CommandDataParser(const QByteArray *data);

	QByteArray *data() { return d; }
	void setData(QByteArray *data);
	void setData(const QByteArray *data);

	/* Read position; does not affect writes */
	int pos() const { return p; }
	void setPos(int pos);

	/* Indicates truncation or read-only for writes or missing data for reads */
	bool hasError() const { return error; }
	operator bool() const { return !hasError(); }
	bool operator!() const { return hasError(); }

	bool testSize(int i) const { return i < d->size(); }

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
	bool writable, error;

	template<typename T> bool appendInt(T value);
	template<typename T> bool takeInt(T &value);
};

#endif // COMMANDDATAPARSER_H
