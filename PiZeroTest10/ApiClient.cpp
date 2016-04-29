#include "ApiClient.h"
#include <QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QUrlQuery>


ApiClient::ApiClient()
{
	//http://192.168.0.23:8089/api/
	baseUrl = QString("http://trackerdemosite.azurewebsites.net/api/");
}

void ApiClient::PostMovement(QString cardId)
{
	QJsonObject json;
	json.insert("Uid", cardId);
	json.insert("DeviceId", 5);	

	QNetworkAccessManager* mgr = new QNetworkAccessManager(this);
		
	connect(mgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(OnResult(QNetworkReply*)));	
	
	QUrl url(baseUrl + "Movement");
	QNetworkRequest request(url);	
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	
	qDebug() << "Http Post JSON";
	QByteArray data = QJsonDocument(json).toJson();
	QNetworkReply* currentReply = mgr->post(request, data);
}

void ApiClient::handleSSLErrors(QNetworkReply* reply)
{
	qDebug() << "Http Post SSL Error";
}

void ApiClient::OnResult(QNetworkReply* reply)
{
	qDebug() << "Http Post Response";

	if (reply->error() != QNetworkReply::NoError)
		return;  // ...only in a blog post emit error signal
	
	Dtos::MovementResponse response;
	
	QByteArray response_data = reply->readAll();
	QJsonDocument json = QJsonDocument::fromJson(response_data);
	QJsonObject jsonObject = json.object();
	
	response.id = jsonObject["Id"].toInt();
	response.ingress = jsonObject.value("Ingress").toBool();
	response.name = jsonObject.value("Name").toString();
	response.image = jsonObject.value("Image").toString().toUtf8();	

	emit movementResponse(response);
		
	reply->abort();
	reply->deleteLater();
	reply->manager()->deleteLater();
}

ApiClient::~ApiClient()
{
}
