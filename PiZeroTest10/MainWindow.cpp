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
#include <QDateTime>

using namespace utility;
using namespace std;
	
QMutex mutex;

#define PINGREEN 17
#define PINBLUE 4
#define PINRED 22
#define PINBEEP 27

const QString server = "m21.cloudmqtt.com";
const QString uname = "apzvvubw";
const QString pword = "bqqhHe9qGf1A";
const int deviceId = 5;
const int locationId = 2;
static const QString dbPath = "tracker.db";
static void OnMessageCallBack();

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, mClient(new QtMosquittoClient(QString(), true, this))
{
	QApplication::setOverrideCursor(Qt::BlankCursor);
	setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
	ui->setupUi(this);	
	QTimer::singleShot(500, this, SLOT(showFullScreen()));
	
	mClient->setAutoReconnect(true);	
	connect(mClient, SIGNAL(message(QString, QByteArray)), this, SLOT(message(QString, QByteArray)));
	doConnect();
	doSubscribe();
	
	MyNfcReader = new NfcReader;	
	MyApiClient = new ApiClient;
		
	MyDbManager = new DbManager(dbPath);
	
	//Load people
	//Call API
	qDebug() << "Load People: Calling API...";	
	QList<int> excludeIds;
	MyApiClient->GetPeople(excludeIds, deviceId);
	
	//wire up events
	QObject::connect(MyNfcReader,
		SIGNAL(cardPresent(QString)),
		SLOT(OnCardPresent(QString)));
	
	QObject::connect(MyNfcReader,
		SIGNAL(cardRemoved()),
		SLOT(OnCardRemoved()));	
		
	QObject::connect(MyApiClient,
		SIGNAL(movementResponse(Dtos::Person)),
		this,
		SLOT(OnMovementResponse(Dtos::Person)));
	
	QObject::connect(MyApiClient,
		SIGNAL(getPeopleResponse(Dtos::PeopleResponse)),
		this,
		SLOT(OnPeopleResponse(Dtos::PeopleResponse)));
	
	QObject::connect(MyApiClient,
		SIGNAL(requestError(QString)),
		this,
		SLOT(OnRequestError(QString)));	
	
	//Start movement msg timer
	movementTimer = new QTimer(this);
	connect(movementTimer, SIGNAL(timeout()), this, SLOT(OnSendMovements()));
	movementTimer->start(1000);
	
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

//TEST CODE
void MainWindow::doConnect()
{  	
	if (!(uname.isEmpty() || pword.isEmpty()))
	{
		mClient->setUsernamePassword(uname, pword);
	}
	mClient->doConnect(server, 10891);
}

void MainWindow::doPublish(const QString& payload)
{
	const QString topic = "location/2/movement";  
	if (mClient->publish(topic, payload) < 0)
	{
		qDebug() << "Publish failed";
	}
}

void MainWindow::doSubscribe()
{
	const QString topic = "location/2/movement";
	if (!mClient->subscribe(topic))
	{
		qDebug() << "Subscribe failed";
	}
}

void MainWindow::message(const QString& topic, const QByteArray& payload)
{
	qDebug() << "Window::message" << topic << payload;	
	
	QJsonParseError error;
	QJsonDocument jsonDoc = QJsonDocument::fromJson(payload, &error);
		
	if (error.error != QJsonParseError::NoError)
	{
		qDebug() << "MQTT message, Movement Json Parse error.";			
		return;
	}
		
	if ((jsonDoc.isNull() || jsonDoc.isEmpty()))
	{
		qDebug() << "MQTT message, Movement Json is empty.";	
		return;
	}
		
	QJsonObject jsonObject = jsonDoc.object();
	int movementDeviceId = jsonObject["DeviceId"].toInt();
	
	if (movementDeviceId == deviceId)
	{
		qDebug() << "Message ignored: from this device";
		return;
	}
	
	QString movementCardId = jsonObject["CardId"].toString();
	int movementInLocation = jsonObject["InLocation"].toInt();
	
	//Update DB
	Dtos::Person person = MyDbManager->GetPersonByCardId(movementCardId);
	
	if (person.Id > 0)
	{
		//person is known update location status
		person.InLocation = movementInLocation;
		
		MyDbManager->UpsertPerson(person);
		return;
	}
	
	//TODO Get person Data from server....	
}

void MainWindow::doUnsubscribe()
{
	const QString topic = "location/2/movement";
	if (!mClient->unsubscribe(topic))
	{
		qDebug() << "Unsubscribe failed";
	}
}
//END

void MainWindow::OnCardRemoved()
{	
	//Buzzer OFF
	digitalWrite(PINBEEP, LOW);	
}

void MainWindow::OnCardPresent(QString cardId)
{	
	//Call API
	
	//Local DB cache first	
	Dtos::Person person = MyDbManager->GetPersonByCardId(cardId);
	
	if (person.Name.isEmpty())
	{
		qDebug() << "Card Present Not in Cache: Calling API...";	
		MyApiClient->PostMovement(cardId, deviceId);		
		
		//Buzzer ON
		digitalWrite(PINBEEP, HIGH);
	
		//BLUE LED
		digitalWrite(PINGREEN, HIGH);
		digitalWrite(PINBLUE, LOW);
	}
	else
	{
		person.InLocation = !person.InLocation;		
		MyDbManager->UpsertPerson(person);		
			
		//Buzzer ON
		digitalWrite(PINBEEP, HIGH);
	
		//BLUE LED
		digitalWrite(PINGREEN, HIGH);
		digitalWrite(PINBLUE, LOW);
		
		ShowPersonDetailsDialog(person);
	}	
	
	//Add movement to DB
	QDateTime local(QDateTime::currentDateTime());
	QDateTime UTC(local.toUTC());
	MyDbManager->AddMovement(cardId, UTC.toString(Qt::ISODate), person.InLocation);	
	
	qDebug() << "movement saved to DB";
}

void MainWindow::OnPeopleResponse(Dtos::PeopleResponse response)
{
	mutex.lock();
	
	if (response.IsSuccess)
	{
		foreach(const Dtos::Person & person, response.people)
		{
			MyDbManager->UpsertPerson(person);
		}
	}
	else
	{
		qDebug() << "Get People Call Error: " + response.ErrorMessage;		
	}
	
	mutex.unlock();
}

void MainWindow::OnMovementResponse(Dtos::Person person)
{
	mutex.lock();
	qDebug() << "On Movement Post Response";
	qDebug() << "ID: " + QString::number(person.Id);
	qDebug() << "Name: " + person.Name; 
		
	MyDbManager->UpsertPerson(person);
	
	ShowPersonDetailsDialog(person);
	
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

void MainWindow::OnSendMovements()
{
	QList <Dtos::Movement> movements = MyDbManager->GetMovements();
	
	foreach(Dtos::Movement movement, movements)
	{
		//publish movement
		QJsonObject json;
		json.insert("CardId", movement.CardId);
		json.insert("DeviceId", deviceId);	
		json.insert("InLocation", movement.InLocation);
		json.insert("SwipeTime", movement.SwipeTime);
	
		QJsonDocument doc(json);
		QString strJson(doc.toJson(QJsonDocument::Compact));
	
		//send message
		doPublish(strJson);
	
		qDebug() << "Movement message publish";
		
		//delete message
		if (!MyDbManager->DeleteMovement(movement.Id))
		{
			qDebug() << "Failed to delete movement!!";
		}		
	}
}

MainWindow::~MainWindow()
{
	digitalWrite(PINRED, LOW);
	digitalWrite(PINGREEN, LOW);
	digitalWrite(PINBLUE, LOW);	
	
	delete MyApiClient;
	delete MyNfcReader;
	delete ui;
	delete MyDbManager;
}

void MainWindow::ShowPersonDetailsDialog(const Dtos::Person & person)
{
	PersonDetailsDialog* details = new PersonDetailsDialog(this);
	details->setName(person.Name);
	details->setMessage(person.InLocation ? QString("Welcome") : QString("Good Bye"));
	
	//image
	if (!(person.Image.length() <= 0))
	{		
		QPixmap p;
		p.loadFromData(person.Image);	
		details->setImage(p);	
	}
	
	details->show();	
	
	QTimer::singleShot(2000, details, SLOT(hide())); 	
	
	//GREEN LED
	digitalWrite(PINBLUE, HIGH);
	digitalWrite(PINGREEN, LOW);
}
