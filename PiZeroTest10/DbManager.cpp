#include "DbManager.h"

DbManager::DbManager(const QString &path)
{
	_db = QSqlDatabase::addDatabase("QSQLITE");
	_db.setDatabaseName(path);
 
	if (!_db.open())
	{
		qDebug() << "Error: connection with database fail";
	}
	else
	{
		qDebug() << "Database: connection ok";
		
		//Create tables
		QSqlQuery query = QSqlQuery(_db);
		query.exec("CREATE TABLE IF NOT EXISTS Person (Id INTEGER PRIMARY KEY, Name TEXT, Image BLOB, InLocation INTEGER, CardId TEXT)");		
	}
}

Dtos::Person DbManager::GetPersonByCardId(const QString &cardId)
{
	Dtos::Person result;
	
	qDebug() << "DB Get CardId: " + cardId;
	if (!cardId.isEmpty())
	{
		QSqlQuery sqlQuery;		
		
		sqlQuery.prepare("SELECT Id, Name, Image, InLocation, CardId FROM Person WHERE CardId LIKE :cardId;");
		sqlQuery.bindValue(":cardId", cardId);
				
		if (sqlQuery.exec())
		{			
			sqlQuery.first();
			qDebug() << "Name : " << sqlQuery.value("Name").toString();
			
			result.Id = sqlQuery.value("Id").toInt();
			result.Image = sqlQuery.value("Image").toByteArray();
			result.Name = sqlQuery.value("Name").toString();
			result.InLocation = sqlQuery.value("Inlocation").toInt() == 1 ? true : false;
			result.CardUid = sqlQuery.value("CardId").toString();
		}
		else
		{
			qDebug() << "Get Person By CardID error: " << sqlQuery.lastError();
		}		
	}
	
	return result;
}

bool DbManager::UpsertPerson(const Dtos::Person& person)
{
	bool success = false;

	if (!person.Name.isEmpty() && person.Id > 0)
	{		
		QSqlQuery queryUpsert;
		
		QString query1 = "INSERT or IGNORE INTO Person (Id, Name, Image, Inlocation, CardId) VALUES (:id, :name, :image, :inLocation, :cardId);"; 
		QString query2 = "UPDATE Person SET Name = :name, Image = :image, InLocation = :inLocation, CardId = :cardId WHERE changes()=0 AND Id = :id; ";
		
		for (size_t i = 0; i < 2; i++)
		{
			QString query = i == 0 ? query1 : query2;
			queryUpsert.prepare(query);		
			queryUpsert.bindValue(":id", person.Id);
			queryUpsert.bindValue(":name", person.Name);
			queryUpsert.bindValue(":image", person.Image);
			queryUpsert.bindValue(":inLocation", person.InLocation ? 1 : 0);
			queryUpsert.bindValue(":cardId", person.CardUid);
			
			if (queryUpsert.exec())
			{
				success = true;
			}
			else
			{
				qDebug() << "Upsert person failed: " << queryUpsert.lastError();
			}
		}	
	}
	else
	{
		qDebug() << "add person failed: name cannot be empty";
	}

	return success;
}

bool DbManager::IsOpen() const
{
	return _db.isOpen();
}

DbManager::~DbManager()
{
	if (_db.isOpen())
	{
		_db.close();
	}
}
