#include "ApiClient.h"
#include <QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QUrlQuery>

Q_DECLARE_METATYPE(Dtos::MovementResponse)
	
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
		
	connect(mgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(onResult(QNetworkReply*)));	
	
	QUrl url(baseUrl + "Movement");
	QNetworkRequest request(url);	
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	
	qDebug() << "Http Post JSON";
	QByteArray data = QJsonDocument(json).toJson();
	QNetworkReply* reply = mgr->post(request, data);
	
	connect(reply,
		SIGNAL(error(QNetworkReply::NetworkError)),
		this,
		SLOT(onError(QNetworkReply::NetworkError)));
}

void ApiClient::onResult(QNetworkReply* reply)
{
	qDebug() << "Http Post Response";

	if (reply->error() != QNetworkReply::NoError)
	{
		QString message;		
		message = QString("Error calling API: ") + QString::number(reply->error());		
		emit requestError(message);	
	}
	else
	{
		Dtos::MovementResponse response;
	
		QByteArray response_data = reply->readAll();
		QJsonDocument json = QJsonDocument::fromJson(response_data);
		QJsonObject jsonObject = json.object();
	
		response.id = jsonObject["Id"].toInt();
		response.ingress = jsonObject.value("Ingress").toBool();
		response.name = jsonObject.value("Name").toString();
		response.image = QByteArray::fromBase64(jsonObject.value("Image").toString().toLatin1());	

		emit movementResponse(response);	
	}	
		
	reply->abort();
	reply->deleteLater();
	reply->manager()->deleteLater();
}

void ApiClient::onError(QNetworkReply::NetworkError err)
{
	if (err != QNetworkReply::NoError)
	{
		QString message;		
		message = QString("Error calling API: ") + QString::number(err);		
		emit requestError(message);	
	}
}

ApiClient::~ApiClient()
{
}
