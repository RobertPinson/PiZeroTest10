#include "PersonDetailsDialog.h"
#include "ui_PersonDetailsDialog.h"
#include <QPainter>
#include <QBitmap>

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
	QPixmap scaledImage = image.scaled(ui->lblImage->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		
	// Draw the mask.
	QBitmap mask(scaledImage.size());
	QPainter painter(&mask);
	mask.fill(Qt::white);
	painter.setBrush(Qt::black);
	painter.drawEllipse(QPoint(mask.width() / 2, mask.height() / 2), 140, 140);
	
	scaledImage.setMask(mask);
	ui->lblImage->setPixmap(scaledImage);
}

PersonDetailsDialog::~PersonDetailsDialog()
{
	delete ui;
}
