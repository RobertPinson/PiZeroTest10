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

using namespace utility;
using namespace std;
	
QMutex mutex;

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
	
	if (MyNfcReader->init() > 0)
	{
		MyNfcReader->start();
	}	
}

void MainWindow::OnCardRemoved()
{
	//TODO set LED colour green
}

void MainWindow::OnCardPresent(QString uid)
{	
	//TODO Set LED colour amber
	
	
	//Call API
	qDebug() << "Card Present: Calling API...";
	MyApiClient->PostMovement(uid);	
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
	
	//TODO beep & LED
	
	mutex.unlock();
}

void MainWindow::OnRequestError(QString message)
{
	qDebug() << "API ERROR: " + message;	
}

MainWindow::~MainWindow()
{
	delete ui;
}