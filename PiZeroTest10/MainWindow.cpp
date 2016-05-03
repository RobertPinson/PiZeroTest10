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

#define PIN 3

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
	
	QObject::connect(&yellowLedTimer, SIGNAL(timeout()), this, SLOT(doWork()));
	
	//Start NFC Reader thread
	if (MyNfcReader->init() > 0)
	{
		MyNfcReader->start();
	}
	
	if (wiringPiSetup() == -1)
	{
		qDebug() << "wiringPi setup error";
	}
	
	//softToneCreate(PIN);
	//pwmSetMode(PWM_MODE_MS);
	
	//set pin mode for LED
	pinMode(pinZero, OUTPUT);
	pinMode(pinTwo, OUTPUT);	
	pinMode(pinOne, PWM_OUTPUT);
	
	//buzzer
	pinMode(PIN, OUTPUT);	
	
	//Set LED Green
	digitalWrite(pinZero, LOW);
	digitalWrite(pinTwo, HIGH);	
}

void MainWindow::OnCardRemoved()
{
	//TODO set LED colour ??
	//pwmWrite(pinOne, 0);
	digitalWrite(PIN, LOW);
}

void MainWindow::OnCardPresent(QString uid)
{	
	//Call API
	qDebug() << "Card Present: Calling API...";
	MyApiClient->PostMovement(uid);
	
	//Set LED Yellow
	startYellowLed();
	
	//Sound buzzer
	digitalWrite(PIN, HIGH);
	
	//pwmWrite(pinOne, 200);
	
//	int scale[8] = { 600, 0 };
//
//	int i, j;
//	char buf[80];
//	
//	for (i = 0; i < 2; ++i)
//	{
//		printf("%3d\n", i);
//		softToneWrite(PIN, scale[i]);
//		delay(100);
//	}	
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
	
	//TODO success beep & LED GREEN
	stopYellowLed();
	
	mutex.unlock();
}

void MainWindow::doWork()
{
	if (isGreen)
	{
		//LED red
		digitalWrite(pinZero, HIGH);
		digitalWrite(pinTwo, LOW);	
		isGreen = false;
	}
	else
	{
		//LED green
		digitalWrite(pinZero, LOW);
		digitalWrite(pinTwo, HIGH);
		isGreen = true;
	}		
}

void MainWindow::startYellowLed()
{	
	yellowLedTimer.start(0);
}

void MainWindow::stopYellowLed()
{
	yellowLedTimer.stop();
	
	//LED green
	digitalWrite(pinZero, LOW);
	digitalWrite(pinTwo, HIGH);	
}

void MainWindow::OnRequestError(QString message)
{
	qDebug() << "API ERROR: " + message;	
	
	//TODO LED RED then GREEN
	
	//TODO buzzer error	
	
}

MainWindow::~MainWindow()
{
	delete ui;
	digitalWrite(pinZero, LOW);
	digitalWrite(pinTwo, LOW);	
}