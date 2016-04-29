#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMessageBox>
#include <QString>
#include <QTimer>
#include <nfc/nfc.h>
#include <nfc/nfc-types.h>
#include <nfc/nfc-emulation.h>
#include "Utility.h"
//#include "NfcReader.h"

using namespace utility;
using namespace std;

Q_DECLARE_METATYPE(Dtos::MovementResponse)
	
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
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
		SIGNAL(movementResponse(Dtos::MovementResponse)), this,
		SLOT(OnMovementResponse(Dtos::MovementResponse)));	
	
	if (MyNfcReader->init() > 0)
	{
		MyNfcReader->start();
	}	
}

void MainWindow::OnCardRemoved()
{
	//Todo card removed action e.g. set led colour green
}

void MainWindow::OnCardPresent(QString uid)
{
	//ui->txtOutput->appendPlainText(uid += "\n");
	
	//TODO Set Led colour amber
	
	
	//Call API
	qDebug() << "Card Present: Calling API...";
	MyApiClient->PostMovement(uid);	
}

void MainWindow::OnMovementResponse(Dtos::MovementResponse movementResponse)
{
	ui->txtOutput->appendPlainText("ID: " + QString::number(movementResponse.id));
	ui->txtOutput->appendPlainText("Name: " + movementResponse.name);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ButtonClicked()
{	
	
	
//	ui->txtOutput->appendPlainText("");
//	
//	nfc_device *pnd;
//	nfc_target nt;
//	
//	nfc_context *context;	
//	string output;
//	
//	nfc_init(&context);
//	if (context == NULL) {
//		output += "Unable to init libnfc (malloc)\n";
//		exit(EXIT_FAILURE);
//	}
//	
//	const char *acLibnfcVersion = nfc_version();
//	output += "Pi Zero uses libnfc ";
//	output += acLibnfcVersion;
//	output += "\n";
//	
//	pnd = nfc_open(context, NULL);
// 
//	if (pnd == NULL) {
//		output += "ERROR: Unable to open NFC device.\n";
//		exit(EXIT_FAILURE);
//	}
//	
//	if (nfc_initiator_init(pnd) < 0) {
//		nfc_perror(pnd, "nfc_initiator_init");
//		exit(EXIT_FAILURE);
//	}
// 
//	output += "NFC reader: ";
//	output += nfc_device_get_name(pnd);
//	output += "opened.\n";
//	
//	const nfc_modulation nmMifare = {
//		.nmt = NMT_ISO14443A,
//		.nbr = NBR_106,
//	};
//	
//	if (nfc_initiator_select_passive_target(pnd, nmMifare, NULL, 0, &nt) > 0) {
//		output += "The following (NFC) ISO14443A tag was found:\n";
//		output += "    ATQA (SENS_RES): ";
//		output += hexstring(nt.nti.nai.abtAtqa, 2);
//		output += "-";
//		output += hexstring(nt.nti.nai.abtAtqa, 2);
//		output += "       UID (NFCID";
//		output += (nt.nti.nai.abtUid[0] == 0x08 ? '3' : '1');
//		output += "): ";
//		output += hexstring(nt.nti.nai.abtUid, nt.nti.nai.szUidLen);
//		output += "      SAK (SEL_RES): ";
//		output += hexstring(&nt.nti.nai.btSak, 1);
//		if (nt.nti.nai.szAtsLen) {
//			output += "          ATS (ATR): ";
//			output += hexstring(nt.nti.nai.abtAts, nt.nti.nai.szAtsLen);
//		}
//	}	
//	
//	ui->txtOutput->appendPlainText(QString::fromStdString(output));
//	
//	nfc_close(pnd);
//	nfc_exit(context);	
}
