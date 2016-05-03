#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "NfcReader.h"
#include "ApiClient.h"
#include "PersonDetailsDialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
		
	NfcReader *MyNfcReader;
	ApiClient *MyApiClient;	
    
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
	
};

#endif // MAINWINDOW_H
