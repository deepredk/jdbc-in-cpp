#include "ODBCConnection.h"
#include "OdbcTemplate.h"

int main() {
    OdbcConnection connection("LOCAL", "root", "");
    OdbcTemplate odbcTemplate(connection);

    int insertedCount = odbcTemplate.update(
        "INSERT INTO test_table4 (str, inte, lon, doub) VALUES (?, ?, ?, ?)",
        "test", 1, 2147483647L * 100L, 1.2345678901234567
    );
    std::cout << "inserted: " << insertedCount << std::endl;

    long long count = odbcTemplate.queryForValue<long long>("SELECT COUNT(*) FROM test_table4");
    std::cout << "queried count: " << count << std::endl;

    int updatedCount = odbcTemplate.update("UPDATE test_table4 SET str = ? WHERE inte = ?", "test23", 1);
    std::cout << "updated: " << updatedCount << std::endl;

    int deletedCount = odbcTemplate.update("DELETE FROM test_table4 WHERE inte = ?", 2);
    std::cout << "deleted: " << deletedCount << std::endl;

    odbcTemplate.execute("DELETE FROM test_table4");

    auto selectPstmt = connection.prepareStatement("SELECT * FROM test_table4");
    auto result = selectPstmt.executeQuery();
    while (result.next()) {
        std::cout << result.getString(1) << " " << result.getInt(2) << " " << result.getLong(3) << " " << result.getDouble(4) << std::endl;
    }

    return 0;
}
