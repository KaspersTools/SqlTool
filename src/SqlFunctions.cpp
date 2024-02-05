//
// Created by Kasper de Bruin on 31/01/2024.
//
#include <xdevapi.h>
#include "../include/Hummingbird_SQL/Connection.h"
#include "../private/SqlFunctions.h"

namespace HummingBird::Sql {


  namespace Server {
    bool connectionCheck(const Connection &connection) {
      if (!connection.isConnected()) {
        HUMMINGBIRD_SQL_ASSERT(false && "Connection is not established");
        return false;
      }
      return true;
    }

    ColumnType columnTypeToColumnType(const mysqlx::Type &type) {
      switch (type) {
        case mysqlx::Type::TINYINT:
        case mysqlx::Type::SMALLINT:
        case mysqlx::Type::MEDIUMINT:
        case mysqlx::Type::INT:
        case mysqlx::Type::BIGINT:
          return ColumnType::INT64;
        case mysqlx::Type::FLOAT:
          return ColumnType::FLOAT;
        case mysqlx::Type::DOUBLE:
          return ColumnType::DOUBLE;
        case mysqlx::Type::DECIMAL:
          return ColumnType::DOUBLE;
        case mysqlx::Type::STRING:
          return ColumnType::STRING;
        default:
          return ColumnType::NULL_TYPE;
      }
    }

    ColumnType valueTypeToColumnType(const mysqlx::Value &value) {
      switch (value.getType()) {
        case mysqlx::Value::Type::VNULL:
          return ColumnType::NULL_TYPE;
        case mysqlx::Value::Type::INT64:
        case mysqlx::Value::Type::UINT64:
          return ColumnType::INT;
        case mysqlx::Value::Type::FLOAT:
          return ColumnType::FLOAT;
        case mysqlx::Value::Type::DOUBLE:
          return ColumnType::DOUBLE;
        case mysqlx::Value::Type::BOOL:
          return ColumnType::BOOL;
        case mysqlx::Value::Type::STRING:
          return ColumnType::STRING;
        case mysqlx::Value::Type::RAW:
          return ColumnType::STRING;
        case mysqlx::Value::Type::DOCUMENT:
          return ColumnType::STRING;
        default:
          return ColumnType::NULL_TYPE;
      }
    }

    std::vector<SchemaInfo>
    Server::fetchSchemas(const Connection &connection, const bool fetchTables, const bool fetchColumnsAndRows) {
      HUMMINGBIRD_SQL_LOG_FUNCTION("Fetching schemas from server");
      connectionCheck(connection);


      std::vector<SchemaInfo> cachedSchemas = {};
      std::vector<mysqlx::Schema> serverSchemas = {};

      try {
        HUMMINGBIRD_SQL_LOG_FUNCTION("Fetching schemas from server for schema");
        serverSchemas = connection.getSession().getSchemas();

        for (const mysqlx::Schema &serverSchema: serverSchemas) {
          const std::string schemaName = serverSchema.getName();

          if (schemaName == "information_schema" || schemaName == "mysql" || schemaName == "performance_schema" || schemaName == "sys") {
            continue;
          }

          std::vector<TableInfo> tables = {};
          if (fetchTables) {
            tables = Server::fetchTables(connection, schemaName, fetchColumnsAndRows);
          }

          SchemaInfo schemaInfo = SchemaInfo(schemaName, tables);
          cachedSchemas.push_back(schemaInfo);
        }
      } catch (const std::exception &err) {
        HUMMINGBIRD_SQL_ASSERT(false && err.what());
      }

      return cachedSchemas;
    }

    std::vector<TableInfo>
    Server::fetchTables(const Connection &connection, const std::string &schemaName, const bool fetchRows) {
      HUMMINGBIRD_SQL_LOG_FUNCTION("Fetching tables from server for schema " + schemaName);
      connectionCheck(connection);

      std::vector<mysqlx::Table> serverTables = {};
      std::vector<TableInfo> cachedTables = {};

      try {
        serverTables = connection.getSession().getSchema(schemaName).getTables();
        for (mysqlx::Table &serverTable: serverTables) {
          const std::string tableName = serverTable.getName();
          if (tableName == "schema_version") {
            continue;
          }

          if (serverTable.isView()) {
            HUMMINGBIRD_SQL_WARNING_FUNCTION("Skipping view " + tableName + " in schema " + schemaName + " as it is not supported");
            continue;
          }
          std::vector<Row> rows = {};
          if (fetchRows) {
            rows = Server::fetchRows(connection, schemaName, tableName);
          }

          TableInfo tableInfo = TableInfo(tableName, schemaName, rows);
          cachedTables.push_back(tableInfo);
        }

      } catch (const std::exception &err) {
        HUMMINGBIRD_SQL_ASSERT(false && err.what());
      }

      return cachedTables;
    }

    std::vector<Row>
    Server::fetchRows(const Connection &connection, const std::string &schemaName, const std::string &tableName) {
      if (!connectionCheck) {
        HUMMINGBIRD_SQL_ASSERT(false && "Connection is not established");
      }

      std::vector<Row> cachedRows = {};
      std::vector<mysqlx::Row> serverRows = {};

      try {
        mysqlx::Schema schema = connection.getSession().getSchema(schemaName);
        mysqlx::Table table = schema.getTable(tableName);
        mysqlx::RowResult rowResult = table.select("*").limit(100).execute();
        serverRows = rowResult.fetchAll();

        for (mysqlx::Row &serverRow: serverRows) {
          int columnIndex = 0;
          std::vector<Sql::ColumnInfo> columns = {};

          for (const auto &column: rowResult.getColumns()) {
            const std::string columnName = column.getColumnLabel();
            ColumnValue value = std::nullopt;
            ColumnType columnType = NULL_TYPE;
            ColumnType currentType = NULL_TYPE;

            columnType = columnTypeToColumnType(column.getType());

            if (serverRow.isNull()) {
              value = std::nullopt;
              currentType = NULL_TYPE;
            } else {
              currentType = valueTypeToColumnType(serverRow.get(columnIndex));
              switch (currentType) {
                case ColumnType::INT:
                  value = serverRow.get(columnIndex).get<int>();
                  break;
                case ColumnType::FLOAT:
                  value = serverRow.get(columnIndex).get<float>();
                  break;
                case ColumnType::DOUBLE:
                  value = serverRow.get(columnIndex).get<double>();
                  break;
                case ColumnType::BOOL:
                  value = serverRow.get(columnIndex).get<bool>();
                  break;
                case ColumnType::STRING:
                  value = serverRow.get(columnIndex).get<std::string>();
                  break;
                default:
                  value = std::nullopt;
                  HUMMINGBIRD_SQL_WARNING_FUNCTION("Unknown column type for column " + columnName);
                  break;
              }
            }

            Sql::ColumnInfo columnInfo = Sql::ColumnInfo(columnName, columnType, value, currentType);
            columns.push_back(columnInfo);
            columnIndex++;
          }

          Row row(columns);
          cachedRows.push_back(row);
        }
      } catch (const std::exception &err) {
        HUMMINGBIRD_SQL_ASSERT(false && err.what());
      }

      return cachedRows;
    }
  }// namespace Server
}// namespace HummingBird::Sql