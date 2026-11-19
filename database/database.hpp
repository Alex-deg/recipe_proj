#pragma once

#include <string>
#include <vector>
#include <variant>
#include <algorithm>
#include <stdexcept>
#include <sqlite3.h>

using SQLValue = std::variant<std::string, int, double, long long>;

class DataBaseException : public std::runtime_error {
public:
    DataBaseException(const std::string &msg) : std::runtime_error(msg) {}
};

class QueryResult {
public:
    
    QueryResult(std::vector<std::vector<SQLValue>> _rows, std::vector<std::string> _columns);

    bool empty() const;
    size_t size() const;
    
    template<typename T>
    T get(size_t row, size_t col) const {
        return std::get<T>(rows[row][col]);
    }

    template<typename T>
    T get(size_t row, const std::string& columnName) const {
        auto it = std::find(columnNames.begin(), columnNames.end(), columnName);
        if (it == columnNames.end()) {
            throw std::runtime_error("Column not found: " + columnName);
        }
        size_t col = std::distance(columnNames.begin(), it);
        return get<T>(row, col);
    }
private:
    std::vector<std::vector<SQLValue>> rows = {};
    std::vector<std::string> columnNames = {};
};

class DataBase{
public:
    DataBase() = default;
    explicit DataBase(const std::string &path_to_database);
    ~DataBase();
    void open(const std::string &path_to_database);
    bool isOpen() const;
    void close();
    bool isTableExists(const std::string& table_name);
    void vacuum();
protected:
    void executeRequest(const std::string &sql);
    void executeRequest(const std::string& sql, 
                        const std::vector<SQLValue>& params);
    QueryResult executeQuery(const std::string& sql,
                             const std::vector<SQLValue>& params = {});
private:
    sqlite3 *db = nullptr;
};