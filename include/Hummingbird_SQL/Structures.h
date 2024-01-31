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
    ColumnInfo(const std::string &name = "", ColumnType type = ColumnType::NULL_TYPE, const ColumnValue &value = std::nullopt) : name(name), value(value), type(type) {
    }

public:
    const std::string &getName() const {
      return name;
    }

    const ColumnValue &getValue() const {
      return value;
    }

    ColumnType getType() const {
      return type;
    }

    const std::string getTypeAsString() const {
      switch (type) {
        case HEADER:
          return "HEADER";
        case UINT64:
          return "UINT64";
        case INT64:
          return "INT64";
        case INT:
          return "INT";
        case FLOAT:
          return "FLOAT";
        case DOUBLE:
          return "DOUBLE";
        case BOOL:
          return "BOOL";
        case STRING:
          return "STRING";
        case NULL_TYPE:
          return "NULL";
      }
      return "NULL";
    }

    const std::string getValueAsString() const {
      switch (type) {
        case HEADER:
          return "HEADER";
        case UINT64:
          return std::to_string(std::get<uint64_t>(value));
        case INT64:
          return std::to_string(std::get<int64_t>(value));
        case INT:
          return std::to_string(std::get<int>(value));
        case FLOAT:
          return std::to_string(std::get<float>(value));
        case DOUBLE:
          return std::to_string(std::get<double>(value));
        case BOOL:
          return std::to_string(std::get<bool>(value));
        case STRING:
          return std::get<std::string>(value);
        case NULL_TYPE:
          return "NULL";
        default:
          return "Unkown type: " +
                 std::to_string(type) + " for value: " +
                 "UNKOWN TYPE" +
                 " in column: " + name + " with type: " +
                 getTypeAsString();
      }

    }
//    ColumnInfo(){
//      name = "";
//      value = std::nullopt;
//      type = NULL_TYPE;
//    }



private:
    std::string name;
    ColumnValue value;
    ColumnType type;
  };

  class Row {
public:
    Row() = default;
    ~Row() = default;

    void addColumn(const std::string &columnName, const ColumnInfo &columnValue) {
      m_columns[columnName] = columnValue;
    }

    ColumnInfo getColumn(const std::string &columnName) const {
      auto it = m_columns.find(columnName);
      if (it != m_columns.end()) {
        return it->second;
      }
      HUMMINGBIRD_SQL_ERROR_FUNCTION("Column not found: " + columnName);
      return ColumnInfo("", NULL_TYPE, std::nullopt);
    }
    std::string getColumnValueAsString(const std::string &columnName) const {
      auto it = m_columns.find(columnName);
      if (it != m_columns.end()) {
        return it->second.getValueAsString();
      }
      HUMMINGBIRD_SQL_ERROR_FUNCTION("Column not found: " + columnName);
      return "";
    }
    std::unordered_map<std::string, ColumnInfo> getColumns() const {
      return m_columns;
    }
    void setColumns(const std::unordered_map<std::string, ColumnInfo> &columns) {
      m_columns = columns;
    }

private:
    std::unordered_map<std::string, ColumnInfo> m_columns;
  };

  struct TableInfo {
    std::string name;
    std::string schemaName;
    std::unordered_map<std::string, ColumnInfo> columns;
    std::vector<Row> rows;
  };

  struct SchemaInfo {
    std::string name;
    std::unordered_map<std::string, TableInfo> tables = {};

    std::optional<TableInfo> getTable(const std::string &tableName) const {
      auto it = tables.find(tableName);
      if (it != tables.end()) {
        return it->second;
      }
      return std::nullopt;
    }
  };
}// namespace HummingBird::Sql