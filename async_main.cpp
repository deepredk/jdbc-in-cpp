#ifdef IGNORE
#include "OdbcTemplate.h"
#include <iostream>
#include <thread>
#include <future>
#include <vector>
#include <chrono>
#include <string>

static int getValue(OdbcTemplate& odbcTemplate, int id, bool usePessimistic) {
    if (usePessimistic) {
        return odbcTemplate.queryForValue<int>(
            "select value from transaction_test where id = ? for update", id);
    }
    return odbcTemplate.queryForValue<int>(
        "select value from transaction_test where id = ?", id);
}

static void incrementN(int id, int updateCount, bool usePessimistic) {
    OdbcConnection connection("LOCAL", "root", "");
    auto txStatus = connection.getTransactionStatus();
    txStatus.setAutoCommit(false);

    OdbcTemplate odbcTemplate(connection);

    for (int i = 0; i < updateCount; i++) {
        if (usePessimistic) {
            int currentValue = getValue(odbcTemplate, id, true);
            odbcTemplate.execute(
                "update transaction_test set value = ? where id = ?",
                currentValue + 1, id
            );
        } else {
            while (true) {
                int currentValue = getValue(odbcTemplate, id, false);
                int updated = odbcTemplate.update(
                    "update transaction_test set value = ? where id = ? and value = ?",
                    currentValue + 1, id, currentValue
                );
                if (updated > 0) break;
                txStatus.rollback();
            }
        }
        txStatus.commit();
    }
}

static int getCurrentValue(int id) {
    OdbcConnection connection("LOCAL", "root", "");
    OdbcTemplate odbcTemplate(connection);
    return odbcTemplate.queryForValue<int>(
        "select value from transaction_test where id = ?", id);
}

int main() {
    bool usePessimistic = false; // 무조건 낙관락

    bool useAsync;
    std::cout << "sync(0) or async(1): ";
    std::cin >> useAsync;

    int concurrentCount;
    std::cout << "concurrentCount: ";
    std::cin >> concurrentCount;

    int updateCount;
    std::cout << "updateCount: ";
    std::cin >> updateCount;

    constexpr int rowId = 1;

    int beforeValue = getCurrentValue(rowId);
    std::cout << "beforeValue: " << beforeValue << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    if (useAsync) {
        std::vector<std::future<void>> futures;
        futures.reserve(concurrentCount);
        for (int i = 0; i < concurrentCount; ++i) {
            futures.emplace_back(std::async(std::launch::async, incrementN, rowId, updateCount, usePessimistic));
        }
        for (auto& f : futures) f.get();
    } else {
        std::vector<std::thread> threads;
        threads.reserve(concurrentCount);
        for (int i = 0; i < concurrentCount; ++i) {
            threads.emplace_back(incrementN, rowId, updateCount, usePessimistic);
        }
        for (auto& t : threads) t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "elapsedMs: " << elapsedMs << std::endl;

    int afterValue = getCurrentValue(rowId);
    std::cout << "afterValue: " << afterValue << std::endl;
    std::cout << "total incremented: " << (afterValue - beforeValue) << std::endl;

    return 0;
}
#endif