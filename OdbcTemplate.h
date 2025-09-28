#ifndef JDBC_IN_CPP_ODBCTEMPLATE_H
#define JDBC_IN_CPP_ODBCTEMPLATE_H

#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <stdexcept>

#include "ODBCConnection.h"
#include "ResultSet.h"

class OdbcTemplate {
public:
    explicit OdbcTemplate(OdbcConnection& connection);

    template <typename... Args>
    int update(const std::string& sql, Args&&... args) {
        auto pstmt = connection.prepareStatement(sql);
        bindAll(pstmt, std::forward<Args>(args)...);
        return pstmt.executeUpdate();
    }

    template <typename T, typename... Args>
    T queryForValue(const std::string& sql, Args&&... args) {
        auto pstmt = connection.prepareStatement(sql);
        bindAll(pstmt, std::forward<Args>(args)...);
        auto rs = pstmt.executeQuery();
        if (!rs.next()) {
            throw std::runtime_error("No rows returned.");
        }
        return getValue<T>(rs, 1);
    }

    template <typename T, typename RowMapper, typename... Args>
    T queryForObject(const std::string& sql, RowMapper&& rowMapper, Args&&... args) {
        auto pstmt = connection.prepareStatement(sql);
        bindAll(pstmt, std::forward<Args>(args)...);
        auto rs = pstmt.executeQuery();
        if (!rs.next()) {
            throw std::runtime_error("No rows returned.");
        }
        return std::forward<RowMapper>(rowMapper)(rs);
    }

    template <typename T, typename RowMapper, typename... Args>
    std::vector<T> query(const std::string& sql, RowMapper&& rowMapper, Args&&... args) {
        auto pstmt = connection.prepareStatement(sql);
        bindAll(pstmt, std::forward<Args>(args)...);
        auto rs = pstmt.executeQuery();
        std::vector<T> results;
        while (rs.next()) {
            results.emplace_back(std::forward<RowMapper>(rowMapper)(rs));
        }
        return results;
    }

    template <typename... Args>
    void execute(const std::string& sql, Args&&... args) {
        auto pstmt = connection.prepareStatement(sql);
        bindAll(pstmt, std::forward<Args>(args)...);
        pstmt.execute();
    }

private:
    OdbcConnection& connection;

    static void setParameter(PreparedStatement& pstmt, const int index, const std::string& value) { pstmt.setString(index, value); }
    static void setParameter(PreparedStatement& pstmt, const int index, const char* value) { pstmt.setString(index, std::string(value)); }
    static void setParameter(PreparedStatement& pstmt, const int index, const int value) { pstmt.setInt(index, value); }
    static void setParameter(PreparedStatement& pstmt, const int index, const long value) { pstmt.setLong(index, value); }
    static void setParameter(PreparedStatement& pstmt, const int index, const long long value) { pstmt.setLong(index, value); }
    static void setParameter(PreparedStatement& pstmt, const int index, const double value) { pstmt.setDouble(index, value); }

    template <typename... Args>
    static void bindAll(PreparedStatement& pstmt, Args&&... args) {
        int idx = 1;
        ((setParameter(pstmt, idx++, std::forward<Args>(args)), void()), ...);
    }

    template <typename T>
    static T getValue(ResultSet& rs, const int index) {
        if constexpr (std::is_same_v<T, std::string>) {
            return rs.getString(index);
        } else if constexpr (std::is_same_v<T, int>) {
            return rs.getInt(index);
        } else if constexpr (std::is_same_v<T, long long>) {
            return static_cast<long long>(rs.getLong(index));
        } else if constexpr (std::is_same_v<T, double>) {
            return rs.getDouble(index);
        } else {
            static_assert(sizeof(T) == 0, "Unsupported return type for queryForValue.");
        }
    }
};

#endif //JDBC_IN_CPP_ODBCTEMPLATE_H