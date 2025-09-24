#ifndef JDBC_IN_CPP_RESULTSET_H
#define JDBC_IN_CPP_RESULTSET_H

#include <string>
#include <sql.h>

class ResultSet {
public:
    explicit ResultSet(SQLHSTMT stmt);
    ~ResultSet();

    bool next();
    [[nodiscard]] std::string getString(int index) const;
    [[nodiscard]] int getInt(int index) const;
    [[nodiscard]] long getLong(int index) const;
    [[nodiscard]] double getDouble(int index) const;
    void close();

private:
    SQLHSTMT statement;
    bool cursorClosed = false;
};

#endif //JDBC_IN_CPP_RESULTSET_H