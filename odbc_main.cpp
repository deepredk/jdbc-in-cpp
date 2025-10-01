#ifdef IGNORE

#include "ODBCConnection.h"
#include "OdbcTemplate.h"

// create table test_table4(str varchar(300), inte int, lon bigint, doub double);
int odbc_main() {
    OdbcConnection connection("LOCAL", "root", "");
    auto txStatus = connection.getTransactionStatus();
    txStatus.setAutoCommit(false);

    OdbcTemplate odbcTemplate(connection);

    int insertedCount = odbcTemplate.update(
        "INSERT INTO test_table4 (str, inte, lon, doub) VALUES (?, ?, ?, ?)",
        "test", 3, 2147483647L * 100L, 1.2345678901234567
    );
    std::cout << "inserted: " << insertedCount << std::endl;

    long long count = odbcTemplate.queryForValue<long long>("SELECT COUNT(*) FROM test_table4");
    std::cout << "queried count: " << count << std::endl;

    int updatedCount = odbcTemplate.update("UPDATE test_table4 SET str = ? WHERE inte = ?", "test23", 1);
    std::cout << "updated: " << updatedCount << std::endl;

    int deletedCount = odbcTemplate.update("DELETE FROM test_table4 WHERE inte = ?", 2);
    std::cout << "deleted: " << deletedCount << std::endl;

    // odbcTemplate.execute("DELETE FROM test_table4");

    struct TestTable4Dto {
        std::string str;
        int inte;
        long lon;
        double doub;
    };
    auto testTable4RowMapper = [](const ResultSet& rs) {
        return TestTable4Dto{
            rs.getString(1),
            rs.getInt(2),
            rs.getLong(3),
            rs.getDouble(4)
        };
    };

    auto rows = odbcTemplate.query<TestTable4Dto>(
        "SELECT * FROM test_table4",
        testTable4RowMapper
    );
    for (const auto& row : rows) {
        std::cout << "query: " << row.str << " " << row.inte << " " << row.lon << " " << row.doub << std::endl;
    }

    auto row = odbcTemplate.queryForObject<TestTable4Dto>(
        "SELECT * FROM test_table4 WHERE inte = ?",
        testTable4RowMapper,
        3
    );
    std::cout << "queryForObject: " << row.str << " " << row.inte << " " << row.lon << row.doub << std::endl;

    auto intes = odbcTemplate.query<double>(
        "SELECT inte FROM test_table4",
        [](const ResultSet& rs) {
            return rs.getDouble(1);
        }
    );
    for (const auto& inte : intes) {
        std::cout << "query(value): " << inte << std::endl;
    }
    txStatus.commit();
    txStatus.rollback();

    return 0;
}
#endif
