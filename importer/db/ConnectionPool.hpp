#pragma once

#include <string>
#include <cassert>
#include <thread>
#include <chrono>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <iostream>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include "ConfigFile.h"
#include "Logger.h"

using namespace sql;
using namespace std;

class ConnectionPool {

public:
	static ConnectionPool* getInstance() {
		static ConnectionPool instance;
		return &instance;
	}

	sql::Connection* getConnection() {
		sql::Connection* con = NULL;
		while (con == NULL) {
			try {
				con = driver->connect(url, username, password);
			} catch (sql::SQLException &e) {
				cout << "# ERR: SQLException in " << __FILE__;
				cout << "(" << __FUNCTION__ << ") on line "
				    	<< __LINE__ << endl;
				cout << "# ERR: " << e.what();
				cout << " (MySQL error code: " << e.getErrorCode();
			}
			if (con) break;
			// wait for 50 ms before retry
			LOG(ERROR) << "Cannot get connection to database...retrying...";
			this_thread::sleep_for(chrono::milliseconds(50));
		}
		return con;
	}

	static bool close(Connection* conn, PreparedStatement* ps, ResultSet* rs) {
		try {
			if (rs != NULL) {
				rs->close();
				delete rs;
			}

			if (ps != NULL) {
				ps->close();
				delete ps;
			}

            if (conn != NULL) {
                conn->close();
                delete conn;
            }

			return true;
		}
		catch (sql::SQLException &e) {
			cout << "# ERR: SQLException in " << __FILE__;
			cout << "(" << __FUNCTION__ << ") on line "
				<< __LINE__ << endl;
			cout << "# ERR: " << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
		}
		return false;
	}

	static bool close(Connection* conn, Statement* ps, ResultSet* rs) {
		try {
			if (rs) {
				rs->close();
				delete rs;
			}

			if (ps) {
				ps->close();
				delete ps;
			}

            if (conn) {
                conn->close();
                delete conn;
            }

			return true;
		}
		catch (sql::SQLException &e) {
			cout << "# ERR: SQLException in " << __FILE__;
			cout << "(" << __FUNCTION__ << ") on line "
				<< __LINE__ << endl;
			cout << "# ERR: " << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
		}
		return false;
	}
private:
	ConnectionPool()
	{


		ConfigFile config;

		config.readInto(username,"username");
		config.readInto(password,"password");
		config.readInto(url, "url");

		try {
			driver = sql::mysql::get_mysql_driver_instance();
		} catch (sql::SQLException &e) {
			cout << "# ERR: SQLException in " << __FILE__;
			cout << "(" << __FUNCTION__ << ") on line "
				<< __LINE__ << endl;
			cout << "# ERR: " << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
		}
	}

private:
	mysql::MySQL_Driver* driver;
	string username;
	string password;
	string url;
	unsigned int keepalivetimeout;

};
