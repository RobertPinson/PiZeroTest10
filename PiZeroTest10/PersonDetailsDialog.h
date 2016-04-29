#ifndef PERSONDETAILSDIALOG_H
#define PERSONDETAILSDIALOG_H

#include <QDialog>
#include <QPixmap>

namespace Ui {
	class PersonDetailsDialog;
}

class PersonDetailsDialog : public QDialog
{
	Q_OBJECT

	public :
	    explicit PersonDetailsDialog(QWidget *parent = 0);
	void setName(QString name);
	void setMessage(QString message);
	void setImage(QPixmap image);

	        //setWindowFlags(windowFlags()| Qt::FramelessWindowHint);

	~PersonDetailsDialog();

private:
	Ui::PersonDetailsDialog *ui;
};

#endif // PERSONDETAILSDIALOG_H