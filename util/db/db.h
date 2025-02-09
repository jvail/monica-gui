/**
Authors: 
Michael Berg <michael.berg@zalf.de>

Maintainers: 
Currently maintained by the authors.

This file is part of the util library used by models created at the Institute of 
Landscape Systems Analysis at the ZALF.
Copyright (C) 2007-2013, Leibniz Centre for Agricultural Landscape Research (ZALF)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __DB_H__
#define __DB_H__

#ifndef NO_MYSQL
#ifdef WIN32
#ifdef WINSOCK2
#include <WinSock2.h>
#else
#include <WinSock.h>
#endif //WINSOCK2
#include "mysql.h"
#else
#include <mysql/mysql.h>
#endif
#endif

#include <list>
#include <vector>
#include <string>

#ifndef Q_MOC_RUN
#include <boost/shared_ptr.hpp>
#endif //Q_MOC_RUN

#include "tools/date.h"

#ifndef NO_SQLITE
#include "sqlite3.h"
#endif

namespace Db
{
	typedef std::vector<std::string> DBRow;

	class DB
	{
	public:
		virtual ~DB() {}
		
		virtual bool exec(const char* sqlStatement) = 0;
		virtual bool exec(const std::string& sqlStatement) { return exec(sqlStatement.c_str()); }

		virtual bool select(const char* selectStatement){ return exec(selectStatement); }
		virtual bool select(const std::string& selectStatement){ return exec(selectStatement.c_str()); }

		virtual bool query(const char* sqlStatement){ return exec(sqlStatement); }
		virtual bool query(const std::string& sqlStatement){ return exec(sqlStatement.c_str()); }

		virtual bool insert(const char* insertStatement){ return exec(insertStatement); }
		virtual bool insert(const std::string& insertStatement){ return insert(insertStatement.c_str()); }

		virtual bool update(const char* updateStatement){ return exec(updateStatement); }
		virtual bool update(const std::string& updateStatement){ return update(updateStatement.c_str()); }

		virtual bool del(const char* deleteStatement){ return exec(deleteStatement); }
		virtual bool del(const std::string& deleteStatement){ return del(deleteStatement.c_str()); }


		virtual unsigned int getNumberOfFields() = 0;
		//MYSQL_FIELD* getFields();
		//MYSQL_FIELD* getNextField();
		virtual unsigned long getNumberOfRows() = 0;
		virtual DBRow getRow() = 0;
		virtual void freeResultSet() = 0;

		virtual bool isConnected() = 0; //{ return _isConnected; }

		virtual int insertId() = 0;

		virtual std::string errorMsg() { return std::string(); }

		virtual std::string now() const = 0;

		virtual std::string toDBDate(Tools::Date date) const = 0;

		virtual void setCharacterSet(const char* charsetName) = 0;
		virtual void setCharacterSet(const std::string charsetName) { setCharacterSet(charsetName.c_str()); }

		virtual DB* clone() const = 0; // { return new DB(_host, _user, _pwd, _schema, _port); }

		//! if supported by database, attach another db to the current one
		virtual bool attachDB(std::string /*pathToDB*/, std::string /*alias*/){ return false; }
	};

#ifndef NO_MYSQL
	class MysqlDB : public DB
	{
	public:
		MysqlDB(const std::string& host, const std::string& user, const std::string& pwd,
						const std::string& schema, unsigned int port = 0);
		virtual ~MysqlDB();
		virtual bool exec(const char* query);
		virtual unsigned int getNumberOfFields();
		MYSQL_FIELD* getFields();
		MYSQL_FIELD* getNextField();
		virtual unsigned long getNumberOfRows();
		virtual DBRow getRow();
		MYSQL_ROW getMysqlRow();
		virtual void freeResultSet();

		virtual bool isConnected(){ return _isConnected; }

		virtual int insertId();

		virtual std::string now() const { return "now()"; }

		virtual std::string toDBDate(Tools::Date date) const;

		virtual void setCharacterSet(const char* charsetName);

		virtual MysqlDB* clone() const { return new MysqlDB(_host, _user, _pwd, _schema, _port); }

	private: //methods
		void lazyInit();
		void init();

	private:
		std::string _host;
		std::string _user;
		std::string _pwd;
		std::string _schema;
		unsigned int _port;

		MYSQL* _connection;
		MYSQL_RES* _resultSet;
		bool _isConnected;
		long _id;
		bool _initialized;
	};
#endif

	//------------------------------------------------------------------------

#ifndef NO_SQLITE
	class SqliteDB : public DB
	{
	public:
		SqliteDB(const std::string& filename);
		virtual ~SqliteDB();
		virtual bool exec(const char* sqlStatement);
		virtual bool insert(const char* insertStatement){ return inUpDel(insertStatement); }
		virtual bool update(const char* updateStatement){ return inUpDel(updateStatement); }
		virtual bool del(const char* deleteStatement){ return inUpDel(deleteStatement); }

		virtual unsigned int getNumberOfFields();
		virtual unsigned long getNumberOfRows();
		virtual DBRow getRow();

		virtual void freeResultSet();

		virtual bool isConnected(){ return _isConnected; }

		virtual int insertId();

		virtual std::string errorMsg();

		virtual std::string now() const { return "datetime('now')"; }

		virtual std::string toDBDate(Tools::Date date) const;

		virtual void setCharacterSet(const char* charsetName);

		virtual DB* clone() const { return new SqliteDB(_filename); }


		virtual bool attachDB(std::string pathToDB, std::string alias);

	private: //methods
		void lazyInit();
		void init();

		void addNeededSQLFunctions();

		bool inUpDel(const char* inUpDelStatement);

	private:
		std::string _filename;
		sqlite3* _db;
		sqlite3_stmt* _ppStmt;
		std::string _query;
		bool _isConnected;
		long _id;
		bool _initialized;
		int _currentRowNo;
		int _noOfRows;
	};
#endif

	//----------------------------------------------------------------------------

	struct DBConData
	{
		DBConData() : maxNoOfConnections(0) {}
		//! construct a Mysql connection
		DBConData(const std::string& host, const std::string& user,
							const std::string& pwd, const std::string& schema,
							unsigned int port = 0)
			: host(host), user(user), pwd(pwd), schema(schema),
				maxNoOfConnections(1), port(port){}

		//! construct a SqliteDB connection
		DBConData(const std::string& filename) : filename(filename) {}
		std::string host;
		std::string user;
		std::string pwd;
		std::string schema;
		std::string filename;
		int maxNoOfConnections;
		unsigned int port;
		bool isValid() const
		{
			return !filename.empty() || (!host.empty() && !user.empty() && !pwd.empty() && !schema.empty());
		}
		bool isSqliteDB() const { return isValid() && !filename.empty(); }
		bool isMysqlDB() const { return !isSqliteDB(); }

		std::string toString() const;
	};

	//----------------------------------------------------------------------------

	typedef boost::shared_ptr<DB> DBPtr;

	//! user takes ownership
	DB* newConnection(const DBConData& connectionData);

#ifndef NO_MYSQL
	typedef boost::shared_ptr<MysqlDB> MysqlDBPtr;
	inline MysqlDB* toMysqlDB(DB* db){ return dynamic_cast<MysqlDB*>(db); }
#endif

}

#endif
