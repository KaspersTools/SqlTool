//
// Created by Kasper de Bruin on 31/01/2024.
//

#include <SqlFunctions.h>
#include <vector>

namespace HummingBird::Sql {
  namespace Server {
    void connectionCheck(const Connection &connection) {
      if (!connection.isConnected()) {
        HUMMINGBIRD_SQL_ASSERT(false && "Connection is not established");
      }
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

    std::unordered_map<std::string, std::unique_ptr<SchemaInfo>, case_insensitive_unordered_map::hash, case_insensitive_unordered_map::comp>
    Server::fetchSchemas(const Connection &connection, const bool fetchTables, const bool fetchColumnsAndRows) {
      HUMMINGBIRD_SQL_LOG_FUNCTION("Fetching schemas from server");
      connectionCheck(connection);

      std::unordered_map<std::string, std::unique_ptr<SchemaInfo>, case_insensitive_unordered_map::hash, case_insensitive_unordered_map::comp>
              cachedSchemas = {};
      std::vector<mysqlx::Schema> serverSchemas = {};

      try {
        HUMMINGBIRD_SQL_LOG_FUNCTION("Fetching schemas from server for schema");
        serverSchemas = connection.getSession().getSchemas();

        for (const mysqlx::Schema &serverSchema: serverSchemas) {
          const std::string schemaName = serverSchema.getName();

          if (schemaName == "information_schema" || schemaName == "mysql" || schemaName == "performance_schema" || schemaName == "sys") {
            continue;
          }

          std::unordered_map<std::string, std::unique_ptr<TableInfo>> tables;
          if (fetchTables) {
            tables = Server::fetchTables(connection, schemaName, fetchColumnsAndRows);
          }

          std::unique_ptr<SchemaInfo> schemaInfo = std::make_unique<SchemaInfo>(schemaName, std::move(tables));
          cachedSchemas[schemaName] = std::move(schemaInfo);
        }
      } catch (const mysqlx::Error &err) {
        HUMMINGBIRD_SQL_ASSERT(false && err.what());
        return {};
      }

      return cachedSchemas;
    }

    std::unordered_map<std::string, std::unique_ptr<TableInfo>>
    Server::fetchTables(const Connection &connection, const std::string &schemaName, const bool fetchColumnsAndRows) {
      HUMMINGBIRD_SQL_LOG_FUNCTION("Fetching tables from server for schema " + schemaName);
      connectionCheck(connection);

      std::unordered_map<std::string, std::unique_ptr<TableInfo>> cachedTables = {};
      std::vector<mysqlx::Table> serverTables = {};

      try {
        serverTables = connection.getSession().getSchema(schemaName).getTables();

        for (const mysqlx::Table &serverTable: serverTables) {
          const std::string tableName = serverTable.getName();
          std::unique_ptr<TableInfo> tableInfo = std::make_unique<TableInfo>(tableName, schemaName);
          cachedTables[tableName] = std::move(tableInfo);
        }

      } catch (const mysqlx::Error &err) {
        HUMMINGBIRD_SQL_ASSERT(false && err.what());
        return {};
      }

      return cachedTables;
    }

  }// namespace Server
}// namespace HummingBird::Sql