//
// Created by Kasper de Bruin on 29/01/2024.
//
#pragma once
//forward declare
namespace mysqlx::abi2::r0 {
  class Session;
}

namespace HummingBird::Sql {
  class Row;
  struct ColumnInfo;
  struct TableInfo;
  struct SchemaInfo;
}// namespace HummingBird::Sql

//aliasses
namespace mysqlx {
  using mysqlx::abi2::r0::Session;
}

#include <Hummingbird_SQL/Config.h>
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


#pragma region Server_Functions
#pragma region Schema functions
    void fetchSchemas(const bool fetchTables, const bool fetchColumnsAndRows);
#pragma endregion Schema functions

#pragma region Table functions
    void fetchTables(const std::string &schemaName, const bool fetchColumnsAndRows);
    void fetchTables(SchemaInfo &schema, const bool fetchColumnsAndRows);
#pragma endregion Table functions

#pragma region Column and Row functions
    //column and row functions
    void fetchColumns(const std::string &schemaName, const std::string &tableName);
    void fetchColumns(SchemaInfo &schema, const std::string &tableName);
    void fetchRows(const std::string &schemaNames, const std::string &tableName);
    void fetchRows(SchemaInfo &schema, const std::string &tableName);
#pragma endregion Column and Row functions
#pragma endregion Server_Functions

#pragma region Local_Functions
#pragma region Schema functions
    SchemaInfo *getSchema(const std::string &schemaName);
#pragma endregion Schema functions
#pragma region Table functions
    TableInfo *getTable(SchemaInfo& schema, const std::string &tableName);
    TableInfo *getTable(const std::string &schemaName, const std::string &tableName);
#pragma endregion Table functions

#pragma region Column and Row Functions

#pragma endregion Column and Row Functions

#pragma endregion Local_Functions

    //getters and setters
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