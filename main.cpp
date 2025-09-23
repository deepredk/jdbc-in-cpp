#include "ODBCConnection.h"

int main() {
    OdbcConnection connection("LOCAL", "root", "");
    auto insertPstmt = connection.prepareStatement("INSERT INTO test_table4 (str, inte, lon, doub) VALUES (?, ?, ?, ?)");
    insertPstmt->setString(1, "test");
    insertPstmt->setInt(2, 2);
    insertPstmt->setLong(3, 2147483647L * 100L);
    insertPstmt->setDouble(4, 1.234567890124567);
    int insertedCount = insertPstmt->executeUpdate();
    std::cout << "inserted: " << insertedCount << std::endl;

    auto updatePstmt = connection.prepareStatement("UPDATE test_table4 SET str = ? WHERE inte = ?");
    updatePstmt->setString(1, "test23");
    updatePstmt->setInt(2, 2);
    int updatedCount = updatePstmt->executeUpdate();
    std::cout << "updated: " << updatedCount << std::endl;

    auto deletePstmt = connection.prepareStatement("DELETE FROM test_table4 WHERE inte = ?");
    deletePstmt->setInt(1, 2);
    int deletedCount = deletePstmt->executeUpdate();
    std::cout << "deleted: " << deletedCount << std::endl;

    return 0;
}
