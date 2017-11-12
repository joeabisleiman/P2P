
#include <unistd.h>

#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>
#include <cstdlib>
#include <ctime>
#include <string>
#include <QTimer>

#include "main.hh"

ChatDialog::ChatDialog()
{
	setWindowTitle("P2Papp");

	// Read-only text box where we display messages from everyone.
	// This widget expands both horizontally and vertically.
	textview = new QTextEdit(this);
	textview->setReadOnly(true);
    textview->setMinimumSize(800,800);

	// Small text-entry box the user can enter messages.
	// This widget normally expands only horizontally,
	// leaving extra vertical space for the textview widget.
	//
	// You might change this into a read/write QTextEdit,
	// so that the user can easily enter multi-line messages.
	textline = new QLineEdit(this);

	// Lay out the widgets to appear in the main window.
	// For Qt widget and layout concepts see:
	// http://doc.qt.nokia.com/4.7-snapshot/widgets-and-layouts.html
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(textview);
	layout->addWidget(textline);
	setLayout(layout);
	seqNo = 0;
	// Register a callback on the textline's returnPressed signal
	// so that we can send the message entered by the user.
	connect(textline, SIGNAL(returnPressed()),
		this, SLOT(gotReturnPressed()));
	
	sock = new NetSocket();
	if (!sock->bind())
		exit(1);
	setWindowTitle(QString::number(sock->assignedPort % sock->myPortMin + 1));

	connect(sock, SIGNAL(readyRead()),
		this, SLOT(readMessage()));

    entropyTimer = new QTimer(this);
    connect(entropyTimer, SIGNAL(timeout()),
        this, SLOT(anti_entropy()));
    entropyTimer->start(10000);

    timeout = new QTimer(this);
    connect(timeout, SIGNAL(timeout()),
        this, SLOT(rumorMonger()));
}

int ChatDialog::choosePeer(){
    if(sock->assignedPort == sock->myPortMin) {
        peer = sock->myPortMin+1;
    }

    else if(sock->assignedPort == sock->myPortMax) {
        peer = sock->myPortMax-1;
    }

    else {
        qsrand(time(0));
        if(1+(rand() %2) == 1) {
            peer = sock->assignedPort-1;
        }
        else {
        peer = sock->assignedPort+1;
        }
    }
    return 0;
}

void ChatDialog::rumorMonger()
{
    //FLIP COIN to EITHER choose new PEER and RUMORMONGER or QUIT
    peer = choosePeer();
    sendStatusMessage(peer);
    timeout->start(2000);
}

void ChatDialog::anti_entropy()
{
    qDebug() << "Entropy timer fired up.";
    receiverPort = choosePeer(); //TO SIMULATE RANDOM PEER CHOICE
    sendStatusMessage(receiverPort);
    entropyTimer->start(10000);
}

void ChatDialog::sendStatusMessage(int rPort)
{
    QByteArray statusMessage = serializeStatusMessage();
    qDebug() <<  "INFO: Trying to send status message.";
    sock->writeDatagram(statusMessage, statusMessage.size(), QHostAddress::LocalHost, receiverPort);
}

QByteArray ChatDialog::serializeStatusMessage()
{
    QMap<QString, QMap<QString, quint32> > networkStatusMessage;
    networkStatusMessage.insert("Want", statusMessage);

    QByteArray statusMessageArray;
    QDataStream statusMessageStream(&statusMessageArray,QIODevice::ReadWrite);
    statusMessageStream << networkStatusMessage;
    return statusMessageArray;
}

void ChatDialog::sendMessage(QString input)
{
	QByteArray message = serializeMessage(input);
	qDebug() <<  "INFO: Trying to send message.";

/*	if(sock->assignedPort == sock->myPortMin) {
		sock->writeDatagram(message, message.size(), QHostAddress::LocalHost, sock->myPortMin+1);
	}

	else if(sock->assignedPort == sock->myPortMax) {
		sock->writeDatagram(message, message.size(), QHostAddress::LocalHost, sock->myPortMax-1);
	}

	else { 
		qsrand(time(0));
		if(1+(rand() %2) == 1) {
			sock->writeDatagram(message, message.size(), QHostAddress::LocalHost, sock->assignedPort-1);
		}
		else {
		sock->writeDatagram(message, message.size(), QHostAddress::LocalHost, sock->assignedPort+1);
		}
    }
*/
    peer = choosePeer();
    sock->writeDatagram(message, message.size(), QHostAddress::LocalHost, peer);

    //TODO: ADD TIMEOUT
    timeout->start(2000);
}

QByteArray ChatDialog::serializeMessage(QString input)
{
	QVariantMap message;
	message.insert("ChatText", input);
	//TODO: Add Origin and SeqNo
	message.insert("Origin", sock->randomID);
	message.insert("SeqNo", seqNo);
	seqNo ++;

	QByteArray messageArray;
	QDataStream messageStream(&messageArray,QIODevice::ReadWrite);
	messageStream << message;	
	return messageArray;
}

void ChatDialog::readMessage()
{
    //while(sock->hasPendingDatagrams()) {
        qDebug() <<  "INFO: Trying to read message12.";
        // when data comes in
                QByteArray message;
            message.resize(sock->pendingDatagramSize());

        QHostAddress sender;
            quint16 senderPort;

        sock->readDatagram(message.data(), message.size(),
                         &sender, &senderPort);
        deserializeMessage(message);

    //}
}

void ChatDialog::deserializeMessage(QByteArray input)
{
	QVariantMap map;
	QDataStream deserializer(&input, QIODevice::ReadOnly);
    deserializer >> map;

    if (map.contains("ChatText")){
        handleReceivedMessage(map);
    }
    else if(map.contains("Want")) {
        handleReceivedStatusMessage(map);
    }
}

void ChatDialog::handleReceivedMessage(QVariantMap map)
{
        //CHECK SEQNO: IF EXPECTED, ADD TO MESSAGELIST AND STATUS MESSAGE LIST AND THEN SEND UPDATED STATUS MESSAGE LIST (has new seqno) also decide whether
        //to send the message to another random peer
        //IF NOT EXPECTED, DISREGARD, and SEND OLD STATUS MESSAGE LIST
        qDebug() << "Received Origin: " << map.value("Origin").toString();
        if(statusMessage.contains(map.value("Origin").toString()) ) {
            qDebug() << "Existing Peer";

        }
        else {
            textview->append("ARE WE HERE?");
            statusMessage.insert(map.value("Origin").toString(),map.value("SeqNo").toUInt());
        }

        textview->append("Incoming Message from: " + map.value("Origin").toString());
        textview->append("Message is: " + map.value("ChatText").toString());
        textview->append("Sequence Number is: " + map.value("SeqNo").toString());

        //

}

void ChatDialog::handleReceivedStatusMessage()
{

    //DESERIALIZE STATUS MESSAGE AND DECIDE WHETHER YOU NEED TO SEND, OR RECEIVE, OR FLIP (DIE)
}

void ChatDialog::gotReturnPressed()
{
	// Initially, just echo the string locally.
	// Insert some networking code here...
	QString message = textline ->text();
    //qDebug() << "FIX: send message to other peers: " << message;
    //textview->append(textline->text());
	//TODO: Add sent message to our own list of messages statuses
	if(statusMessage.contains(sock->randomID)) {
        	statusMessage[sock->randomID] += 1;
    	}
    	else {
        	statusMessage.insert(sock->randomID, 1);
	}

	//Construct our message and add to list of all messages
	currentMessageList.append(message);
        allMessages.insert(sock->randomID, currentMessageList);

	sendMessage(message);
	// Clear the textline to get ready for the next input message.
	textline->clear();
}

NetSocket::NetSocket()
{
	// Pick a range of four UDP ports to try to allocate by default,
	// computed based on my Unix user ID.
	// This makes it trivial for up to four P2Papp instances per user
	// to find each other on the same host,
	// barring UDP port conflicts with other applications
	// (which are quite possible).
	// We use the range from 32768 to 49151 for this purpose.
	myPortMin = 32768 + (getuid() % 4096)*4;
	myPortMax = myPortMin + 3;
}

bool NetSocket::bind()
{
	// Try to bind to each of the range myPortMin..myPortMax in turn.
	for (int p = myPortMin; p <= myPortMax; p++) {
		if (QUdpSocket::bind(p)) {
			qDebug() << "bound to UDP port " << p;
			assignedPort = p;
			qsrand(time(0));
			randomID = QString::number(p) + QString::number(qrand());
			qDebug() << "Your ID is:  " << randomID;
			return true;
		}
	}

	qDebug() << "Oops, no ports in my default range " << myPortMin
		<< "-" << myPortMax << " available";
	return false;
}

int main(int argc, char **argv)
{
	// Initialize Qt toolkit
	QApplication app(argc,argv);

	// Create an initial chat dialog window
	ChatDialog dialog;
	dialog.show();

	// Create a UDP network socket
	//NetSocket sock;
	//sock = new NetSocket();
	//if (!sock.bind())
	//	exit(1);

	// Enter the Qt main loop; everything else is event driven
	return app.exec();
}

