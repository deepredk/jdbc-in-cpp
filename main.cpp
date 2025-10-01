#include "OdbcTemplate.h"
#include <thread>
#include <chrono>

int getValue(OdbcTemplate& odbcTemplate, int id, bool usePessimistic) {
    if (usePessimistic) {
        return odbcTemplate.queryForValue<int>("select value from transaction_test where id = ? for update", id);
    }
    return odbcTemplate.queryForValue<int>("select value from transaction_test where id = ?", id);
}

void incrementTenTimes(int id, int updateCount, bool usePessimistic) {
    OdbcConnection connection("LOCAL", "root", "");
    auto txStatus = connection.getTransactionStatus();
    txStatus.setAutoCommit(false);

    OdbcTemplate odbcTemplate(connection);

    for (int i = 0; i < updateCount; i++) {
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
            // std::cout << tried << "번의 시도 끝에 optimistic00 lock으로 update 성공" << std::endl;
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

    std::cout << "threadCount: ";
    int threadCount;
    std::cin >> threadCount;

    std::cout << "updateCount: ";
    int updateCount;
    std::cin >> updateCount;

    constexpr int rowId = 1;
    int beforeValue = getValue(rowId);
    std::cout << "beforeValue: " << beforeValue << std::endl;

    std::vector<std::thread> threads;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < threadCount; i++) {
        threads.emplace_back(incrementTenTimes, rowId, updateCount, usePessimistic);
    }

    for (auto& thread : threads) {
        thread.join();
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "스레드들 시작으로부터 전부 끝날때까지 elapsedMs: " << elapsedMs.count() << std::endl;

    int afterValue = getValue(rowId);
    std::cout << "afterValue: " << afterValue << std::endl;

    std::cout << "total incremented: " << afterValue - beforeValue;

    return 0;
}

