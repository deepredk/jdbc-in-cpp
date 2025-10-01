#include "ODBCConnection.h"
#include "OdbcTemplate.h"
#include <thread>

int getValue(OdbcTemplate& odbcTemplate, int id) {
    return odbcTemplate.queryForValue<int>("select value from transaction_test where id = ?", id);
}

void incrementTenTimes(int id, bool useTransaction) {
    OdbcConnection connection("LOCAL", "root", "");
    auto txStatus = connection.getTransactionStatus();
    if (useTransaction) txStatus.setAutoCommit(false);

    OdbcTemplate odbcTemplate(connection);

    for (int i = 0; i < 10; i++) {
        int currentValue = getValue(odbcTemplate, id);
        std::cout << "currentValue is " << currentValue << ". set value = " << currentValue + 1 << std::endl;
        odbcTemplate.execute("update transaction_test set value = ? where id = ?", currentValue + 1, id);
        if (useTransaction) txStatus.commit();
    }
}

int getValue(int id) {
    OdbcConnection connection("LOCAL", "root", "");
    OdbcTemplate odbcTemplate(connection);
    return getValue(odbcTemplate, id);
}


// create table transaction_test(id int not null primary key auto_increment, value int not null);
int main() {
    std::cout << "useTransaction? (1 or 0): ";
    bool useTransaction;
    std::cin >> useTransaction;

    constexpr int rowId = 1;
    int beforeValue = getValue(rowId);
    std::cout << "beforeValue: " << beforeValue << std::endl;

    std::vector<std::thread> threads;

    constexpr int threadCount = 10;
    for (int i = 0; i < threadCount; i++) {
        threads.emplace_back(incrementTenTimes, rowId, useTransaction);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    int afterValue = getValue(rowId);
    std::cout << "afterValue: " << afterValue << std::endl;

    std::cout << "total incremented: " << afterValue - beforeValue;
}

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
