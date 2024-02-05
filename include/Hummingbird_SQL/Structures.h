//
// Created by Kasper de Bruin on 29/01/2024.
//
#pragma once

#include <Hummingbird_SQL/Config.h>
#include <vector>

namespace HummingBird::Sql {
  using ColumnValue = std::variant<uint64_t, int64_t, int,
                                   float, double,
                                   bool,
                                   std::string,
                                   std::nullopt_t>;// Add more types as needed

  enum ColumnType {
    INT64,
    INT,
    FLOAT,
    DOUBLE,
    BOOL,
    STRING,
    NULL_TYPE
  };

  class ColumnInfo {
public:
    ColumnInfo(const std::string &name, const ColumnType columnType)
        : ColumnInfo(name, columnType, std::nullopt, ColumnType::NULL_TYPE) {}

    ColumnInfo(const std::string &name, const ColumnType columnType, const ColumnValue &value, const ColumnType &currentType)
        : name(name), value(value), columnType(columnType), currentType(currentType) {
    }

public:
    const std::string &getName() const {
      return name;
    }

    const ColumnValue &getValue() const {
      return value;
    }

    std::string getValueAsString() const {
      std::string result = "NULL";
      try {
        switch (currentType) {
          case ColumnType::INT64:
            result = std::to_string(std::get<int64_t>(value));
            break;
          case ColumnType::INT:
            result = std::to_string(std::get<int>(value));
            break;
          case ColumnType::FLOAT:
            result = std::to_string(std::get<float>(value));
            break;
          case ColumnType::DOUBLE:
            result = std::to_string(std::get<double>(value));
            break;
          case ColumnType::BOOL:
            result = std::to_string(std::get<bool>(value));
            break;
          case ColumnType::STRING:
            result = std::get<std::string>(value);
            break;
          case ColumnType::NULL_TYPE:
            result = "NULL";
            break;
        }
      } catch (std::bad_variant_access &e) {
        result = "BADD VARIANT ACCES";
      }
      return result;
    }

    const ColumnType &getCurrentType() const {
      return currentType;
    }

    const ColumnType &getType() const {
      return columnType;
    }

private:
    std::string name = "";
    ColumnValue value = std::nullopt;
    ColumnType columnType = NULL_TYPE;
    ColumnType currentType = NULL_TYPE;
  };

  struct Row {
public:
    Row(const std::vector<ColumnInfo> &columns) {
      for (const auto &column: columns) {
        this->columns.push_back(std::make_shared<ColumnInfo>(column));
      }
    }

    ColumnInfo &getColumn(const int index) const {
      if (this == nullptr) {
        ColumnInfo *columnInfo = new ColumnInfo("EMPTY", NULL_TYPE);
        return *columnInfo;
      }
      return *columns[index];
    }

    int getColumnCount() const {
      if (this == nullptr) {
        return 0;
      }
      return columns.size();
    }

private:
    std::vector<std::shared_ptr<ColumnInfo>> columns;
  };

  struct TableInfo {
private:
    std::string name;
    std::string schemaName;
    std::vector<std::shared_ptr<Row>> rows;
    int columnCount = 0;

public:
    TableInfo(const std::string &name, const std::string &schemaName, const std::vector<Row> &rows, const int columnCount = -1)
        : name(name), schemaName(schemaName), columnCount(columnCount) {
      for (const auto &row: rows) {
        this->rows.push_back(std::make_shared<Row>(row));
      }
    }

    const std::string &getName() const {
      return name;
    }

    const std::string &getSchemaName() const {
      return schemaName;
    }

    const int getColumnCount() const {
      return columnCount;
    }

    std::vector<Row> getRows() const {
      if (this == nullptr) {
        HUMMINGBIRD_SQL_ERROR_FUNCTION("TableInfo is nullptr");
        return {};
      }
      std::vector<Row> _rows;
      for (const auto &row: rows) {
        _rows.push_back(*row);
      }
      return _rows;
    }

    void fetchRows(const Connection &connection);
  };

  struct SchemaInfo {
private:
    std::string name;
    //std::unordered_map<std::string, std::unique_ptr<TableInfo>> tables = {};
    std::unordered_map<std::string, std::shared_ptr<TableInfo>> tables = {};
    TableInfo *m_currentTable = nullptr;

public:
    SchemaInfo() = default;
    SchemaInfo(const std::string &name, std::vector<TableInfo> tables)
        : name(name) {
      for (const auto &table: tables) {
        this->tables[table.getName()] = std::make_shared<TableInfo>(table);
      }
    }

    SchemaInfo(const SchemaInfo *schemaInfo) {
      name = schemaInfo->name;
      for (const auto &table: schemaInfo->tables) {
        tables[table.first] = table.second;
      }
    }

    const std::string &getName() const {
      return name;
    }

    std::vector<std::string> getTableNames() const {
      if (this == nullptr) {
        return {};
      }

      if (tables.size() <= 0) {
        return {};
      }

      if (tables.empty()) {
        return {};
      }
      std::vector<std::string> tableNames;

      for (const auto &pair: tables) {
        tableNames.push_back(pair.first);
      }
      return tableNames;
    }

    void setTable(const std::string &tableName, const Connection &connection) {
      if (this == nullptr) {
        return;
      }
      if(this->m_currentTable != nullptr && this->m_currentTable->getName() == tableName){
        return;
      }
      if (tableName.empty()) {
        HUMMINGBIRD_SQL_ERROR_FUNCTION("Table name is empty");
        return;
      }

      auto table = tables.find(tableName);
      if (table != tables.end()) {
        m_currentTable = table->second.get();
        m_currentTable->fetchRows(connection);
      } else {
        HUMMINGBIRD_SQL_ERROR_FUNCTION("Table not found: " + tableName);
      }
    }

    const bool isTableSet() const {
      return m_currentTable != nullptr;
    }

    TableInfo &getCurrentTable() const {
      if (!this || !isTableSet()) {
        TableInfo *tableInfo = new TableInfo("EMPTY", "EMPTY", {});
        return *tableInfo;
      }
      return *m_currentTable;
    }
  };
}// namespace HummingBird::Sql