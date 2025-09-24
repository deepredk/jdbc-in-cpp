#include "ResultSet.h"

#include <vector>
#include <stdexcept>
#include <sqlext.h>

#include "SQLUtil.h"

ResultSet::ResultSet(SQLHSTMT stmt) : statement(stmt) {}

ResultSet::~ResultSet() {
    close();
}

bool ResultSet::next() {
    if (cursorClosed) {
        return false;
    }
    const SQLRETURN rc = SQLFetch(statement);
    if (rc == SQL_NO_DATA) {
        SQLCloseCursor(statement);
        cursorClosed = true;
        return false;
    }
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
        SQLUtil::logError(SQL_HANDLE_STMT, statement);
        throw std::runtime_error("Failed to fetch next row.");
    }
    return true;
}

std::string ResultSet::getString(const int index) const {
    std::string result;
    const auto col = static_cast<SQLUSMALLINT>(index);

    constexpr SQLLEN chunkSize = 4096;
    std::vector<char> buffer(static_cast<size_t>(chunkSize) + 1);
    SQLLEN indicator = 0;

    while (true) {
        SQLRETURN rc = SQLGetData(
            statement,
            col,
            SQL_C_CHAR,
            buffer.data(),
            chunkSize,
            &indicator
        );

        if (rc == SQL_NO_DATA) {
            break;
        }
        if (indicator == SQL_NULL_DATA) {
            result.clear();
            break;
        }
        if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
            SQLUtil::logError(SQL_HANDLE_STMT, statement);
            throw std::runtime_error("Failed to get string data.");
        }

        if (indicator > 0) {
            result.append(buffer.data());
        } else {
            result.append(buffer.data());
        }

        std::ranges::fill(buffer, 0);
    }

    return result;
}

int ResultSet::getInt(const int index) const {
    SQLINTEGER value = 0;
    SQLLEN indicator = 0;
    const SQLRETURN rc = SQLGetData(statement, static_cast<SQLUSMALLINT>(index), SQL_C_LONG, &value, 0, &indicator);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
        SQLUtil::logError(SQL_HANDLE_STMT, statement);
        throw std::runtime_error("Failed to get int data.");
    }
    if (indicator == SQL_NULL_DATA) {
        throw std::runtime_error("Column is NULL (int).");
    }
    return value;
}

long ResultSet::getLong(const int index) const {
    long long value = 0;
    SQLLEN indicator = 0;
    const SQLRETURN rc = SQLGetData(statement, static_cast<SQLUSMALLINT>(index), SQL_C_SBIGINT, &value, 0, &indicator);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
        SQLUtil::logError(SQL_HANDLE_STMT, statement);
        throw std::runtime_error("Failed to get long data.");
    }
    if (indicator == SQL_NULL_DATA) {
        throw std::runtime_error("Column is NULL (long).");
    }
    return value;
}

double ResultSet::getDouble(const int index) const {
    double value = 0.0;
    SQLLEN indicator = 0;
    const SQLRETURN rc = SQLGetData(statement, static_cast<SQLUSMALLINT>(index), SQL_C_DOUBLE, &value, 0, &indicator);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
        SQLUtil::logError(SQL_HANDLE_STMT, statement);
        throw std::runtime_error("Failed to get double data.");
    }
    if (indicator == SQL_NULL_DATA) {
        throw std::runtime_error("Column is NULL (double).");
    }
    return value;
}

void ResultSet::close() {
    if (statement != SQL_NULL_HANDLE && !cursorClosed) {
        SQLCloseCursor(statement);
        cursorClosed = true;
    }
}

