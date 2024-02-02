//
// Created by Kasper de Bruin on 29/01/2024.
//
#pragma once

#include <Hummingbird_SQL/Config.h>
#include <Hummingbird_SQL/Connection.h>
#include <iostream>
#include <unordered_map>
#include <variant>
#include <vector>

namespace HummingBird::Sql {
  using ColumnValue = std::variant<uint64_t, int64_t, int,
                                   float, double,
                                   bool,
                                   std::string,
                                   std::nullopt_t>;// Add more types as needed

  enum ColumnType {
    HEADER,
    UINT64,
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
    /*
     * @brief Constructor for ColumnInfo
     * @param name Name of the column
     * @param value ColumnValue of the column
     * @param type ColumnType of the column
     */
    ColumnInfo(const std::string &name,
               const ColumnType columnType)
        : ColumnInfo(name, columnType, std::nullopt, ColumnType::NULL_TYPE) {}

    ColumnInfo(const std::string &name,
               const ColumnType columnType,
               const ColumnValue &value,
               const ColumnType &currentType) : name(name), value(value), columnType(columnType), currentType(currentType) {
    }

public:
    const std::string &getName() const {
      return name;
    }

    const ColumnValue &getValue() const {
      return value;
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

  class Row {
public:
    Row() = default;
    ~Row() = default;

    const ColumnInfo &getColumn(const std::string &columnName) const {
    }

    const std::string &getColumnValueAsString(const std::string &columnName) const {
    }

    const ColumnValue &getColumnValue(const std::string &columnName) const {
    }

    const void setColumn(const std::string &columnName, const ColumnInfo &column) {
      m_columns[columnName] = std::make_unique<ColumnInfo>(column);
    }

private:
    std::unordered_map<std::string, std::unique_ptr<ColumnInfo>> m_columns = {};
  };

  struct TableInfo {
private:
    std::string name;
    std::string schemaName;
    std::vector<std::unique_ptr<ColumnInfo>> columns;
    std::vector<std::unique_ptr<Row>> rows;

public:
    TableInfo(const std::string &name, const std::string &schemaName)
        : name(name), schemaName(schemaName) {}

    const std::string &getName() const {
      return name;
    }

    const std::string &getSchemaName() const {
      return schemaName;
    }

    const std::vector<std::unique_ptr<ColumnInfo>> &getColumns() const {
      return columns;
    }

    const std::vector<std::unique_ptr<Row>> &getRows() const {
      return rows;
    }

    void fetchColumns() {
    }

    void fetchRows() {
    }
  };

  struct SchemaInfo {
private:
    std::string name;
    std::unordered_map<std::string, std::unique_ptr<TableInfo>> tables = {};

public:
    SchemaInfo(const std::string &name, std::unordered_map<std::string, std::unique_ptr<TableInfo>> tables)
        : name(name), tables(std::move(tables)) {}

    SchemaInfo(SchemaInfo *schemaInfo) : name(schemaInfo->getName()) {
      for (const auto &table: schemaInfo->getTables()) {
        tables[table.first] = std::make_unique<TableInfo>(table.second->getName(), table.second->getSchemaName());
      }
    }

    const std::string &getName() const {
      return name;
    }

    const std::unordered_map<std::string, std::unique_ptr<TableInfo>> &getTables() const {
      return tables;
    }

    std::unique_ptr<SchemaInfo> getCopy() {
      return std::make_unique<SchemaInfo>(this);
    }
  };
}// namespace HummingBird::Sql