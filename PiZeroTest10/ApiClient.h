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
}

class ApiClient : public QObject
{	
	Q_OBJECT

		QString baseUrl;
	
public:
	ApiClient();
	~ApiClient();
	void PostMovement(QString cardId);
	void GetPeople(const QList<int>& excludeIds);
	
signals:
	void getPeopleResponse(const Dtos::PeopleResponse &);
	void movementResponse(const Dtos::Person &);
	void requestError(QString message);

protected slots:
	void onGetPeopleResponse(QNetworkReply* reply);
	void onResult(QNetworkReply* reply);
	void onError(QNetworkReply::NetworkError reply);	
};

