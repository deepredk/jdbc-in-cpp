#include "OdbcConnection.h"
#include <iostream>
#include <sqlext.h>

SQLCHAR* OdbcConnection::toSQLCHARPtr(const std::string &str) {
    return reinterpret_cast<SQLCHAR *>(const_cast<char *>(str.c_str()));
}

OdbcConnection::OdbcConnection(const std::string& dataSourceName, const std::string& username, const std::string& password)
    : environment(SQL_NULL_HANDLE), connection(SQL_NULL_HANDLE) {

    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &environment) != SQL_SUCCESS) {
        throw std::runtime_error("Failed to allocate environment handle.");
    }

    if (SQLSetEnvAttr(environment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0) != SQL_SUCCESS) {
        SQLFreeHandle(SQL_HANDLE_ENV, environment);
        throw std::runtime_error("Failed to set ODBC version.");
    }

    if (SQLAllocHandle(SQL_HANDLE_DBC, environment, &connection) != SQL_SUCCESS) {
        SQLFreeHandle(SQL_HANDLE_ENV, environment);
        throw std::runtime_error("Failed to allocate connection handle.");
    }

    const SQLRETURN resultConnect = SQLConnect(connection,
                               toSQLCHARPtr(dataSourceName), SQL_NTS,
                               toSQLCHARPtr(username), SQL_NTS,
                               toSQLCHARPtr(password), SQL_NTS);

    if (resultConnect != SQL_SUCCESS && resultConnect != SQL_SUCCESS_WITH_INFO) {
        SQLFreeHandle(SQL_HANDLE_DBC, connection);
        SQLFreeHandle(SQL_HANDLE_ENV, environment);
        throw std::runtime_error("Failed to connect to database.");
    }

    std::cout << "Successfully connected to the database!" << std::endl;
}

OdbcConnection::~OdbcConnection() {
    if (connection != SQL_NULL_HANDLE) {
        SQLDisconnect(connection);
        SQLFreeHandle(SQL_HANDLE_DBC, connection);
        std::cout << "Disconnected from the database and freed connection handle." << std::endl;
    }
    if (environment != SQL_NULL_HANDLE) {
        SQLFreeHandle(SQL_HANDLE_ENV, environment);
        std::cout << "Freed SQL environment handle." << std::endl;
    }
}
