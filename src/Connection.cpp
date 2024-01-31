//
// Created by Kasper de Bruin on 29/01/2024.
//
#include <xdevapi.h>//at top for forward declare

#include "../include/Hummingbird_SQL/Config.h"
#include "../include/Hummingbird_SQL/Connection.h"
#include "../include/Hummingbird_SQL/Structures.h"
#include "SqlFunctions.h"


namespace HummingBird::Sql {
  Connection::Connection() {
  }

  Connection::~Connection() {
    if (isConnected()) {
      disconnect();
    }
    if (session != nullptr) {
      delete session;
    }
    if (m_currentSchema != nullptr) {
      m_currentSchema = nullptr;
    }
  }

  void Connection::connect(const std::string &hostname, const std::string &username,
                           const std::string &password, const std::string &database,
                           uint16_t port) {
    if (isConnected()) {
      HUMMINGBIRD_SQL_ASSERT(false && "Already connected to database");
    }

    m_host = hostname;
    m_user = username;
    m_password = password;
    m_port = port;

    mysqlx::SessionSettings settings(hostname, port, username, password);

    try {
      session = new mysqlx::Session(settings);
      m_isConnected = true;
    } catch (const mysqlx::Error &err) {
      HUMMINGBIRD_SQL_ASSERT(false && err.what());
    }

    fetchSchemas(Settings::OnConnect.FetchTablesOnConnect, Settings::OnConnect.FetchColumnsAndRowsOnConnect);

//    if (!database.empty() || database != "" || database != " ") {
//      setSchema(database);
//    }
  }

  void Connection::disconnect() {
    if (!isConnected()) {
      HUMMINGBIRD_SQL_ASSERT(false && "Not connected to database");
    }

    try {
      session->close();
    } catch (const mysqlx::Error &err) {
      HUMMINGBIRD_SQL_ASSERT(false && "Error while closing connection");
    }

    m_isConnected = false;
  }

  //Fetch functions

  void Connection::fetchCurrentSchema(const bool fetchTables, const bool fetchColumnsAndRows, const uint rowLimit) {
    if (m_currentSchema == nullptr) {
      HUMMINGBIRD_SQL_ERROR_FUNCTION("No current schema set");
      return;
    }
    if (fetchTables) {
      Connection::fetchTables(*m_currentSchema, fetchColumnsAndRows);
    }
    if (fetchColumnsAndRows) {
      for (auto &[tableName, table]: m_currentSchema->tables) {
        Connection::fetchColumns(*m_currentSchema, table);
        Connection::fetchRows(*m_currentSchema, table, rowLimit);
      }
    }
  }

  void Connection::fetchSchemas(const bool getTables, const bool getColumnsAndRows) {
    m_schemas.clear();
    m_schemas = Server::getSchemas(*this, getTables, getColumnsAndRows);

    if (m_schemas.empty()) {
      HUMMINGBIRD_SQL_ERROR_FUNCTION("No schemas found");
    }
  }


  void Connection::fetchTables(SchemaInfo &schema, const bool getColumnsAndRows) {
    schema.tables.clear();
    schema.tables = Server::getTables(*this, schema, getColumnsAndRows);
  }

  void Connection::fetchTables(const std::string &databaseName, const bool getColumnsAndRows) {
    SchemaInfo *schemaInfo = getSchemaPtr(databaseName);
    if (schemaInfo == nullptr) {
      return;
    }
    fetchTables(*schemaInfo, getColumnsAndRows);
  }

  void Connection::fetchColumns(SchemaInfo &schema, TableInfo &table) {
    table.columns.clear();
    table.columns = Server::getTableColumns(*this, schema, table);
  }
  void Connection::fetchColumns(SchemaInfo &schema, const std::string &tableName) {
    TableInfo *tableInfo = getTablePtr(schema, tableName);
    if (tableInfo == nullptr) {
      return;
    }
    fetchColumns(schema, *tableInfo);
  }
  void Connection::fetchColumns(const std::string &schemaName, const std::string &tableName) {
    SchemaInfo *schemaInfo = getSchemaPtr(schemaName);
    if (schemaInfo == nullptr) {
      return;
    }
    fetchColumns(*schemaInfo, tableName);
  }

  void Connection::fetchRows(SchemaInfo &schema, TableInfo &table, const uint limit) {
    table.rows.clear();
    table.rows = Server::getTableRows(*this, schema, table, limit);
  }
  void Connection::fetchRows(SchemaInfo &schema, const std::string &tableName, const uint limit) {
    TableInfo *tableInfo = getTablePtr(schema, tableName);
    if (tableInfo == nullptr) {
      return;
    }
    fetchRows(schema, *tableInfo, limit);
  }
  void Connection::fetchRows(const std::string &databaseName, const std::string &tableName, const uint limit) {
    SchemaInfo *schemaInfo = getSchemaPtr(databaseName);
    if (schemaInfo == nullptr) {
      return;
    }
    fetchRows(*schemaInfo, tableName, limit);
  }

  //Cache functions
  const SchemaInfo &Connection::getSchema(const std::string &schemaName) {
    SchemaInfo *schemaInfo = getSchemaPtr(schemaName);
    if (schemaInfo == nullptr) {
      HUMMINGBIRD_SQL_ERROR_FUNCTION("Schema not found in cache " + schemaName);
    }
    return *schemaInfo;
  }

  const TableInfo &Connection::getTable(SchemaInfo &schema, const std::string &tableName) {
    TableInfo *tableInfo = getTablePtr(schema, tableName);
    if (tableInfo == nullptr) {
      HUMMINGBIRD_SQL_ERROR_FUNCTION("Table not found in cache " + tableName);
    }
    return *tableInfo;
  }

  //  void Connection::setSchema(const std::string &schemaName) {
  //    HUMMINGBIRD_SQL_LOG_FUNCTION("Setting schema to " + schemaName);
  //
  //    SchemaInfo *schemaInfo = getSchemaPtr(schemaName);
  //    m_currentSchema = schemaInfo;
  //
  //    if (schemaInfo == nullptr) {
  //      HUMMINGBIRD_SQL_ERROR_FUNCTION("Schema not found in cache " + schemaName);
  //      return;
  //    }
  //
  //    fetchCurrentSchema(true, false);
  //    m_currentTable = nullptr;
  //
  //    if (m_currentSchema->tables.empty()) {
  //      HUMMINGBIRD_SQL_ERROR_FUNCTION("No tables found in schema " + schemaName);
  //      return;
  //    }
  //
  //    setTable(m_currentSchema->tables.begin()->first);
  //  }
  //
  //  void Connection::setTable(const std::string &tableName) {
  //    HUMMINGBIRD_SQL_LOG_FUNCTION("Setting table to " + tableName);
  //
  //    if (m_currentSchema == nullptr) {
  //      HUMMINGBIRD_SQL_ERROR_FUNCTION("No current schema set");
  //      return;
  //    }
  //    TableInfo *tableInfo = getTablePtr(*m_currentSchema, tableName);
  //    m_currentTable = tableInfo;
  //
  //    if (tableInfo == nullptr) {
  //      HUMMINGBIRD_SQL_ERROR_FUNCTION("Table not found in cache " + tableName);
  //      return;
  //    }
  //
  //    fetchColumns(*m_currentSchema, *m_currentTable);
  //    fetchRows(*m_currentSchema, *m_currentTable, Settings::Limits.CurrentRowLimit);
  //  }]
  void Connection::setSchema(HummingBird::Sql::SchemaInfo *schema) {
    HUMMINGBIRD_SQL_LOG_FUNCTION("Setting schema to " + schema->name);

    SchemaInfo *schemaInfo = getSchemaPtr(schema->name);
    m_currentSchema = schemaInfo;

    if (schemaInfo == nullptr) {
      HUMMINGBIRD_SQL_ERROR_FUNCTION("Schema not found in cache " + schema->name);
      return;
    }

    fetchCurrentSchema(true, false);
    m_currentTable = nullptr;

    if (m_currentSchema->tables.empty()) {
      HUMMINGBIRD_SQL_ERROR_FUNCTION("No tables found in schema " + schema->name);
      return;
    }
  }
  void Connection::setTable(TableInfo *table) {
    HUMMINGBIRD_SQL_LOG_FUNCTION("Setting table to " + table->name);

    if (m_currentSchema == nullptr) {
      HUMMINGBIRD_SQL_ERROR_FUNCTION("No current schema set");
      return;
    }

    TableInfo *tableInfo = getTablePtr(*m_currentSchema, table->name);
    m_currentTable = tableInfo;

    if (tableInfo == nullptr) {
      HUMMINGBIRD_SQL_ERROR_FUNCTION("Table not found in cache " + table->name);
      return;
    }
  }


  std::vector<std::string> Connection::getCurrentColumnLayout() const {
    if (m_currentTable == nullptr) {
      HUMMINGBIRD_SQL_ERROR_FUNCTION("No current table set");
      return {};
    }

    std::vector<std::string> columnLayout = {};
    TableInfo &table = *m_currentTable;
    for (auto &[columnName, column]: table.columns) {
      columnLayout.push_back(columnName);
    }
    return columnLayout;
  }


#pragma region private_functions
  SchemaInfo *Connection::getSchemaPtr(const std::string &schemaName) {
    SchemaInfo *schemaInfo = nullptr;
    auto it = m_schemas.find(schemaName);
    if (it != m_schemas.end()) {
      schemaInfo = it->second.get();
    } else {
      HUMMINGBIRD_SQL_ERROR_FUNCTION("Schema not found" + schemaName);
    }
    return schemaInfo;
  }

  TableInfo *Connection::getTablePtr(SchemaInfo &schema, const std::string &tableName) {
    TableInfo *tableInfo = nullptr;
    auto it = schema.tables.find(tableName);
    if (it != schema.tables.end()) {
      tableInfo = &it->second;
    } else {
      HUMMINGBIRD_SQL_ERROR_FUNCTION("Table not found" + tableName);
    }
    return tableInfo;
  }
#pragma endregion private_functions


}// namespace HummingBird::Sql