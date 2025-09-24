#include "OdbcConnection.h"
#include <iostream>
#include <sqlext.h>

#include "SQLUtil.h"

OdbcConnection::OdbcConnection(const std::string& dataSourceName, const std::string& username, const std::string& password)
    : environment(SQL_NULL_HANDLE), connection(SQL_NULL_HANDLE) {

    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &environment) != SQL_SUCCESS) {
        SQLUtil::logError(SQL_HANDLE_ENV, environment);
        throw std::runtime_error("Failed to allocate environment handle.");
    }

    if (SQLSetEnvAttr(environment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0) != SQL_SUCCESS) {
        std::cout << "Failed to set ODBC version.";
        SQLUtil::logError(SQL_HANDLE_ENV, environment);
        throw std::runtime_error("Failed to set ODBC version.");
    }

    if (SQLAllocHandle(SQL_HANDLE_DBC, environment, &connection) != SQL_SUCCESS) {
        std::cout << "Failed to allocate connection handle.";
        SQLUtil::logError(SQL_HANDLE_DBC, connection);
        throw std::runtime_error("Failed to allocate connection handle.");
    }

    const SQLRETURN resultConnect = SQLConnect(connection,
                               reinterpret_cast<SQLCHAR*>(const_cast<char*>(dataSourceName.c_str())), SQL_NTS,
                               reinterpret_cast<SQLCHAR*>(const_cast<char*>(username.c_str())), SQL_NTS,
                               reinterpret_cast<SQLCHAR*>(const_cast<char*>(password.c_str())), SQL_NTS);

    if (resultConnect != SQL_SUCCESS && resultConnect != SQL_SUCCESS_WITH_INFO) {
        std::cout << "Failed to connect to database.";
        SQLUtil::logError(SQL_HANDLE_DBC, connection);
        throw std::runtime_error("Failed to connect to database.");
    }

    std::cout << "Successfully connected to the database!" << std::endl;
}

OdbcConnection::~OdbcConnection() {
    close();
}

PreparedStatement OdbcConnection::prepareStatement(const std::string& sql) const {
    SQLHSTMT statement;
    SQLAllocHandle(SQL_HANDLE_STMT, connection, &statement);
    SQLRETURN resultPrepare = SQLPrepare(statement, reinterpret_cast<SQLCHAR*>(const_cast<char*>(sql.c_str())), SQL_NTS);
    if (resultPrepare != SQL_SUCCESS && resultPrepare != SQL_SUCCESS_WITH_INFO) {
        std::cout << "Failed to prepare statement. Freeing statement handle...";
        SQLFreeHandle(SQL_HANDLE_STMT, statement);
        std::cout << "Freed statement handle." << std::endl;
        SQLUtil::logError(SQL_HANDLE_STMT, statement);
        throw std::runtime_error("Failed to prepare statement.");
    }
    return PreparedStatement(statement);
}

void OdbcConnection::close() {
    if (connection != SQL_NULL_HANDLE) {
        SQLDisconnect(connection);
        std::cout << "Disconnected from the database." << std::endl;

        SQLFreeHandle(SQL_HANDLE_DBC, connection);
        connection = SQL_NULL_HANDLE;
        std::cout << "Freed SQL connection handle." << std::endl;
    }
    if (environment != SQL_NULL_HANDLE) {
        SQLFreeHandle(SQL_HANDLE_ENV, environment);
        environment = SQL_NULL_HANDLE;
        std::cout << "Freed SQL environment handle." << std::endl;
    }
}