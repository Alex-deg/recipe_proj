#include "database.hpp"

#include <iostream>
#include <memory>

QueryResult::QueryResult(std::vector<std::vector<SQLValue>> _rows, std::vector<std::string> _columns) :
                         rows(std::move(_rows)), columnNames(std::move(_columns)) {}

bool  QueryResult::empty() const { return rows.empty(); }
size_t QueryResult::size() const { return rows.size();  }



DataBase::DataBase(const std::string &path_to_DataBase) {
    open(path_to_DataBase);
}

DataBase::~DataBase() {
    close();
}

struct StmtDeleter {
    void operator()(sqlite3_stmt* s) const noexcept {
        if (s) sqlite3_finalize(s);
    }
};

using StmtPtr = std::unique_ptr<sqlite3_stmt, StmtDeleter>;

StmtPtr prepareStatement(sqlite3* db, const std::string& sql) {
    sqlite3_stmt* raw = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &raw, nullptr) != SQLITE_OK)
        throw DataBaseException("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    return StmtPtr(raw);
}

void bindParams(sqlite3_stmt* stmt, const std::vector<SQLValue>& params) {
    for (size_t i = 0; i < params.size(); ++i) {
        const int idx = static_cast<int>(i + 1);
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if      constexpr (std::is_same_v<T, std::string>) sqlite3_bind_text  (stmt, idx, arg.c_str(), -1, SQLITE_TRANSIENT);
            else if constexpr (std::is_same_v<T, int>)         sqlite3_bind_int   (stmt, idx, arg);
            else if constexpr (std::is_same_v<T, double>)      sqlite3_bind_double(stmt, idx, arg);
            else if constexpr (std::is_same_v<T, long long>)   sqlite3_bind_int64 (stmt, idx, arg);
        }, params[i]);
    }
}

void DataBase::executeRequest(const std::string &sql) {
    if (!db)
        throw DataBaseException("DataBase not opened");
    char* err_msg = nullptr;
    int response = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg);
    if (response != SQLITE_OK){
        std::string err = err_msg ? err_msg : "Unknown error";
        sqlite3_free(err_msg);
        throw DataBaseException("SQL error: " + err + "\nQuery: " + sql);
    }
}

void DataBase::executeRequest(const std::string& sql, const std::vector<SQLValue>& params) {
    
    if (!db) throw DataBaseException("DataBase not opened");

    auto stmt = prepareStatement(db, sql);
    bindParams(stmt.get(), params);

    int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        throw DataBaseException("Execution failed: " + std::string(sqlite3_errmsg(db)) + 
                                "\nQuery: " + sql);
    }
}

QueryResult DataBase::executeQuery(const std::string& sql, const std::vector<SQLValue>& params) {
    
    if (!db) throw DataBaseException("DataBase not opened");

    auto stmt = prepareStatement(db, sql);
    bindParams(stmt.get(), params);

    const int columnCount = sqlite3_column_count(stmt.get());
    std::vector<std::string> columnNames = {};
    columnNames.reserve(columnCount);
    for (int i = 0; i < columnCount; ++i)
        columnNames.emplace_back(sqlite3_column_name(stmt.get(), i));

    std::vector<std::vector<SQLValue>> rows;
    int rc;
    while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
        std::vector<SQLValue> row = {};
        row.reserve(columnCount);
        for (int i = 0; i < columnCount; ++i) {
            switch (sqlite3_column_type(stmt.get(), i)) {
                case SQLITE_INTEGER: row.emplace_back(sqlite3_column_int64 (stmt.get(), i)); break;
                case SQLITE_FLOAT:   row.emplace_back(sqlite3_column_double(stmt.get(), i)); break;
                case SQLITE_TEXT:    row.emplace_back(reinterpret_cast<const char*>(
                                          sqlite3_column_text(stmt.get(), i)));              break;
                default:             row.emplace_back(std::string());                        break;
            }
        }
        rows.push_back(std::move(row));
    }
    if (rc != SQLITE_DONE) {
        throw DataBaseException("Query failed: " +
                                std::string(sqlite3_errmsg(db)));
    }

    return QueryResult(std::move(rows), std::move(columnNames));
}

void DataBase::open(const std::string& path_to_DataBase) {
    if (db)
        close();
    int db_session = sqlite3_open(path_to_DataBase.c_str(), &db);
    if (db_session != SQLITE_OK){
        std::string err = sqlite3_errmsg(db);
        sqlite3_close(db);
        db = nullptr;
        throw DataBaseException("Cannot open DataBase: " + err);
    }
    std::cout << "DataBase has been opened successfully!" << std::endl;
}

void DataBase::close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool DataBase::isTableExists(const std::string &table_name) {
    QueryResult response = executeQuery("SELECT name FROM sqlite_master WHERE name = ?;", {table_name});
    return !response.empty();
}

void DataBase::vacuum() {
    executeRequest("VACUUM");
}

bool DataBase::isOpen() const {
    return db != nullptr;
}