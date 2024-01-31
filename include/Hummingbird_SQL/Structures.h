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
  using ColumnValue = std::variant<uint64_t, int64_t,
                                    float, double,
                                    bool,
                                    std::string,
                                    std::nullopt_t>;// Add more types as needed

  class Row {
public:
    Row() = default;
    ~Row() = default;

    void addColumn(const std::string &columnName, const ColumnValue &columnValue) {
      m_columns[columnName] = columnValue;
    }

    ColumnValue getColumn(const std::string &columnName) const {
      auto it = m_columns.find(columnName);
      if (it != m_columns.end()) {
        return it->second;
      }
      return std::nullopt;
    }

    std::unordered_map<std::string, ColumnValue> getColumns() const {
      return m_columns;
    }

    void setColumns(const std::unordered_map<std::string, ColumnValue> &columns) {
      m_columns = columns;
    }

private:
    std::unordered_map<std::string, ColumnValue> m_columns;
  };

  struct ColumnInfo {
    std::string name;
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