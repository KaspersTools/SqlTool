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

  void Connection::fetchSchemas(const bool getTables, const bool getColumnsAndRows) {
    m_schemas.clear();
    m_schemas = Server::getSchemas(*this, getTables, getColumnsAndRows);
  }

  void Connection::fetchTables(const std::string& databaseName, const bool getColumnsAndRows){
  }

  void Connection::fetchColumns(const std::string& databaseName, const std::string &tableName) {
  }

  void Connection::fetchRows(const std::string& databaseName, const std::string &tableName) {
  }

}// namespace HummingBird::Sql