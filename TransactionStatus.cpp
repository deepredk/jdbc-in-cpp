#include "TransactionStatus.h"

#include <iostream>
#include <ostream>
#include <sqlext.h>
#include <stdexcept>

#include "SQLUtil.h"

void TransactionStatus::setAutoCommit(bool enable) const {
    SQLUINTEGER mode = enable ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF;
    const SQLRETURN rc = SQLSetConnectAttr(connection, SQL_ATTR_AUTOCOMMIT,
                                           reinterpret_cast<SQLPOINTER>(static_cast<uintptr_t>(mode)),
                                           0);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
        SQLUtil::logError(SQL_HANDLE_DBC, connection);
        throw std::runtime_error("Failed to set autocommit mode.");
    }
}

void TransactionStatus::commit() const {
    const SQLRETURN rc = SQLEndTran(SQL_HANDLE_DBC, connection, SQL_COMMIT);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
        SQLUtil::logError(SQL_HANDLE_DBC, connection);
        throw std::runtime_error("Failed to commit transaction.");
    }
    // std::cout << "Transaction committed." << std::endl;
}

void TransactionStatus::rollback() const {
    const SQLRETURN rc = SQLEndTran(SQL_HANDLE_DBC, connection, SQL_ROLLBACK);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
        SQLUtil::logError(SQL_HANDLE_DBC, connection);
        throw std::runtime_error("Failed to rollback transaction.");
    }
    // std::cout << "Transaction rolled back." << std::endl;
}