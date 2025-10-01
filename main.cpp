#include <iostream>
#include <string>
#include <thread>
#include <future>
#include <chrono>
#include <array>
#include <memory>

#include "ODBCConnection.h"
#include "OdbcTemplate.h"
#include "TransactionStatus.h"

enum class IsolationLevel {
    ReadUncommitted = 1,
    ReadCommitted = 2,
    RepeatableRead = 3,
    Serializable = 4
};

static constexpr std::array kIsolationSql = {
    "READ UNCOMMITTED",
    "READ COMMITTED",
    "REPEATABLE READ",
    "SERIALIZABLE"
};

static const char* toSql(const IsolationLevel level) {
    const auto idx = static_cast<size_t>(static_cast<int>(level) - 1);
    return kIsolationSql[idx];
}

bool isSerializable(const IsolationLevel level) {
    return level == IsolationLevel::Serializable;
}

static constexpr std::array kChoiceToIsolation = {
    IsolationLevel::ReadUncommitted,
    IsolationLevel::ReadCommitted,
    IsolationLevel::RepeatableRead,
    IsolationLevel::Serializable
};

void ensureTransactionTestTable() {
    OdbcConnection connection("LOCAL", "root", "");
    OdbcTemplate odbcTemplate(connection);
    odbcTemplate.execute("CREATE TABLE IF NOT EXISTS transaction_test (\n"
                         "  id INT NOT NULL PRIMARY KEY AUTO_INCREMENT,\n"
                         "  value INT NOT NULL\n"
                         ")");
    odbcTemplate.execute("INSERT INTO transaction_test(id, value) VALUES (1, 100)\n"
                         "ON DUPLICATE KEY UPDATE value = VALUES(value)");
}

void ensurePhantomTestTableCleared() {
    OdbcConnection conn("LOCAL", "root", "");
    OdbcTemplate tpl(conn);
    tpl.execute("CREATE TABLE IF NOT EXISTS phantom_test (\n"
                "  id INT NOT NULL PRIMARY KEY AUTO_INCREMENT\n"
                ")");
    tpl.execute("DELETE FROM phantom_test");
}

void applyIsolation(OdbcTemplate& tpl, const IsolationLevel isolation) {
    std::string sql = std::string("SET SESSION TRANSACTION ISOLATION LEVEL ") + toSql(isolation);
    tpl.execute(sql);
}

struct Session {
    OdbcConnection connection;
    OdbcTemplate odbcTemplate;
    TransactionStatus txStatus;
    Session() : connection("LOCAL", "root", ""), odbcTemplate(connection), txStatus(connection.getTransactionStatus()) {}
};

template <typename Phase1, typename Phase2, typename Other>
static void runWithOptionalThreads(bool useThreads, Phase1&& phase1, Other&& middle, Phase2&& phase2) {
    std::promise<void> phase1DonePromise;
    auto phase1Done = phase1DonePromise.get_future().share();

    if (useThreads) {
        std::thread splitThread([&] {
            phase1(phase1DonePromise);
            phase2();
        });
        std::thread otherThread([&] {
            phase1Done.wait();
            middle();
        });
        splitThread.join();
        otherThread.join();
    } else {
        phase1(phase1DonePromise);
        middle();
        phase2();
    }
}

static void resetTransactionTestValue(int value) {
    OdbcConnection admin("LOCAL", "root", "");
    OdbcTemplate adminTpl(admin);
    adminTpl.execute("UPDATE transaction_test SET value = ? WHERE id = 1", value);
}

static int getValue(OdbcTemplate& tpl, int id) {
    return tpl.queryForValue<int>("SELECT value FROM transaction_test WHERE id = ?", id);
}

static void runDirtyReadTest(const IsolationLevel isolation) {
    resetTransactionTestValue(100);

    int aBefore = 0;
    int bSeenAfterAUpdate = 0;
    std::shared_ptr<Session> aSession;

    auto aPhase1 = [&](std::promise<void>& ready) {
        aSession = std::make_shared<Session>();
        applyIsolation(aSession->odbcTemplate, isolation);
        aSession->txStatus.setAutoCommit(false);
        aBefore = getValue(aSession->odbcTemplate, 1);
        aSession->odbcTemplate.execute("UPDATE transaction_test SET value = ? WHERE id = 1", aBefore + 1);
        ready.set_value();
    };
    auto aPhase2 = [&] {
        aSession->txStatus.rollback();
    };
    auto bAll = [&] {
        Session b;
        applyIsolation(b.odbcTemplate, isolation);
        b.txStatus.setAutoCommit(false);
        bSeenAfterAUpdate = b.odbcTemplate.queryForValue<int>("SELECT value FROM transaction_test WHERE id = 1");
        b.txStatus.rollback();
    };

    runWithOptionalThreads(isSerializable(isolation), aPhase1, bAll, aPhase2);

    const bool dirtyOccurred = (bSeenAfterAUpdate == aBefore + 1);

    std::cout << "Dirty read 테스트 결과:\n";
    std::cout << " A가 처음 읽은 값: " << aBefore
              << ", A가 커밋 없이 update한 값: " << (aBefore + 1)
              << ", B가 본 값: " << bSeenAfterAUpdate << "\n";
    std::cout << " Dirty read 발생 여부: " << (dirtyOccurred ? "예" : "아니오") << "\n\n";
}

static void runNonRepeatableReadTest(const IsolationLevel isolation) {
    resetTransactionTestValue(200);

    int bRead1 = 0;
    int bRead2 = 0;
    std::shared_ptr<Session> bSession;

    auto bPhase1 = [&](std::promise<void>& ready) {
        bSession = std::make_shared<Session>();
        applyIsolation(bSession->odbcTemplate, isolation);
        bSession->txStatus.setAutoCommit(false);
        bRead1 = getValue(bSession->odbcTemplate, 1);
        ready.set_value();
    };
    auto bPhase2 = [&] {
        bRead2 = getValue(bSession->odbcTemplate, 1);
        bSession->txStatus.rollback();
    };
    auto aAll = [&] {
        Session a;
        applyIsolation(a.odbcTemplate, isolation);
        a.txStatus.setAutoCommit(false);
        a.odbcTemplate.execute("UPDATE transaction_test SET value = ? WHERE id = 1", bRead1 + 1);
        a.txStatus.commit();
    };

    runWithOptionalThreads(isSerializable(isolation), bPhase1, aAll, bPhase2);

    const bool nonRepeatableOccurred = (bRead1 != bRead2);
    std::cout << "Non-repeatable read 테스트 결과:\n";
    std::cout << " B의 첫 번째 읽기: " << bRead1
              << ", A가 커밋한 값: " << (bRead1 + 1)
              << ", B의 두 번째 읽기: " << bRead2 << "\n";
    std::cout << " Non-repeatable read 발생 여부: " << (nonRepeatableOccurred ? "예" : "아니오") << "\n\n";
}

static void runPhantomReadTest(const IsolationLevel isolation) {
    ensurePhantomTestTableCleared();

    long long aCount1 = 0;
    long long aCount2 = 0;
    std::shared_ptr<Session> aSession;

    auto aPhase1 = [&](std::promise<void>& ready) {
        aSession = std::make_shared<Session>();
        applyIsolation(aSession->odbcTemplate, isolation);
        aSession->txStatus.setAutoCommit(false);
        aCount1 = aSession->odbcTemplate.queryForValue<long long>("SELECT COUNT(*) FROM phantom_test");
        ready.set_value();
    };
    auto aPhase2 = [&] {
        aCount2 = aSession->odbcTemplate.queryForValue<long long>("SELECT COUNT(*) FROM phantom_test");
        aSession->txStatus.rollback();
    };
    auto bAll = [&] {
        Session b;
        applyIsolation(b.odbcTemplate, isolation);
        b.txStatus.setAutoCommit(false);
        b.odbcTemplate.execute("INSERT INTO phantom_test(id) VALUES (NULL)");
        b.txStatus.commit();
    };

    runWithOptionalThreads(isSerializable(isolation), aPhase1, bAll, aPhase2);

    const bool phantomOccurred = (aCount1 != aCount2);
    std::cout << "Phantom read 테스트 결과:\n";
    std::cout << " A의 첫 번째 행 개수: " << aCount1
              << ", B가 insert 후 커밋, A의 두 번째 개수: " << aCount2 << "\n";
    std::cout << " Phantom read 발생 여부: " << (phantomOccurred ? "예" : "아니오") << "\n";
}

int main() {
    int choice;
    std::cout << "1) READ UNCOMMITTED\n2) READ COMMITTED\n3) REPEATABLE READ\n4) SERIALIZABLE\n> ";
    std::cin >> choice;

    const IsolationLevel isolation = kChoiceToIsolation[static_cast<size_t>(choice - 1)];

    ensureTransactionTestTable();
    ensurePhantomTestTableCleared();

    runDirtyReadTest(isolation);
    runNonRepeatableReadTest(isolation);
    runPhantomReadTest(isolation);

    return 0;
}