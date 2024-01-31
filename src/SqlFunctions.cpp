//
// Created by Kasper de Bruin on 31/01/2024.
//

#include <SqlFunctions.h>

namespace HummingBird::Sql {
  namespace Server {
    std::unordered_map<std::string, std::unique_ptr<SchemaInfo>,
                       case_insensitive_unordered_map::hash,
                       case_insensitive_unordered_map::comp>
    getSchemas(const Connection &connection,
               const bool getTables,
               const bool getColumnsAndRows) {
      if (!connection.isConnected()) {
        HUMMINGBIRD_SQL_ASSERT(false && "Connection is not established");
        return {};
      }
      std::unordered_map<std::string, std::unique_ptr<SchemaInfo>,
                         case_insensitive_unordered_map::hash,
                         case_insensitive_unordered_map::comp>
              schemaInfos = {};
      std::vector<mysqlx::Schema> serverSchemas = {};

      try {
        HUMMINGBIRD_SQL_LOG_FUNCTION("Fetching schemas from server");
        serverSchemas = connection.getSession().getSchemas();
      } catch (const mysqlx::Error &err) {
        HUMMINGBIRD_SQL_ASSERT(false && err.what());
        return {};
      }
      int schemaIndex = 0;
      int schemaCount = serverSchemas.size();

      for (const mysqlx::Schema &schema: serverSchemas) {
        std::string schemaName = schema.getName();
        HUMMINGBIRD_SQL_TRACE_FUNCTION("Fetching schema " + schemaName + " (" + std::to_string(schemaIndex + 1) + "/" + std::to_string(schemaCount) + ")");
        std::unique_ptr<SchemaInfo> schemaInfo = std::make_unique<SchemaInfo>();
        schemaInfo->name = schemaName;
        if (getTables) {
          schemaInfo->tables = Server::getTables(connection, *schemaInfo, getColumnsAndRows);
        }

        schemaInfos[schemaName] = std::move(schemaInfo);
        schemaIndex++;
      }

      return schemaInfos;
    }

    std::unordered_map<std::string, TableInfo> getTables(const Connection &connection,
                                                         const SchemaInfo &databaseInfo,
                                                         const bool getColumnsAndRows) {
      if (!connection.isConnected()) {
        HUMMINGBIRD_SQL_ASSERT(false && "Connection is not established");
        return {};
      }
      std::unordered_map<std::string, TableInfo> tableInfos = {};
      std::vector<mysqlx::Table> serverTables = {};

      try {
        HUMMINGBIRD_SQL_LOG_FUNCTION("Fetching tables from database: " + databaseInfo.name);
        serverTables = connection.getSession().getSchema(databaseInfo.name).getTables();
      } catch (const mysqlx::Error &err) {
        HUMMINGBIRD_SQL_ASSERT(false && err.what());
        return {};
      }

      int tableIndex = 0;
      for (mysqlx::Table &table: serverTables) {
        //todo: add support for views
        if (table.isView()) {
          tableIndex++;
          continue;
        }

        TableInfo tbInfo = {};
        tbInfo.schemaName = databaseInfo.name;
        tbInfo.name = table.getName();

        HUMMINGBIRD_SQL_TRACE_FUNCTION("Fetching table from database: " + databaseInfo.name + " table: " + tbInfo.name + " (" + std::to_string(tableIndex + 1) + "/" + std::to_string(serverTables.size()) + ")");

        if (getColumnsAndRows) {
          tbInfo.columns = Server::getTableColumns(connection, databaseInfo, tbInfo);
          tbInfo.rows = Server::getTableRows(connection, databaseInfo, tbInfo);
        }

        tableInfos[tbInfo.name] = tbInfo;
        tableIndex++;
      }

      return tableInfos;
    }

    /**
     * Used to get the "headers" of a table
     */
    std::unordered_map<std::string, ColumnInfo> Server::getTableColumns(const Connection &connection,
                                                                        const SchemaInfo &databaseInfo,
                                                                        const TableInfo &tableInfo) {
      if (!connection.isConnected()) {
        HUMMINGBIRD_SQL_ASSERT(false && "Connection is not established");
        return {};
      }
      std::unordered_map<std::string, ColumnInfo> columnInfos = {};
      mysqlx::RowResult rowResult;
      uint16_t columnCount = 0;

      try {
        mysqlx::Schema schema = connection.getSession().getSchema(databaseInfo.name);
        mysqlx::Table table = schema.getTable(tableInfo.name);
        rowResult = table.select("*").execute();
        columnCount = rowResult.getColumnCount();
      } catch (const mysqlx::Error &err) {
        HUMMINGBIRD_SQL_ASSERT(false && err.what());
        return {};
      }

      if (columnCount == 0) {
        return {};
      }

      for (uint16_t i = 0; i < columnCount; i++) {
        std::string lbl = rowResult.getColumn(i).getColumnLabel();
        HUMMINGBIRD_SQL_TRACE_FUNCTION("Fetching column from database: " + databaseInfo.name +
                                       " table: " + tableInfo.name +
                                       " column: " + lbl +
                                       " (" + std::to_string(i + 1) + "/" + std::to_string(columnCount) + ")");


        ColumnInfo clInfo(lbl, ColumnType::HEADER);
        columnInfos[lbl] = clInfo;
      }
      return columnInfos;
    }

    std::vector<Row> Server::getTableRows(const Connection &connection,
                                          const SchemaInfo &databaseInfo,
                                          const TableInfo &tableInfo,
                                          uint limit) {
      if (!connection.isConnected()) {
        HUMMINGBIRD_SQL_ASSERT(false && "Connection is not established");
        return {};
      }

      std::vector<mysqlx::Row> serverRows = {};
      std::vector<Row> rows = {};

      try {
        HUMMINGBIRD_SQL_LOG_FUNCTION("Fetching rows from database: " + databaseInfo.name + " table: " + tableInfo.name);
        mysqlx::Schema schema = connection.getSession().getSchema(databaseInfo.name);
        mysqlx::Table table = schema.getTable(tableInfo.name);
        mysqlx::RowResult rowResult = table.select("*").limit(limit).execute();
        serverRows = rowResult.fetchAll();
      } catch (const mysqlx::Error &err) {
        HUMMINGBIRD_SQL_ASSERT(false && err.what());
        return {};
      }

      if (serverRows.empty()) {
        return {};
      }

      int rowIndex = 0;
      for (const auto &serverRow: serverRows) {
        if (serverRow.isNull()) {
          rowIndex++;
          continue;
        }
        Row rowInfo = {};
        uint16_t columnIndex = 0;

        for (const auto &column: tableInfo.columns) {
          ColumnValue val = std::nullopt;
          ColumnType type = ColumnType::NULL_TYPE;

          HUMMINGBIRD_SQL_TRACE_FUNCTION("Fetching row from database: " + databaseInfo.name + " table: " + tableInfo.name + " column: " + column.first + " (" + std::to_string(rowIndex + 1) + "/" + std::to_string(serverRows.size()) + ")");
          try {
            switch (serverRow[columnIndex].getType()) {
              case mysqlx::Value::Type::VNULL:///< Null value
                val = std::nullopt;
                type = ColumnType::NULL_TYPE;
                break;
              case mysqlx::Value::Type::UINT64:///< Unsigned integer
                val = serverRow[columnIndex].get<uint64_t>();
                type = ColumnType::UINT64;
                break;
              case mysqlx::Value::Type::INT64:///< Signed integer
                val = serverRow[columnIndex].get<int64_t>();
                type = ColumnType::INT64;
                break;
              case mysqlx::Value::Type::FLOAT:///< Float number
                val = serverRow[columnIndex].get<float>();
                type = ColumnType::FLOAT;
                break;
              case mysqlx::Value::Type::DOUBLE:///< Double number
                val = serverRow[columnIndex].get<double>();
                type = ColumnType::DOUBLE;
                break;
              case mysqlx::Value::Type::BOOL:///< Boolean
                val = serverRow[columnIndex].get<bool>();
                type = ColumnType::BOOL;
                break;
              case mysqlx::Value::Type::STRING:///< String
                val = serverRow[columnIndex].get<std::string>();
                type = ColumnType::STRING;
                break;
                //todo: add support for document
                // case mysqlx::Value::Type::DOCUMENT:///< Document
                // val = serverRow[columnIndex].get<std::string>();
                // break;

                //todo: add support for raw
                // case mysqlx::Value::Type::RAW:///< Raw bytes
                // val = serverRow[columnIndex].get<std::string>();
                // type = ColumnType::RAW;
                // break;

                //todo: add support for array
                // case mysqlx::Value::Type::ARRAY:///< Array of values
                // val = serverRow[columnIndex].get<std::string>();
                // break;
              default:
                HUMMINGBIRD_SQL_ERROR_FUNCTION("Unkown column type: " + std::to_string(serverRow[columnIndex].getType()) + "for column: " + column.first);
                val = std::nullopt;
                type = ColumnType::NULL_TYPE;
                break;
            }
          } catch (const mysqlx::Error &err) {
            HUMMINGBIRD_SQL_ASSERT(false && err.what());
            val = std::nullopt;
          }

          ColumnInfo clInfo(column.first, type, val);
          rowInfo.addColumn(column.first, clInfo);
          columnIndex++;
        }

        rowIndex++;
        rows.push_back(rowInfo);
      }

      return rows;
    }
  }// namespace Server
}// namespace HummingBird::Sql