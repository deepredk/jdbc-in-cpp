#ifndef JDBC_IN_CPP_TRASNACTIONSTATUS_H
#define JDBC_IN_CPP_TRASNACTIONSTATUS_H

#include <sql.h>

class TransactionStatus {
public:
    explicit TransactionStatus(SQLHDBC connection) : connection(connection) {}

    void setAutoCommit(bool enable) const;

    void commit() const;

    void rollback() const;

private:
    SQLHDBC connection;
};

#endif // JDBC_IN_CPP_TRASNACTIONSTATUS_H
