#include "PersonDetailsDialog.h"
#include "ui_PersonDetailsDialog.h"

PersonDetailsDialog::PersonDetailsDialog(QWidget *parent)
	: QDialog(parent), ui(new Ui::PersonDetailsDialog)
{
    setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_DeleteOnClose);
	
	ui->setupUi(this);	
}

void PersonDetailsDialog::setName(QString name)
{
	ui->lblName->setText(name);
}

void PersonDetailsDialog::setMessage(QString message)
{
	ui->lblWelcomeMsg->setText(message);
}

void PersonDetailsDialog::setImage(QPixmap image)
{	
	ui->lblImage->setPixmap(image.scaled(ui->lblImage->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

PersonDetailsDialog::~PersonDetailsDialog()
{
	delete ui;
}
