#ifdef IGNORE

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
int transaction_test() {
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

#endif
