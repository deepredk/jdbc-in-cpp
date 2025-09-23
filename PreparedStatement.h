#ifndef HELLO_CPP_PREPAREDSTATEMENT_H
#define HELLO_CPP_PREPAREDSTATEMENT_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <sqltypes.h>

class PreparedStatement {
public:
    explicit PreparedStatement(SQLHSTMT statement);
    ~PreparedStatement();

    PreparedStatement(const PreparedStatement&) = delete;
    PreparedStatement& operator=(const PreparedStatement&) = delete;

    PreparedStatement(PreparedStatement&& other) noexcept;
    PreparedStatement& operator=(PreparedStatement&& other) noexcept;

    void setString(int index, std::string value) const;
    void setInt(int index, int value);
    void setLong(int index, long value);
    void setDouble(int index, double value);
    void clearParameters();

    int executeUpdate();
    void execute() const;

    void close();

private:
    SQLHSTMT statement;

    constexpr static size_t MAX_VARCHAR_LEN = 4000;
};

#endif //HELLO_CPP_PREPAREDSTATEMENT_H