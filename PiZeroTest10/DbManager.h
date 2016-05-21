#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QtSql/QtSql>
#include "ApiClient.h"

class DbManager
{
public:
	DbManager(const QString &path);
	
	~DbManager();

	bool IsOpen() const;
	bool UpsertPerson(const Dtos::Person &person);
	Dtos::Person GetPersonByCardId(const QString &cardId);
	bool AddMovement(const QString &cardId, const QString &swipeTime, const int &inLocation);
	QList <Dtos::Movement> GetMovements();
	bool DeleteMovement(const int &id);
	
private:
	QSqlDatabase _db;	
};

#endif

