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
	void UpdatePerson(const Dtos::Person &person);
private:
	QSqlDatabase _db;	
};

#endif

