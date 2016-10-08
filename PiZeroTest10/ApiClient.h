#pragma once
#include <QtCore>
#include <QtNetwork/QNetworkReply>

namespace Dtos
{
	struct MovementRequest
	{
		int uid;
		int deviceid;
	};

	struct PeopleRequest
	{
		QStringList Ids;
	};

	struct Person
	{
		int Id;
		QString Name;
		QByteArray Image;
		bool InLocation;
		QString CardUid;
		QString LastUpdate;
	};

	struct PeopleResponse
	{
		bool IsSuccess;
		QString ErrorMessage;
		QList<Person> people;
	};

	struct MessageReceived
	{
		QString Topic;
		QString PayLoad;
	};

	struct Movement
	{
		int Id;
		QString CardId;
		QString SwipeTime;
		int InLocation;
	};
}

class ApiClient : public QObject
{
	Q_OBJECT

		QString baseUrl;

public:
	ApiClient();
	~ApiClient();
	void GetPerson(const QString& cardId);
	void GetPeople(const QList<int>& excludeIds, const int& deviceId);

signals:
	void getPeopleResponse(const Dtos::PeopleResponse &);
	void getPersonResponse(const Dtos::Person &);
	void requestError(const QString& message);

protected slots:
	void onGetPeopleResponse(QNetworkReply* reply);
	void onResult(QNetworkReply* reply);
	void onError(QNetworkReply::NetworkError reply);
};

