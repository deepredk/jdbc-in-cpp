#include "ODBCConnection.h"

int main() {
    OdbcConnection connection("LOCAL", "root", "");
    auto pstmt = connection.prepareStatement("INSERT INTO test_table (id) VALUES (?)");
    pstmt->setString(1, "test");
    pstmt->execute();
    pstmt->close();
    connection.close();
    return 0;
}
