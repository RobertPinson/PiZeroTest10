#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "NfcReader.h"
#include "ApiClient.h"

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
    void ButtonClicked();
	void OnCardPresent(QString uid);
	void OnCardRemoved();
	void OnMovementResponse(Dtos::MovementResponse movementResponse);	

private:
    Ui::MainWindow *ui;	
};

#endif // MAINWINDOW_H
