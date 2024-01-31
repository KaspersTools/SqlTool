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


#pragma region Server_Functions

#pragma region Schema functions
  void Connection::fetchSchemas(const bool getTables, const bool getColumnsAndRows) {
    m_schemas.clear();
    m_schemas = Server::getSchemas(*this, getTables, getColumnsAndRows);

    if (m_schemas.empty()) {
      HUMMINGBIRD_SQL_SERVER_ERROR_FUNCTION("No schemas found");
    }
  }
#pragma endregion Schema functions

#pragma region Table functions
  void Connection::fetchTables(SchemaInfo &schema, const bool getColumnsAndRows) {
    schema.tables = Server::getTables(*this, schema, getColumnsAndRows);
  }
  void Connection::fetchTables(const std::string &databaseName, const bool getColumnsAndRows) {
    SchemaInfo *schemaInfo = getSchema(databaseName);
    if (schemaInfo == nullptr) {
      return;
    }

    fetchTables(*schemaInfo, getColumnsAndRows);
  }
#pragma endregion Table functions

#pragma region Column and Row functions
  void Connection::fetchColumns(SchemaInfo &schema, const std::string &tableName) {
    TableInfo *tableInfo = getTable(schema, tableName);
    if (tableInfo == nullptr) {
      return;
    }

    tableInfo->columns = Server::getTableColumns(*this, schema, *tableInfo);
  }
  void Connection::fetchColumns(const std::string &schemaName, const std::string &tableName) {
    SchemaInfo *schemaInfo = getSchema(schemaName);
    if (schemaInfo == nullptr) {
      return;
    }

    fetchColumns(*schemaInfo, tableName);
  }

  void Connection::fetchRows(SchemaInfo &schema, const std::string &tableName) {
    TableInfo *tableInfo = getTable(schema, tableName);
    if (tableInfo == nullptr) {
      return;
    }

    tableInfo->rows = Server::getTableRows(*this, schema, *tableInfo);
  }
  void Connection::fetchRows(const std::string &databaseName, const std::string &tableName) {
    SchemaInfo *schemaInfo = getSchema(databaseName);
    if (schemaInfo == nullptr) {
      return;
    }

    fetchRows(*schemaInfo, tableName);
  }
#pragma endregion Column and Row functions

#pragma endregion Server_Functions

#pragma region Local_Functions

#pragma region Schema functions
  SchemaInfo *Connection::getSchema(const std::string &schemaName) {
    if (m_schemas.find(schemaName) == m_schemas.end()) {
      HUMMINGBIRD_SQL_SERVER_ERROR_FUNCTION("Schema: " + schemaName + " not found");
      return nullptr;
    }
    return m_schemas[schemaName].get();
  }
#pragma endregion Schema functions
#pragma region Table functions
  TableInfo *Connection::getTable(HummingBird::Sql::SchemaInfo &schema, const std::string &tableName) {
    if (schema.tables.find(tableName) == schema.tables.end()) {
      HUMMINGBIRD_SQL_SERVER_ERROR_FUNCTION("Table: " + tableName + " not found");
      return nullptr;
    }
    return &schema.tables[tableName];
  }
  TableInfo *Connection::getTable(const std::string &schemaName, const std::string &tableName) {
    SchemaInfo *schemaInfo = getSchema(schemaName);
    if (schemaInfo == nullptr) {
      return nullptr;
    }

    return getTable(*schemaInfo, tableName);
  }
#pragma endregion Table functions

#pragma region Column and Row Functions


#pragma endregion Column and Row Functions


#pragma endregion Local_Functions

}// namespace HummingBird::Sql