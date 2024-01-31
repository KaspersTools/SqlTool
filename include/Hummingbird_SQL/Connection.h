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
#include <cctype>
#include <exception>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>

namespace HummingBird::Sql {
  // Makes unordered map keys case insensitive
  struct case_insensitive_unordered_map {
    struct comp {
      bool operator()(const std::string &lhs, const std::string &rhs) const {
        // On non Windows OS, use the function "strcasecmp" in #include <strings.h>
        return strcasecmp(lhs.c_str(), rhs.c_str()) == 0;
      }
    };
    struct hash {
      std::size_t operator()(std::string str) const {
        for (std::size_t index = 0; index < str.size(); ++index) {
          auto ch = static_cast<unsigned char>(str[index]);
          str[index] = static_cast<unsigned char>(std::tolower(ch));
        }
        return std::hash<std::string>{}(str);
      }
    };
  };

  /**
   *
   */
  class Connection {
public:
    Connection();
    ~Connection();

    // connection functions
    void connect(const std::string &hostname, const std::string &username, const std::string &password,
                 const std::string &schemaName, uint16_t port);
    void disconnect();

    void fetchSchemas(const bool fetchTables, const bool fetchColumnsAndRows);

    void fetchTables(SchemaInfo &schema, const bool fetchColumnsAndRows);
    void fetchTables(const std::string &schemaName, const bool fetchColumnsAndRows);

    void fetchColumns(SchemaInfo &schema, const std::string &tableName);
    void fetchColumns(SchemaInfo &schema, TableInfo &table);
    void fetchColumns(const std::string &schemaName, const std::string &tableName);

    void fetchRows(SchemaInfo &schema, TableInfo &table);
    void fetchRows(SchemaInfo &schema, const std::string &tableName);
    void fetchRows(const std::string &schemaNames, const std::string &tableName);


    const SchemaInfo &getSchema(const std::string &schemaName);
    const TableInfo &getTable(SchemaInfo &schema, const std::string &tableName);
    const TableInfo &getTable(const std::string &schema, const std::string &tableName);

    //getters and setters
    bool isConnected() const {
      return m_isConnected;
    }

    mysqlx::Session &getSession() const {
      return *session;
    }

private:
    SchemaInfo *getSchemaPtr(const std::string &schemaName);
    TableInfo *getTablePtr(SchemaInfo &schema, const std::string &tableName);

private:
    bool m_isConnected = false;

    std::string m_currentDatabaseName;
    std::string m_host;
    std::string m_user;
    std::string m_password;
    uint16_t m_port;

    mysqlx::Session *session;
    std::unordered_map<std::string, std::unique_ptr<SchemaInfo>,
                       case_insensitive_unordered_map::hash,
                       case_insensitive_unordered_map::comp>
            m_schemas;
  };
}// namespace HummingBird::Sql