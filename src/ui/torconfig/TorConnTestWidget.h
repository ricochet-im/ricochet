#ifndef TORCONNTESTWIDGET_H
#define TORCONNTESTWIDGET_H

#include <QWidget>

namespace Tor
{
	class TorControlManager;
}

class QLabel;

namespace TorConfig
{

class TorConnTestWidget : public QWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(TorConnTestWidget)

public:
	explicit TorConnTestWidget(QWidget *parent = 0);

	void startTest(const QString &host, quint16 port, const QByteArray &authPassword);

private slots:
	void testSuccess();

private:
	QLabel *infoLabel;
	Tor::TorControlManager *testManager;
};

}

#endif // TORCONNTESTWIDGET_H
