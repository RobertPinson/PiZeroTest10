#pragma once
#include <QtCore>
#include <nfc/nfc.h>
#include <nfc/nfc-types.h>
#include <nfc/nfc-emulation.h>

class NfcReader : public QThread
{
	Q_OBJECT
	
	nfc_device *pnd;
	nfc_target nt;	
	nfc_context *context;
	
	bool verbose = false;
		
	const nfc_modulation nmMifare = {
		.nmt = NMT_ISO14443A,
		.nbr = NBR_106,
	};
	
	const size_t szModulations = 1;
	const uint8_t uiPollNr = 20;
	const uint8_t uiPeriod = 2;
	
public:
	NfcReader();
	~NfcReader();
	void run();
	bool isrunning;
	int init();
	
signals:
	void cardPresent(QString uid);
	void cardRemoved();
	void readError(int errorCode);
};