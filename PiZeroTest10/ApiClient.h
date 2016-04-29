#pragma once
#include <QtCore>
#include <QtNetwork/QNetworkReply>

namespace Dtos
{
	struct MovementResponse
	{
		int id;
		QString name;
		QByteArray image;
		bool ingress;
	}
	;
	
	struct MovementRequest
	{
		int uid;
		int deviceid;
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
	
signals:
	void movementResponse(const Dtos::MovementResponse &);	
	
	protected slots :	
		void OnResult(QNetworkReply* reply);
	void handleSSLErrors(QNetworkReply* reply);
};

