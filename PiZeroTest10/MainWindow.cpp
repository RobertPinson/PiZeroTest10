#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMessageBox>
#include <QString>
#include <QTimer>
#include <nfc/nfc.h>
#include <nfc/nfc-types.h>
#include <nfc/nfc-emulation.h>
#include "Utility.h"
#include <QMutex>
#include <wiringPi.h>
#include <softTone.h>

using namespace utility;
using namespace std;
	
QMutex mutex;

#define PINGREEN 17
#define PINBLUE 4
#define PINRED 22
#define PINBEEP 27

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	QApplication::setOverrideCursor(Qt::BlankCursor);
	setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
	ui->setupUi(this);	
	QTimer::singleShot(1000, this, SLOT(showFullScreen()));
	
	MyNfcReader = new NfcReader;	
	MyApiClient = new ApiClient;
	MyMqttClient = new MqttClient(userName, deviceId, password);
	
	//wire up events
	QObject::connect(MyNfcReader,
		SIGNAL(cardPresent(QString)),
		SLOT(OnCardPresent(QString)));
	
	QObject::connect(MyNfcReader,
		SIGNAL(cardRemoved()),
		SLOT(OnCardRemoved()));	
		
	QObject::connect(MyApiClient,
		SIGNAL(movementResponse(Dtos::MovementResponse)),
		this,
		SLOT(OnMovementResponse(Dtos::MovementResponse)));
	
	QObject::connect(MyApiClient,
		SIGNAL(requestError(QString)),
		this,
		SLOT(OnRequestError(QString)));	
	
	
	//Start NFC Reader thread
	if (MyNfcReader->init() > 0)
	{
		MyNfcReader->start();
	}	
	
	if (wiringPiSetupGpio() == -1)
	{
		qDebug() << "wiringPi setup error";
	}
	
	pinMode(PINRED, OUTPUT);
	pinMode(PINGREEN, OUTPUT);
	pinMode(PINBLUE, OUTPUT);
	pinMode(PINBEEP, OUTPUT);
	
	digitalWrite(PINRED, HIGH);
	digitalWrite(PINGREEN, LOW);
	digitalWrite(PINBLUE, HIGH);	
}

void MainWindow::OnCardRemoved()
{	
	//Buzzer OFF
	digitalWrite(PINBEEP, LOW);	
}

void MainWindow::OnCardPresent(QString uid)
{	
	//Call API
	qDebug() << "Card Present: Calling API...";	
	MyApiClient->PostMovement(uid);
	
	QString message = "Card swiped ID: " + uid;
	
	QByteArray ba = message.toLatin1();
	const char *c_msg = ba.data(); 
	
	//send message
	bool mqttResult = MyMqttClient->publish("device/2/movement", c_msg);
	
	qDebug() << "Movement mesage publish error!";
	
	//Buzzer ON
	digitalWrite(PINBEEP, HIGH);
	
	//BLUE LED
	digitalWrite(PINGREEN, HIGH);
	digitalWrite(PINBLUE, LOW);
}

void MainWindow::OnMovementResponse(Dtos::MovementResponse movementResponse)
{
	mutex.lock();
	qDebug() << "On Movement Post Response";
	qDebug() << "ID: " + QString::number(movementResponse.id);
	qDebug() << "Name: " + movementResponse.name; 
	
	PersonDetailsDialog* details = new PersonDetailsDialog(this);
	details->setName(movementResponse.name);
	details->setMessage(movementResponse.ingress ? QString("Welcome") : QString("Good Bye"));
	
	//image
	if (!(movementResponse.image.length() <= 0))
	{		
		QPixmap p;
		p.loadFromData(movementResponse.image, "JPG");	
		details->setImage(p);	
	}
	
	details->show();	
	
	QTimer::singleShot(2000, details, SLOT(hide())); 	
	
	//GREEN LED
	digitalWrite(PINBLUE, HIGH);
	digitalWrite(PINGREEN, LOW);
	
	mutex.unlock();
}

void MainWindow::OnRequestError(QString message)
{
	qDebug() << "API ERROR: " + message;	
	
	//TODO LED RED then GREEN
	digitalWrite(PINGREEN, HIGH);
	digitalWrite(PINRED, LOW);
	delay(500);
	digitalWrite(PINRED, HIGH);
	digitalWrite(PINGREEN, LOW);
	
	//TODO buzzer error	
	digitalWrite(PINBEEP, HIGH);
	delay(200);
	digitalWrite(PINBEEP, LOW);
}

MainWindow::~MainWindow()
{
	digitalWrite(PINRED, LOW);
	digitalWrite(PINGREEN, LOW);
	digitalWrite(PINBLUE, LOW);
	
	delete MyMqttClient;
	delete MyApiClient;
	delete MyNfcReader;
	delete ui;
}