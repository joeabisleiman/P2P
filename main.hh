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
    void deserializeMessage(QByteArray input, quint16 senderPort);
	quint32 seqNo;
	NetSocket *sock;
    QMap<QString, quint32> statusMessage;
    //QMap<QString, QMap<QString, quint32> > networkStatusMessage; //superfluous
    //QList<QString> currentMessageList;
	QMap<QString, QList<QString> > allMessages;
    void handleReceivedMessage(QVariantMap map, quint16 senderPort);
    QTimer * entropyTimer;
    void sendStatusMessage(int);
    QByteArray serializeStatusMessage();
    int receiverPort;
    int choosePeer();
    int peer;
    void handleReceivedStatusMessage(QMap<QString, QMap<QString, quint32> > _map, quint16 senderPort);
    QTimer * timeout;
    QByteArray lastAttemptedMessage;
    int lastAttemptedPeer;
    QByteArray serializeMissingMessage(QString input, quint32 seqNumber, QString origin);

	

public slots:
	void gotReturnPressed();
	void readMessage();
    void anti_entropy();
    void reSendMessage();

private:
	QTextEdit *textview;
	QLineEdit *textline;
	
};



#endif // P2PAPP_MAIN_HH
