#ifndef P2PAPP_MAIN_HH
#define P2PAPP_MAIN_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>


class NetSocket : public QUdpSocket
{
	Q_OBJECT

public:
	NetSocket();

	// Bind this socket to a P2Papp-specific default port.
	bool bind();
	int myPortMin, myPortMax;
	int assignedPort;
	QString randomID;

private:
	//int myPortMin, myPortMax;
};

class ChatDialog : public QDialog
{
	Q_OBJECT

public:
	ChatDialog();
	void sendMessage(QString);
	QByteArray serializeMessage(QString);
	void deserializeMessage(QByteArray);
	quint32 seqNo;
	NetSocket *sock;
	QMap<QString, quint32> currentStatus;
	QMap<QString, QMap<QString, quint32> > networkStatus;
	

public slots:
	void gotReturnPressed();
	void readMessage();

private:
	QTextEdit *textview;
	QLineEdit *textline;
	
};



#endif // P2PAPP_MAIN_HH
