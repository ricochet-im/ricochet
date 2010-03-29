#include "CommandDataParser.h"
#include <QtEndian>
#include <QString>

CommandDataParser::CommandDataParser(QByteArray *data, ParserMode m)
	: mode(m), d(data), p(0)
{
}

void CommandDataParser::setData(QByteArray *data)
{
	d = data;
	p = 0;
}

void CommandDataParser::setPos(int pos)
{
	Q_ASSERT(mode == Read);
	Q_ASSERT(pos >= 0 && pos < d->size());
	p = pos;
}

/* Helpers */
template<typename T> bool CommandDataParser::appendInt(T value)
{
	Q_ASSERT(mode == Write);

	value = qToBigEndian(value);
	d->append(reinterpret_cast<const char*>(&value), sizeof(T));

	return true;
}

template<> bool CommandDataParser::appendInt<quint8>(quint8 value)
{
	Q_ASSERT(mode == Write);

	d->append(static_cast<char>(value));
	return true;
}

template<typename T> bool CommandDataParser::takeInt(T &value)
{
	Q_ASSERT(mode == Read);

	if (p + sizeof(T) > d->size())
		return false;

	value = qFromBigEndian<T>(reinterpret_cast<const uchar*>(d->constData()) + p);
	p += sizeof(T);

	return true;
}

template<> bool CommandDataParser::takeInt<quint8>(quint8 &value)
{
	Q_ASSERT(mode == Read);

	if (p == d->size())
		return false;

	value = *((quint8*)(d->constData() + p));
	p++;
	return true;
}

/* Input - integer types */
CommandDataParser &CommandDataParser::operator<<(bool boolean)
{
	appendInt(static_cast<quint8>(boolean));
	return *this;
}

CommandDataParser &CommandDataParser::operator<<(qint8 value)
{
	appendInt(static_cast<quint8>(value));
	return *this;
}

CommandDataParser &CommandDataParser::operator<<(quint8 byte)
{
	appendInt(byte);
	return *this;
}

CommandDataParser &CommandDataParser::operator<<(qint16 value)
{
	appendInt(value);
	return *this;
}

CommandDataParser &CommandDataParser::operator<<(quint16 value)
{
	appendInt(value);
	return *this;
}

CommandDataParser &CommandDataParser::operator<<(qint32 value)
{
	appendInt(value);
	return *this;
}

CommandDataParser &CommandDataParser::operator<<(quint32 value)
{
	appendInt(value);
	return *this;
}

CommandDataParser &CommandDataParser::operator<<(qint64 value)
{
	appendInt(value);
	return *this;
}

CommandDataParser &CommandDataParser::operator<<(quint64 value)
{
	appendInt(value);
	return *this;
}

/* Input */
CommandDataParser &CommandDataParser::operator<<(const QString &string)
{
	QByteArray encoded = string.toUtf8();
	if (encoded.size() > 65535)
		encoded.resize(65535);

	d->reserve(2 + encoded.size());
	appendInt((quint16)encoded.size());
	d->append(encoded);

	return *this;
}

/* Output - integer types */
CommandDataParser &CommandDataParser::operator>>(bool &boolean)
{
	takeInt(reinterpret_cast<quint8&>(boolean));
	return *this;
}

CommandDataParser &CommandDataParser::operator>>(qint8 &byte)
{
	takeInt(reinterpret_cast<quint8&>(byte));
	return *this;
}

CommandDataParser &CommandDataParser::operator>>(quint8 &byte)
{
	takeInt(byte);
	return *this;
}

CommandDataParser &CommandDataParser::operator>>(qint16 &value)
{
	takeInt(value);
	return *this;
}

CommandDataParser &CommandDataParser::operator>>(quint16 &value)
{
	takeInt(value);
	return *this;
}

CommandDataParser &CommandDataParser::operator>>(qint32 &value)
{
	takeInt(value);
	return *this;
}

CommandDataParser &CommandDataParser::operator>>(quint32 &value)
{
	takeInt(value);
	return *this;
}

CommandDataParser &CommandDataParser::operator>>(qint64 &value)
{
	takeInt(value);
	return *this;
}

CommandDataParser &CommandDataParser::operator>>(quint64 &value)
{
	takeInt(value);
	return *this;
}

CommandDataParser &CommandDataParser::operator>>(QString &string)
{
	quint16 length = 0;
	if (takeInt(length) && (p + length) <= d->size())
	{
		string = QString::fromUtf8(d->constData() + p, length);
		p += length;
	}

	return *this;
}
