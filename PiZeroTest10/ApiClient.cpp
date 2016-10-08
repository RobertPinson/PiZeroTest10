#include "ApiClient.h"
#include <QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QUrlQuery>

Q_DECLARE_METATYPE(Dtos::PeopleResponse)
Q_DECLARE_METATYPE(Dtos::Person)

ApiClient::ApiClient()
{
	//baseUrl = QString("http://192.168.0.50:55577/api/");
	baseUrl = QString("http://trackeradmin.azurewebsites.net//api/");
}

void ApiClient::GetPeople(const QList<int>& excludeIds, const int& deviceId)
{
	QJsonObject json;
	QString exIds;

	for (int i = 0; i < excludeIds.size(); i++)
	{
		exIds += QString::number(excludeIds[i]);
		if (i < excludeIds.size() - 1)
			exIds += ",";
	}

	json.insert("ExcludeIds", exIds);
	json.insert("DeviceId", deviceId);

	QNetworkAccessManager* mgr = new QNetworkAccessManager(this);

	connect(mgr,
		SIGNAL(finished(QNetworkReply*)),
		this,
		SLOT(onGetPeopleResponse(QNetworkReply*)));

	QUrl url(baseUrl + "People");
	QUrlQuery query;
	query.addQueryItem("deviceId", QString::number(deviceId));
	query.addQueryItem("excludeids", exIds);

	url.setQuery(query);

	QNetworkRequest request(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	qDebug() << "Http GET People";
	QByteArray data = QJsonDocument(json).toJson();
	QNetworkReply* reply = mgr->get(request);

	connect(reply,
		SIGNAL(error(QNetworkReply::NetworkError)),
		this,
		SLOT(onError(QNetworkReply::NetworkError)));
}

void ApiClient::onGetPeopleResponse(QNetworkReply* reply)
{
	qDebug() << "Http GET People response";

	if (reply->error() != QNetworkReply::NoError)
	{
		QString message;
		message = QString("Error calling API GET People: ") + QString::number(reply->error());
		emit requestError(message);
	}
	else
	{
		//Get data
		Dtos::PeopleResponse peopleResponse;

		QByteArray responseData = reply->readAll();

		if (responseData.isEmpty())
		{
			qDebug() << "Get People returned no data.";

			peopleResponse.IsSuccess = false;
			peopleResponse.ErrorMessage = "No Data Returned";

			emit getPeopleResponse(peopleResponse);

			return;
		}

		QJsonParseError error;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &error);

		if (error.error != QJsonParseError::NoError)
		{
			qDebug() << "Get People Json Parse error.";

			peopleResponse.IsSuccess = false;
			peopleResponse.ErrorMessage = "Json Parse Error";

			emit getPeopleResponse(peopleResponse);

			return;
		}

		if ((jsonDoc.isNull() || jsonDoc.isEmpty()))
		{
			qDebug() << "Get People Json is empty.";

			peopleResponse.IsSuccess = false;
			peopleResponse.ErrorMessage = "Json is empty";

			emit getPeopleResponse(peopleResponse);

			return;
		}

		QJsonObject jsonObject = jsonDoc.object();
		QJsonArray jsonArray = jsonObject["people"].toArray();

		foreach(const QJsonValue & val, jsonArray)
		{
			QJsonObject obj = val.toObject();
			Dtos::Person person;
			person.Id = obj["id"].toInt();
			person.Name = obj["name"].toString();
			person.Image = QByteArray::fromBase64(obj["image"].toString().toLatin1());
			person.CardUid = obj["cardUid"].toString();
			person.InLocation = obj["inLocation"].toBool();
			person.LastUpdate = QDateTime::fromString(obj["swipeTime"].toString(),Qt::ISODate).toString(Qt::ISODate);

			//qDebug() << "GET [People] LastUpdate: " + QDateTime::fromString(obj["swipeTime"].toString(), Qt::ISODate).toString(Qt::ISODate);

			peopleResponse.people.append(person);
		}

		qDebug() << "GET People returned: " + QString::number(peopleResponse.people.count());

		peopleResponse.IsSuccess = true;
		emit getPeopleResponse(peopleResponse);
	}
	reply->abort();
	reply->deleteLater();
	reply->manager()->deleteLater();
}

void ApiClient::GetPerson(const QString& cardId)
{
	QJsonObject json;
	json.insert("CardUid", cardId);

	QNetworkAccessManager* mgr = new QNetworkAccessManager(this);

	connect(mgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(onResult(QNetworkReply*)));
	QUrlQuery query;
	query.addQueryItem("cardId", cardId);
	QUrl url(baseUrl + "GetPerson");
	url.setQuery(query.query());
	QNetworkRequest request(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	qDebug() << "Http Post JSON";
	QByteArray data = QJsonDocument(json).toJson();
	QNetworkReply* reply = mgr->get(request);

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
		Dtos::Person person;

		QByteArray response_data = reply->readAll();
		QJsonDocument json = QJsonDocument::fromJson(response_data);
		QJsonObject jsonObject = json.object();

		person.Id = jsonObject["id"].toInt();
		person.CardUid = jsonObject["cardUid"].toString();
		person.InLocation = jsonObject.value("ingress").toBool();
		person.Name = jsonObject.value("name").toString();
		person.Image = QByteArray::fromBase64(jsonObject.value("image").toString().toLatin1());
		person.LastUpdate = QDateTime::fromString(jsonObject["swipeTime"].toString()).toString(Qt::ISODate);

		emit getPersonResponse(person);
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