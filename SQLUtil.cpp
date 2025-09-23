#include <sql.h>
#include <iostream>

#include "SQLUtil.h"

namespace SQLUtil {
    void logError(const SQLSMALLINT handleType, const SQLHANDLE handle) {
        SQLCHAR sqlState[6], message[SQL_MAX_MESSAGE_LENGTH];
        SQLINTEGER nativeError;
        SQLSMALLINT messageLength;

        const SQLRETURN diagResult = SQLGetDiagRec(
            handleType, handle, 1, sqlState, &nativeError, message, sizeof(message),&messageLength
        );

        if (diagResult == SQL_SUCCESS || diagResult == SQL_SUCCESS_WITH_INFO) {
            std::cerr << "SQL error occured. SQLSTATE: " << sqlState
                      << ", Native Error: " << nativeError
                      << ", Message: " << message << std::endl;
        } else {
            std::cerr << "Failed to get SQL error." << std::endl;
        }
    }
}
