#include "DbManager.h"

Q_DECLARE_METATYPE(Dtos::Movement)

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
		query.exec("CREATE TABLE IF NOT EXISTS Person (Id INTEGER PRIMARY KEY, Name TEXT, Image BLOB, InLocation INTEGER, CardId TEXT, LastUpdate TEXT)");
		query.exec("CREATE TABLE IF NOT EXISTS Movement (Id INTEGER PRIMARY KEY, CardId TEXT, SwipeTime TEXT, InLocation INTEGER)");
	}
}

bool DbManager::AddMovement(const QString &cardId, const QString &swipeTime, const int &inLocation)
{
	bool success = false;

	if (!cardId.isEmpty() && !swipeTime.isEmpty())
	{
		QSqlQuery queryAdd;

		QString query = "INSERT INTO Movement (CardId, SwipeTime, InLocation) VALUES (:cardId, :swipeTime, :inLocation);";

		queryAdd.prepare(query);
		queryAdd.bindValue(":cardId", cardId);
		queryAdd.bindValue(":swipeTime", swipeTime);
		queryAdd.bindValue(":inLocation", inLocation);

		if (queryAdd.exec())
		{
			success = true;
		}
		else
		{
			qDebug() << "Add Movement failed: " << queryAdd.lastError();
		}
	}
	else
	{
		qDebug() << "add movement failed: CardId or SwipeTime cannot be empty";
	}

	return success;
}

QList <Dtos::Movement> DbManager::GetMovements()
{
	QList <Dtos::Movement> result;
	QSqlQuery sqlQuery;

	sqlQuery.prepare("SELECT Id, CardId, SwipeTime, InLocation FROM Movement;");

	if (sqlQuery.exec())
	{
		sqlQuery.first();
		while (sqlQuery.next())
		{
			Dtos::Movement movement;
			movement.Id = sqlQuery.value("Id").toInt();
			movement.SwipeTime = sqlQuery.value("SwipeTime").toString();
			movement.CardId = sqlQuery.value("CardId").toString();
			movement.InLocation = sqlQuery.value("InLocation").toInt();

			result.append(movement);
		}
	}
	else
	{
		qDebug() << "Get Movements error: " << sqlQuery.lastError();
	}

	//qDebug() << QString("GetMovements returned %1 records").arg(result.count());

	return result;
}

bool DbManager::DeleteMovement(const int &id)
{
	bool result = false;

	if (id > 0)
	{
		QSqlQuery queryDelete;

		QString query = "DELETE FROM Movement WHERE Id = :id;";

		queryDelete.prepare(query);
		queryDelete.bindValue(":id", id);

		if (queryDelete.exec())
		{
			result = true;
		}
		else
		{
			qDebug() << "Delete Movement failed: " << queryDelete.lastError();
		}
	}
	else
	{
		qDebug() << "DeleteMovement failed: Id <= 0";
	}

	return result;
}

Dtos::Person DbManager::GetPersonByCardId(const QString &cardId)
{
	Dtos::Person result;

	qDebug() << "DB Get CardId: " + cardId;
	if (!cardId.isEmpty())
	{
		QSqlQuery sqlQuery;

		sqlQuery.prepare("SELECT Id, Name, Image, InLocation, CardId, LastUpdate FROM Person WHERE CardId LIKE :cardId;");
		sqlQuery.bindValue(":cardId", cardId);

		if (sqlQuery.exec())
		{
			if (sqlQuery.first())
			{
				qDebug() << "Name : " << sqlQuery.value("Name").toString();

				result.Id = sqlQuery.value("Id").toInt();
				result.Image = sqlQuery.value("Image").toByteArray();
				result.Name = sqlQuery.value("Name").toString();
				result.InLocation = sqlQuery.value("Inlocation").toInt() == 1 ? true : false;
				result.CardUid = sqlQuery.value("CardId").toString();
				result.LastUpdate = sqlQuery.value("LastUpdate").toString();
			}
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

		QString query1 = "INSERT or IGNORE INTO Person (Id, Name, Image, Inlocation, CardId, Lastupdate) VALUES (:id, :name, :image, :inLocation, :cardId, :lastUpdate);";
		QString query2 = "UPDATE Person SET Name = :name, Image = :image, InLocation = :inLocation, CardId = :cardId, LastUpdate = :lastUpdate WHERE changes()=0 AND Id = :id; ";

		for (size_t i = 0; i < 2; i++)
		{
			QString query = i == 0 ? query1 : query2;
			queryUpsert.prepare(query);
			queryUpsert.bindValue(":id", person.Id);
			queryUpsert.bindValue(":name", person.Name);
			queryUpsert.bindValue(":image", person.Image);
			queryUpsert.bindValue(":inLocation", person.InLocation ? 1 : 0);
			queryUpsert.bindValue(":cardId", person.CardUid);
			queryUpsert.bindValue(":lastUpdate", person.LastUpdate);

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
