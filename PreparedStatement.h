#ifndef HELLO_CPP_PREPAREDSTATEMENT_H
#define HELLO_CPP_PREPAREDSTATEMENT_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <variant>
#include <sqltypes.h>

#include "ResultSet.h"

class PreparedStatement {
public:
    explicit PreparedStatement(SQLHSTMT statement);
    ~PreparedStatement();

    void setString(int index, const std::string& value);
    void setInt(int index, int value);
    void setLong(int index, long long value);
    void setDouble(int index, double value);

    [[nodiscard]] ResultSet executeQuery() const;
    [[nodiscard]] int executeUpdate() const;
    void execute() const;

    void close();

private:
    SQLHSTMT statement;
    std::unordered_map<int, std::variant<int, long long, double, std::string>> parameters;

    constexpr static size_t MAX_VARCHAR_LEN = 4000;
};

#endif //HELLO_CPP_PREPAREDSTATEMENT_H