#ifndef HELLO_CPP_ODBCCONNECTION_H
#define HELLO_CPP_ODBCCONNECTION_H

#include <sql.h>
#include <string>

class OdbcConnection {
public:
    OdbcConnection(const std::string& dataSourceName, const std::string& username, const std::string& password);
    ~OdbcConnection();

    OdbcConnection(const OdbcConnection&) = delete;
    OdbcConnection& operator=(const OdbcConnection&) = delete;

private:
    SQLHENV environment;
    SQLHDBC connection;

    static SQLCHAR* toSQLCHARPtr(const std::string &str);
};

#endif