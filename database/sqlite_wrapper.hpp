#pragma once

#include "../main.hpp"
#include "../sqlite/sqlite3.h"

class SQLiteWrapper : public Singleton<SQLiteWrapper>
{
public:
    SQLiteWrapper() = default;
    ~SQLiteWrapper()
    {
        if (db)
        {
            sqlite3_close(db);
            db = nullptr;
        }
    }

    std::vector<std::pair<std::string, std::string>> getTableInfo(const std::string &dbName, const std::string &tableName)
    {
        openDatabase(dbName);
        std::vector<std::pair<std::string, std::string>> tableInfo;
        std::string sql = "PRAGMA table_info(" + tableName + ")";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        }
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::string columnName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            std::string columnType = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            tableInfo.emplace_back(columnName, columnType);
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        db = nullptr;
        return tableInfo;
    }

    void execute(const std::string &dbName, const std::string &sql)
    {
        openDatabase(dbName);
        char *errMsg = nullptr;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
        {
            std::string error = errMsg;
            sqlite3_free(errMsg);
            sqlite3_close(db);
            db = nullptr;
            throw std::runtime_error("SQL error: " + error);
        }
        sqlite3_close(db);
        db = nullptr;
    }

    void add(const std::string &dbName, const std::vector<std::string> &params)
    {
        openDatabase(dbName);
        std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> tableFields;

        for (const auto &param : params)
        {
            auto [table, field, type, value] = parseParam(param);
            tableFields[table].emplace_back(field, type);
        }

        for (const auto &[table, fields] : tableFields)
        {
            createTableIfNotExists(table, fields);
        }

        for (const auto &param : params)
        {
            auto [table, field, type, value] = parseParam(param);
            insertData(table, field, value);
        }
        sqlite3_close(db);
        db = nullptr;
    }

    std::unordered_map<std::string, std::vector<std::unordered_map<std::string, std::string>>>
    retrieve(const std::string &dbName, const std::string &query)
    {
        openDatabase(dbName);
        std::unordered_map<std::string, std::vector<std::unordered_map<std::string, std::string>>> result;
        try
        {
            auto tables = splitString(query, "|");
            for (const auto &table : tables)
            {
                auto [tableName, fieldsWithTypes] = parseTableQuery(table);
                createTableIfNotExists(tableName, fieldsWithTypes);
                result[tableName] = retrieveData(tableName, fieldsWithTypes);
            }
        }
        catch (...)
        {
            sqlite3_close(db);
            db = nullptr;
            throw;
        }
        sqlite3_close(db);
        db = nullptr;
        return result;
    }

    void update(const std::string &dbName, const std::string &query)
    {
        openDatabase(dbName);
        auto updates = splitString(query, "|");
        std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> tableFields;

        for (const auto &update : updates)
        {
            auto [table, field, value] = parseUpdateQuery(update);
            tableFields[table].emplace_back(field, "TEXT");
        }

        for (const auto &[table, fields] : tableFields)
        {
            createTableIfNotExists(table, fields);
        }

        for (const auto &update : updates)
        {
            auto [table, field, value] = parseUpdateQuery(update);
            updateData(table, field, value);
        }
        sqlite3_close(db);
        db = nullptr;
    }

    void remove(const std::string &dbName, const std::string &query)
    {
        openDatabase(dbName);
        auto deletions = splitString(query, "|");
        for (const auto &deletion : deletions)
        {
            if (deletion.substr(0, 13) == "delete_table:")
            {
                auto table = deletion.substr(13);
                deleteTable(table);
            }
            else if (deletion.substr(0, 13) == "delete_field:")
            {
                auto [table, field] = parseDeleteFieldQuery(deletion.substr(13));
                deleteField(table, field);
            }
        }
        sqlite3_close(db);
        db = nullptr;
    }

private:
    sqlite3 *db = nullptr;

    void openDatabase(const std::string &dbName)
    {
        if (sqlite3_open(dbName.c_str(), &db) != SQLITE_OK)
        {
            throw std::runtime_error("Can't open database: " + std::string(sqlite3_errmsg(db)));
        }
    }

    std::tuple<std::string, std::string, std::string, std::string> parseParam(const std::string &param)
    {
        auto parts = splitString(param, ":");
        if (parts.size() != 4)
        {
            throw std::invalid_argument("Invalid parameter format");
        }
        return {parts[0], parts[1], parts[2], parts[3]};
    }

    void createTableIfNotExists(const std::string &table, const std::vector<std::pair<std::string, std::string>> &fieldsWithTypes)
    {
        std::string sql = "CREATE TABLE IF NOT EXISTS " + table + " (id INTEGER PRIMARY KEY AUTOINCREMENT";
        for (const auto &[field, type] : fieldsWithTypes)
        {
            sql += ", " + field + " " + getSqliteType(type);
        }
        sql += ")";
        char *errMsg = nullptr;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
        {
            std::string error = errMsg;
            sqlite3_free(errMsg);
            throw std::runtime_error("SQL error: " + error);
        }
    }

    void addField(const std::string &table, const std::string &field, const std::string &type)
    {
        std::string sql = "ALTER TABLE " + table + " ADD COLUMN " + field + " " + getSqliteType(type);
        char *errMsg = nullptr;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
        {
            if (std::string(errMsg).find("duplicate column name") == std::string::npos)
            {
                std::string error = errMsg;
                sqlite3_free(errMsg);
                throw std::runtime_error("SQL error: " + error);
            }
        }
        sqlite3_free(errMsg);
    }

    void insertData(const std::string &table, const std::string &field, const std::string &value)
    {
        std::string sql = "INSERT INTO " + table + " (" + field + ") VALUES (?)";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        }
        sqlite3_bind_text(stmt, 1, value.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(db)));
        }
        sqlite3_finalize(stmt);
    }

    std::tuple<std::string, std::vector<std::pair<std::string, std::string>>> parseTableQuery(const std::string &query)
    {
        auto parts = splitString(query, ":");
        if (parts.size() < 2)
        {
            throw std::invalid_argument("Invalid table query format");
        }
        std::string tableName = parts[0];
        std::vector<std::pair<std::string, std::string>> fieldsWithTypes;

        std::string fieldsString = join(std::vector<std::string>(parts.begin() + 1, parts.end()), ":");
        auto fieldParts = splitString(fieldsString, ",");

        for (const auto &fieldPart : fieldParts)
        {
            auto fieldAndType = splitString(fieldPart, "::");
            if (fieldAndType.size() != 2)
            {
                throw std::invalid_argument("Invalid field format in table query");
            }
            fieldsWithTypes.emplace_back(fieldAndType[0], fieldAndType[1]);
        }
        return {tableName, fieldsWithTypes};
    }

    std::vector<std::unordered_map<std::string, std::string>> retrieveData(
            const std::string &table,
            const std::vector<std::pair<std::string, std::string>> &fieldsWithTypes)
    {
        std::vector<std::string> fields;
        for (const auto &[field, _] : fieldsWithTypes)
        {
            fields.push_back(field);
        }

        std::string sql = "SELECT " + join(fields, ", ") + " FROM " + table;
        sqlite3_stmt *stmt = nullptr;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        }

        std::vector<std::unordered_map<std::string, std::string>> result;

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::unordered_map<std::string, std::string> row;
            for (int i = 0; i < static_cast<int>(fields.size()); ++i)
            {
                const char *value = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
                row[fields[i]] = value ? value : "";
            }
            result.push_back(row);
        }

        sqlite3_finalize(stmt);
        return result;
    }


    std::tuple<std::string, std::string, std::string> parseUpdateQuery(const std::string &query)
    {
        auto parts = splitString(query, ":");
        if (parts.size() != 3)
        {
            throw std::invalid_argument("Invalid update query format");
        }
        return {parts[0], parts[1], parts[2]};
    }

    void updateData(const std::string &table, const std::string &field, const std::string &value)
    {
        std::string sql = "UPDATE " + table + " SET " + field + " = ?";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        }
        sqlite3_bind_text(stmt, 1, value.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(db)));
        }
        sqlite3_finalize(stmt);
    }

    void deleteTable(const std::string &table)
    {
        std::string sql = "DROP TABLE IF EXISTS " + table;
        char *errMsg = nullptr;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
        {
            std::string error = errMsg;
            sqlite3_free(errMsg);
            throw std::runtime_error("SQL error: " + error);
        }
    }

    std::tuple<std::string, std::string> parseDeleteFieldQuery(const std::string &query)
    {
        auto parts = splitString(query, ":");
        if (parts.size() != 2)
        {
            throw std::invalid_argument("Invalid delete field query format");
        }
        return {parts[0], parts[1]};
    }

    void deleteField(const std::string &table, const std::string &field)
    {
        std::string sql = "ALTER TABLE " + table + " RENAME TO temp_" + table;
        char *errMsg = nullptr;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
        {
            std::string error = errMsg;
            sqlite3_free(errMsg);
            throw std::runtime_error("SQL error: " + error);
        }

        sql = "PRAGMA table_info(" + table + ")";
        sqlite3_stmt *stmt;
        std::vector<std::string> columns;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                std::string columnName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
                if (columnName != field)
                {
                    columns.push_back(columnName);
                }
            }
        }
        sqlite3_finalize(stmt);

        sql = "CREATE TABLE " + table + " AS SELECT " + join(columns, ", ") + " FROM temp_" + table;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
        {
            std::string error = errMsg;
            sqlite3_free(errMsg);
            throw std::runtime_error("SQL error: " + error);
        }

        sql = "DROP TABLE temp_" + table;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
        {
            std::string error = errMsg;
            sqlite3_free(errMsg);
            throw std::runtime_error("SQL error: " + error);
        }
    }

    static std::vector<std::string> splitString(const std::string &s, const std::string &delimiter)
    {
        std::vector<std::string> tokens;
        size_t start = 0, end;

        while ((end = s.find(delimiter, start)) != std::string::npos)
        {
            tokens.push_back(s.substr(start, end - start));
            start = end + delimiter.length();
        }

        tokens.push_back(s.substr(start));
        return tokens;
    }

    static std::string join(const std::vector<std::string> &v, const std::string &delimiter)
    {
        std::string result;
        for (size_t i = 0; i < v.size(); ++i)
        {
            if (i > 0)
            {
                result += delimiter;
            }
            result += v[i];
        }
        return result;
    }

    static std::string getSqliteType(const std::string &cppType)
    {
        if (cppType == "int" || cppType == "int64" || cppType == "long_long")
        {
            return "INTEGER";
        }
        else if (cppType == "double")
        {
            return "REAL";
        }
        else if (cppType == "string")
        {
            return "TEXT";
        }
        else if (cppType == "bool")
        {
            return "INTEGER";
        }
        else
        {
            return "TEXT";
        }
    }
};