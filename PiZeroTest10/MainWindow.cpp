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

static const QString path = "tracker.db";
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
		
	MyDbManager = new DbManager(path);
	
	//Load people
	//Call API
	qDebug() << "Load People: Calling API...";	
	QList<int> excludeIds;
	MyApiClient->GetPeople(excludeIds);
	
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
	const QString server = "m21.cloudmqtt.com";
	const QString uname = "apzvvubw";
	const QString pword = "bqqhHe9qGf1A";
	if (!(uname.isEmpty() || pword.isEmpty()))
	{
		mClient->setUsernamePassword(uname, pword);
	}
	mClient->doConnect(server, 10891);
}

void MainWindow::doPublish(const QString& payload)
{
	const QString topic = "location/2/movement";
	//const QString payload = mPublishPayloadEdit->text().trimmed();
  
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
		MyApiClient->PostMovement(cardId);		
		
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
	
	QJsonObject json;
	json.insert("CardId", cardId);
	json.insert("DeviceId", 5);	
	json.insert("InLocation", person.InLocation);	
	
	QDateTime local(QDateTime::currentDateTime());
	QDateTime UTC(local.toUTC());
	json.insert("SwipeTime", UTC.toString());
	
	QJsonDocument doc(json);
	QString strJson(doc.toJson(QJsonDocument::Compact));
	
	QByteArray ba = strJson.toLatin1();
	const char *c_msg = ba.data(); 
	
	//send message
	doPublish(strJson);
	//bool mqttResult = MyMqttClient->publish("location/2/movement", c_msg);
	
	qDebug() << "Movement message publish!";	
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
