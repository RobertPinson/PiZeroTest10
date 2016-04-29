#include "NfcReader.h"
#include "Utility.h"
#include <QDebug>

using namespace utility;
using namespace std;

NfcReader::NfcReader()
{
}

int NfcReader::init()
{
	QString output;
	
	nfc_init(&context);
	if (context == NULL) {
		qDebug() << "Unable to init libnfc (malloc)\n";
		return -1;
	}
	
	const char *acLibnfcVersion = nfc_version();	
	
	qDebug() << "Pi Zero uses libnfc " << acLibnfcVersion << "\n";	
	
	pnd = nfc_open(context, NULL);
 
	if (pnd == NULL) {
		qDebug() << "ERROR: Unable to open NFC device.\n";
		return -2;
	}
	
	if (nfc_initiator_init(pnd) < 0) {
		nfc_perror(pnd, "nfc_initiator_init");
		return -3;
	}
 
	qDebug() << "NFC reader: " << nfc_device_get_name(pnd) << " opened.\n";	
	
	qDebug() << "NFC Reader Initialized";
	return 1 ;
}

void NfcReader::run()
{
	qDebug() << "NFC Reader running";
	int res = 0;
	int i = 0;
	while (1) {
		
		   // ... hard work
		msleep(100);
		i++;
		if (i == 5)
		{
			if ((res = nfc_initiator_poll_target(pnd, &nmMifare, szModulations, uiPollNr, uiPeriod, &nt)) < 0) {
				nfc_perror(pnd, "nfc_initiator_poll_target");			
			}

			if (res > 0) {
				// Now want to notify main thread:
				string uid = hexstring(nt.nti.nai.abtUid, nt.nti.nai.szUidLen);
				
				emit cardPresent(QString::fromStdString(uid));
				
				qDebug() << "Waiting for card removing...";
				while (0 == nfc_initiator_target_is_present(pnd, NULL)) {}
				
				// Now want to notify main thread:
				emit cardRemoved();
				
				nfc_perror(pnd, "nfc_initiator_target_is_present");
				printf("done.\n");
			}
			else {
				printf("No target found.\n");
			}
			
			i = 0;
		}		   
	}
}

NfcReader::~NfcReader()
{
	nfc_close(pnd);
	nfc_exit(context);
	exit(EXIT_FAILURE);
}