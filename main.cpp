#include "OdbcTemplate.h"
#include <thread>

int getValue(OdbcTemplate& odbcTemplate, int id, bool usePessimistic) {
    if (usePessimistic) {
        return odbcTemplate.queryForValue<int>("select value from transaction_test where id = ? for update", id);
    }
    return odbcTemplate.queryForValue<int>("select value from transaction_test where id = ?", id);
}

void incrementTenTimes(int id, bool usePessimistic) {
    OdbcConnection connection("LOCAL", "root", "");
    auto txStatus = connection.getTransactionStatus();
    txStatus.setAutoCommit(false);

    OdbcTemplate odbcTemplate(connection);

    for (int i = 0; i < 10; i++) {
        int currentValue = getValue(odbcTemplate, id, usePessimistic);
        if (usePessimistic) {
            odbcTemplate.execute("update transaction_test set value = ? where id = ?", currentValue + 1, id);
        } else {
            int tried = 0;
            while (true) {
                tried++;
                currentValue = getValue(odbcTemplate, id, usePessimistic);
                bool updated = odbcTemplate.update("update transaction_test set value = ? where id = ? and value = ?", currentValue + 1, id, currentValue) > 0;
                if (updated) break;
                txStatus.rollback();
            }
            std::cout << tried << "번의 시도 끝에 optimistic lock으로 update 성공" << std::endl;
        }
        txStatus.commit();
    }
}

int getValue(int id) {
    OdbcConnection connection("LOCAL", "root", "");
    OdbcTemplate odbcTemplate(connection);
    return odbcTemplate.queryForValue<int>("select value from transaction_test where id = ?", id);
}

// create table lock_test(id int primary key auto_increment, value int not null);
int main() {
    std::cout << "optimistic(0) or pessimistic(1): ";
    bool usePessimistic;
    std::cin >> usePessimistic;

    constexpr int rowId = 1;
    int beforeValue = getValue(rowId);
    std::cout << "beforeValue: " << beforeValue << std::endl;

    std::vector<std::thread> threads;

    constexpr int threadCount = 10;
    for (int i = 0; i < threadCount; i++) {
        threads.emplace_back(incrementTenTimes, rowId, usePessimistic);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    int afterValue = getValue(rowId);
    std::cout << "afterValue: " << afterValue << std::endl;

    std::cout << "total incremented: " << afterValue - beforeValue;

    return 0;
}

