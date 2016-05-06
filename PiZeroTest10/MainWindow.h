#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "NfcReader.h"
#include "ApiClient.h"
#include "PersonDetailsDialog.h"
#include "MqttClient.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
		
	NfcReader *MyNfcReader;
	ApiClient *MyApiClient;
	MqttClient *MyMqttClient;
    
public:
    explicit MainWindow(QWidget *parent = 0);	
    ~MainWindow();

protected slots:  
	void OnCardPresent(QString uid);
	void OnCardRemoved();
	void OnMovementResponse(Dtos::MovementResponse movementResponse);
	void OnRequestError(QString message);
	
	private slots :
		 void doWork();
private:
    Ui::MainWindow *ui;	
	const int pinZero = 0;
	const int pinTwo = 2;
	const int pinOne = 1;
	bool isGreen;
	QTimer yellowLedTimer;
	void startYellowLed();
	void stopYellowLed();
	char *userName = "apzvvubw";
	char *deviceId = "5";
	char *password = "bqqhHe9qGf1A";
};

#endif // MAINWINDOW_H
