#include "PreparedStatement.h"

#include <sql.h>
#include <sqlext.h>

#include "SQLUtil.h"

PreparedStatement::PreparedStatement(const SQLHSTMT statement) {
    this->statement = statement;
}

PreparedStatement::~PreparedStatement() {
    close();
}

void PreparedStatement::setString(const int index, const std::string value) const {
    SQLSMALLINT sqlDataType;
    SQLULEN columnSize;
    if (value.length() <= MAX_VARCHAR_LEN) {
        sqlDataType = SQL_VARCHAR;
        columnSize = value.length();
    } else {
        sqlDataType = SQL_LONGVARCHAR;
        columnSize = 0; // ignored for long types
    }

    SQLLEN length = SQL_NTS;
    SQLRETURN resultBind = SQLBindParameter(
        statement, static_cast<SQLUSMALLINT>(index), SQL_PARAM_INPUT, SQL_C_CHAR,
        sqlDataType, columnSize, 0, const_cast<char*>(value.c_str()), 0, &length
    );
    if (resultBind != SQL_SUCCESS && resultBind != SQL_SUCCESS_WITH_INFO) {
        SQLUtil::logError(SQL_HANDLE_STMT, statement);
        throw std::runtime_error("Failed to bind string.");
    }
}

void PreparedStatement::execute() const {
    const SQLRETURN resultExecute = SQLExecute(statement);
    if (resultExecute != SQL_SUCCESS && resultExecute != SQL_SUCCESS_WITH_INFO) {
        SQLUtil::logError(SQL_HANDLE_STMT, statement);
        throw std::runtime_error("Failed to execute statement.");
    }
}

void PreparedStatement::close() {
    if (statement == SQL_NULL_HANDLE) {
        return;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, statement);
    statement = SQL_NULL_HANDLE;
    std::cout << "Freed SQL statement handle." << std::endl;
}


