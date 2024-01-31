//
// Created by Kasper de Bruin on 29/01/2024.
//
#pragma once
//forward declare
namespace mysqlx::abi2::r0 {
  class Session;
}

namespace HummingBird::Sql {
  struct SchemaInfo;
}// namespace HummingBird::Sql

//aliasses
namespace mysqlx {
  using mysqlx::abi2::r0::Session;
}

#include <iostream>
#include <string>
#include <unordered_map>

namespace HummingBird::Sql {
  class Connection {
public:
    Connection();
    ~Connection();

    // connection functions
    void connect(const std::string &hostname, const std::string &username, const std::string &password,
                 const std::string &schemaName, uint16_t port);
    void disconnect();

    // database functions
    void useDatabase(const std::string &schemaName);
    void fetchSchemas(const bool fetchTables, const bool fetchColumnsAndRows);
    void fetchTables(const std::string &databaseName, const bool fetchColumnsAndRows);
    void fetchColumns(const std::string &databaseName, const std::string &tableName);
    void fetchRows(const std::string &databaseName, const std::string &tableName);

    bool isConnected() const {
      return m_isConnected;
    }

    mysqlx::Session &getSession() const {
      return *session;
    }

private:
    bool m_isConnected = false;

    std::string m_currentDatabaseName;
    std::string m_host;
    std::string m_user;
    std::string m_password;
    uint16_t m_port;

    mysqlx::Session *session;
    std::unordered_map<std::string, std::unique_ptr<SchemaInfo>> m_schemas;
  };
}// namespace HummingBird::Sql