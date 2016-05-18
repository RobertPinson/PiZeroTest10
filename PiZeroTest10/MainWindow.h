#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "NfcReader.h"
#include "ApiClient.h"
#include "PersonDetailsDialog.h"
#include "DbManager.h"
#include "qtmosquitto.h"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

    
public:
	explicit MainWindow(QWidget *parent = 0);	
	~MainWindow();

protected slots : 
	void OnCardPresent(QString uid);
	void OnCardRemoved();
	void OnMovementResponse(Dtos::Person person);
	void OnPeopleResponse(Dtos::PeopleResponse response);
	void OnRequestError(QString message);	
	
    void doConnect();
	void doSubscribe();
	void doUnsubscribe();
	void doPublish(const QString& payload);
    
	void message(const QString& topic, const QByteArray& payload);
  
	
private:
	QtMosquittoClient* mClient;
	
	Ui::MainWindow *ui;
	NfcReader *MyNfcReader;
	ApiClient *MyApiClient;
	DbManager *MyDbManager;
	const char *userName = "apzvvubw";
	const char *deviceId = "5";
	const char *password = "bqqhHe9qGf1A";
	void ShowPersonDetailsDialog(const Dtos::Person & person);
};

#endif // MAINWINDOW_H
