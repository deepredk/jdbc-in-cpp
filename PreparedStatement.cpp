#include "PreparedStatement.h"

#include <sql.h>
#include <sqlext.h>

#include "SQLUtil.h"

PreparedStatement::PreparedStatement(SQLHSTMT statement) : statement(statement) {}

PreparedStatement::~PreparedStatement() {
    close();
}

void PreparedStatement::setString(const int index, const std::string& value) {
    SQLSMALLINT sqlDataType;
    SQLULEN columnSize;

    parameters.emplace(index, value);
    const auto it = parameters.find(index);
    const auto* storedStringPtr = std::get_if<std::string>(&it->second);

    if (storedStringPtr->length() <= MAX_VARCHAR_LEN) {
        sqlDataType = SQL_VARCHAR;
        columnSize = storedStringPtr->length();
    } else {
        sqlDataType = SQL_LONGVARCHAR;
        columnSize = 0; // ignored for long types
    }

    static SQLLEN length = SQL_NTS;
    SQLRETURN resultBind = SQLBindParameter(
        statement,
        static_cast<SQLUSMALLINT>(index),
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        sqlDataType,
        columnSize,
        0,
        const_cast<char*>(storedStringPtr->c_str()),
        static_cast<SQLLEN>(storedStringPtr->size()),
        &length
    );
    if (resultBind != SQL_SUCCESS && resultBind != SQL_SUCCESS_WITH_INFO) {
        SQLUtil::logError(SQL_HANDLE_STMT, statement);
        throw std::runtime_error("Failed to bind string.");
    }
}

void PreparedStatement::setInt(int index, int value) {
    parameters.emplace(index, value);
    const auto it = parameters.find(index);
    const int* storedIntPtr = std::get_if<int>(&it->second);

    static SQLLEN length = 0;
    SQLRETURN resultBind = SQLBindParameter(
        statement,
        static_cast<SQLUSMALLINT>(index),
        SQL_PARAM_INPUT,
        SQL_C_LONG,
        SQL_INTEGER,
        0,
        0,
        const_cast<int*>(storedIntPtr),
        0,
        &length
    );
    if (resultBind != SQL_SUCCESS && resultBind != SQL_SUCCESS_WITH_INFO) {
        SQLUtil::logError(SQL_HANDLE_STMT, statement);
        throw std::runtime_error("Failed to bind integer.");
    }
}

void PreparedStatement::setLong(int index, long long value) {
    parameters.emplace(index, value);
    const auto it = parameters.find(index);
    const long long* storedLongPtr = std::get_if<long long>(&it->second);

    static SQLLEN length = 0;
    SQLRETURN resultBind = SQLBindParameter(
        statement,
        static_cast<SQLUSMALLINT>(index),
        SQL_PARAM_INPUT,
        SQL_C_SBIGINT,
        SQL_BIGINT,
        0,
        0,
        const_cast<long long*>(storedLongPtr),
        0,
        &length
    );
    if (resultBind != SQL_SUCCESS && resultBind != SQL_SUCCESS_WITH_INFO) {
        SQLUtil::logError(SQL_HANDLE_STMT, statement);
        throw std::runtime_error("Failed to bind integer.");
    }
}

void PreparedStatement::setDouble(int index, double value) {
    parameters.emplace(index, value);
    const auto it = parameters.find(index);
    const double* storedDoublePtr = std::get_if<double>(&it->second);

    static SQLLEN length = 0;
    SQLRETURN resultBind = SQLBindParameter(
        statement,
        static_cast<SQLUSMALLINT>(index),
        SQL_PARAM_INPUT,
        SQL_C_DOUBLE,
        SQL_DOUBLE,
        0,
        0,
        const_cast<double*>(storedDoublePtr),
        0,
        &length
    );
    if (resultBind != SQL_SUCCESS && resultBind != SQL_SUCCESS_WITH_INFO) {
        SQLUtil::logError(SQL_HANDLE_STMT, statement);
        throw std::runtime_error("Failed to bind double.");
    }
}

ResultSet PreparedStatement::executeQuery() const {
    execute();
    return ResultSet(statement);
}

int PreparedStatement::executeUpdate() const {
    execute();

    SQLLEN rowsAffected;
    SQLRETURN resultRowCount = SQLRowCount(statement, &rowsAffected);
    if (resultRowCount != SQL_SUCCESS && resultRowCount != SQL_SUCCESS_WITH_INFO) {
        SQLUtil::logError(SQL_HANDLE_STMT, statement);
        throw std::runtime_error("Failed to get row count.");
    }

    return static_cast<int>(rowsAffected);
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
    // std::cout << "Freed SQL statement handle." << std::endl;
}


