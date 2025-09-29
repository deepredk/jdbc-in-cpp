#ifndef HELLO_CPP_ODBCCONNECTION_H
#define HELLO_CPP_ODBCCONNECTION_H

#include <sql.h>
#include <string>

#include "PreparedStatement.h"
#include "TransactionStatus.h"

class OdbcConnection {
public:
    OdbcConnection(const std::string& dataSourceName, const std::string& username, const std::string& password);
    ~OdbcConnection();

    OdbcConnection(const OdbcConnection&) = delete;
    OdbcConnection& operator=(const OdbcConnection&) = delete;

    [[nodiscard]] PreparedStatement prepareStatement(const std::string& sql) const;

    [[nodiscard]] TransactionStatus getTransactionStatus() const;

    void close();

private:
    SQLHENV environment;
    SQLHDBC connection;
};

#endif