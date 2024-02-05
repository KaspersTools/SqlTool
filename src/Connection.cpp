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
      fetchSchemas(true, false);
    } catch (const mysqlx::Error &err) {
      HUMMINGBIRD_SQL_ASSERT(false && err.what());
    }

    if (database != "") {
      setSchema(database);
    } else {
      if(getSchemaNames().size() == 0){
        HUMMINGBIRD_SQL_ERROR_FUNCTION("No schemas found");
        return;
      }

      std::string schemaName = getSchemaNames()[0];
      setSchema(schemaName);

      if(getCurrentSchema().getTableNames().size() == 0){
        HUMMINGBIRD_SQL_ERROR_FUNCTION("No tables found in schema: " + schemaName);
        return;
      }
      std::string tableName = getCurrentSchema().getTableNames()[0];
      getCurrentSchema().setTable(tableName, *this);
    };
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
  void Connection::fetchSchemas(const bool getTables, const bool getColumnsAndRows) {
    m_schemas.clear();
    std::vector<SchemaInfo> schemas = Server::fetchSchemas(*this, getTables, getColumnsAndRows);
    for (SchemaInfo &schema: schemas) {
      m_schemas[schema.getName()] = std::make_unique<SchemaInfo>(schema);
    }
  }


  //Cache functions
  //getters
  std::vector<std::string> Connection::getSchemaNames() const {
    std::vector<std::string> schemaNames;
    for (const auto &schema: m_schemas) {
      schemaNames.push_back(schema.first);
    }
    return schemaNames;
  }

  SchemaInfo Connection::getSchema(const std::string &schemaName) const {
    if(m_schemas.size() <= 0){
      HUMMINGBIRD_SQL_ASSERT(false && "No schemas found");
      return SchemaInfo();
    }

    if(m_schemas.empty()){
      HUMMINGBIRD_SQL_ASSERT(false && "No schemas found");
      return SchemaInfo();
    }

    auto it = m_schemas.find(schemaName);
    if (it != m_schemas.end()) {
      return *it->second;
    } else {
      std::string err = "Schema not found" + schemaName;
      HUMMINGBIRD_SQL_ASSERT(false && err.c_str());
      return SchemaInfo();
    }
  }

  SchemaInfo Connection::getCurrentSchema() const {
    if(m_currentSchema == nullptr){
      HUMMINGBIRD_SQL_ASSERT(false && "No schema set");
      return SchemaInfo();
    }

    return *m_currentSchema;
  }

  //setters
  void Connection::setSchema(const std::string &schemaName) {
    if (!isConnected()) {
      HUMMINGBIRD_SQL_ERROR_FUNCTION("Not connected to database");
    }
    if (m_currentSchema != nullptr) {
      m_currentSchema = nullptr;
    }

    m_currentSchema = getSchemaPtrByName(schemaName);

    if (m_currentSchema == nullptr) {
      HUMMINGBIRD_SQL_ERROR_FUNCTION("Schema not found" + schemaName);
      return;
    }
  }

#pragma region private_functions
  SchemaInfo *Connection::getSchemaPtrByName(const std::string &schemaName) const {
    SchemaInfo *schemaInfo = nullptr;
    auto it = m_schemas.find(schemaName);
    if (it != m_schemas.end()) {
      schemaInfo = it->second.get();
    } else {
      HUMMINGBIRD_SQL_ERROR_FUNCTION("Schema not found" + schemaName);
    }
    return schemaInfo;
  }
#pragma endregion private_functions


}// namespace HummingBird::Sql