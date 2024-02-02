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
    m_schemas = Server::fetchSchemas(*this, getTables, getColumnsAndRows);
  }


  //Cache functions
  //gettersxw

  /**
     * @brief Get all schema names
     * @return std::vector<std::string> The schema names
     */
  const std::vector<std::string> Connection::getSchemaNames() const {
    std::vector<std::string> schemaNames;
    for (auto &schema: m_schemas) {
      schemaNames.push_back(schema.first);
    }
    return schemaNames;
  }


  /**
     * @brief Get the list of schemas in the database
     * @return std::vector<SchemaInfo> The list of schemas
     */
  const std::vector<std::unique_ptr<SchemaInfo>> Connection::getSchemas() const {
//    std::vector<SchemaInfo> schemas;
//    for (auto &schema: m_schemas) {
//      SchemaInfo copy = *schema.second;
//      schemas.push_back(copy);
//    }
//    return schemas;

      std::vector<std::unique_ptr<SchemaInfo>> schemas;
      for(const auto& [schemaName, schemaInfo] : m_schemas){
        std::unique_ptr<SchemaInfo> schemaInfoCopy = schemaInfo->getCopy();
        schemas.push_back(std::move(schemaInfoCopy));
      }

      return schemas;
  }

  //setters
  void Connection::setSchema(const std::string &schemaName) {
    SchemaInfo *schemaInfo = getSchemaPtrByName(schemaName);
    m_currentSchema = schemaInfo;
  }

  void Connection::setTable(const std::string& schameName, const std::string& tableName){
    setSchema(schameName);
    m_currentSchema->setTable(tableName);
  }

#pragma region private_functions
  SchemaInfo *Connection::getSchemaPtrByName(const std::string &schemaName) {
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