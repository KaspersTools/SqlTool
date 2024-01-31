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

#include "Structures.h"
#include <Hummingbird_SQL/Config.h>
#include <cctype>
#include <exception>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

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
    * @brief Connection class for connecting to a MySQL database
    * @details This class is used to connect to a MySQL database and fetch the schemas, tables, columns and rows.
   */
  class Connection {
public:
    Connection();
    ~Connection();

    /**
     * @brief Connect to a MySQL database
     * @details Connect to a MySQL database with the given hostname, username, password, schemaName and port.
     * @param hostname The hostname of the MySQL database
     * @param username The username of the MySQL database
     * @param password The password of the MySQL database
     * @param schemaName The defaultSchemaName of the MySQL database
     * @param port The port of the MySQL database
     */
    void connect(const std::string &hostname, const std::string &username, const std::string &password,
                 const std::string &schemaName, uint16_t port);

    /**
     * @brief Disconnect from the MySQL database
     * @details Disconnect from the MySQL database
     */
    void disconnect();

    /**
     * @brief Use scheme
     * @param schemaName The schemaName to use
     */
    void setSchema(SchemaInfo* chema);

    /**
     * @brief Use table
     * @param table The table to use
     */
    void setTable(TableInfo *table);

    /**
     * @brief Fetch the current schema
     * @param fetchTables Fetch all tables
     * @param fetchColumnsAndRows Fetch all columns and rows
     * @return void
     */
    void fetchCurrentSchema(const bool fetchTables, const bool fetchColumnsAndRows, const uint rowLimit = Settings::Limits.DefaultRowLimit);

    //TODO: implement
    //    /**
    //     * @brief Fetch the current schema
    //     * @param fetchTables Fetch all tables
    //     * @param fetchColumnsAndRows Fetch all columns and rows
    //     * @return void
    //     */
    // void fetchSchema(const std::string &schemaName, const bool fetchTables, const bool fetchColumnsAndRows);

    /**
     * @brief Fetch all schemas, tables, columns and rows
     * @param fetchTables Fetch all tables
     * @param fetchColumnsAndRows Fetch all columns and rows
     * @return void
     */
    void fetchSchemas(const bool fetchTables, const bool fetchColumnsAndRows);

    /**
     * @brief Fetch all tables, columns and rows
     * @param schema The schema to fetch the tables from
     * @param fetchColumnsAndRows Fetch all columns and rows
     * @return void
     */
    void fetchTables(SchemaInfo &schema, const bool fetchColumnsAndRows);

    /**
     * @brief Fetch all columns and rows
     * @param schemaName The schemaName to fetch the columns and rows from
     * @param fetchColumnsAndRows Fetch all columns and rows
     * @return void
     */
    void fetchTables(const std::string &schemaName, const bool fetchColumnsAndRows);

    /**
     * @brief Fetch all tables, columns and rows
     * @param schema The schema to fetch the tables from
     * @param table The table to fetch the columns and rows from
     * @return void
     */
    void fetchColumns(SchemaInfo &schema, TableInfo &table);

    /**
     * @brief Fetch all columns and rows
     * @param schema The schema to fetch the columns and rows from
     * @param table The table name to fetch the columns and rows from
     * @return void
     */
    void fetchColumns(SchemaInfo &schema, const std::string &tableName);

    /**
     * @brief Fetch all columns and rows
     * @param schemaName The schemaName to fetch the columns and rows from
     * @param tableName The tableName to fetch the columns and rows from
     * @return void
     */
    void fetchColumns(const std::string &schemaName, const std::string &tableName);


    /**
     * @brief Fetch all rows
     * @param schema The schema to fetch the rows from
     * @param table The table to fetch the rows from
     * @return void
     */
    void fetchRows(SchemaInfo &schema, TableInfo &table, uint limit = Settings::Limits.DefaultRowLimit);

    /**
     *
     * @brief Fetch all rows
     * @param schemaName The schemaName to fetch the rows from
     * @param tableName The tableName to fetch the rows from
     * @return void
     */
    void fetchRows(SchemaInfo &schema, const std::string &tableName, uint limit = Settings::Limits.DefaultRowLimit);

    /**
     * @brief Fetch all rows
     * @param schemaName The schemaName to fetch the rows from
     * @param tableName The tableName to fetch the rows from
     * @return void
     */
    void fetchRows(const std::string &schemaNames, const std::string &tableName, uint limit = Settings::Limits.DefaultRowLimit);

    ///Getters///
    /**
     * @brief Get all schema names
     * @return std::vector<std::string> The schema names
     */
    std::vector<std::string> getSchemaNames() const {
      std::vector<std::string> schemaNames;
      for (auto &schema: m_schemas) {
        schemaNames.push_back(schema.first);
      }
      return schemaNames;
    }

    /**
     * @brief Get all schemas
     * @return std::vector<SchemaInfo> The schemas
     */
    std::vector<SchemaInfo *> getSchemas() const {
      std::vector<SchemaInfo *> schemas;
      for (auto &schema: m_schemas) {
        schemas.push_back(schema.second.get());
      }
      return schemas;
    }

    /**
      * @brief Get an schema by name
      * @param schemaName The name of the schema
      * @return SchemaInfo The schema
      */
    const SchemaInfo &getSchema(const std::string &schemaName);

    /**
     * @brief Get the current schema
     * @return SchemaInfo The schema
     */
    const SchemaInfo *getCurrentSchema() const {
      return m_currentSchema;
    }

    /**
     * @brief Get an table by name
     * @param schema The schema to get the table from
     * @param tableName The name of the table
     * @return TableInfo The table
     */
    const TableInfo &getTable(SchemaInfo &schema, const std::string &tableName);

    /**
     * @brief Get an list of tables
     * @param schema The schema to get the table from
     * @return std::vector<TableInfo> The tables
     */
    std::vector<TableInfo *> getTables(SchemaInfo *schema);

    /**
     * @brief Get an table by name
     * @param schemaName The schemaName to get the table from
     * @param tableName The name of the table
     * @return TableInfo The table
     */
    const TableInfo &getTable(const std::string &schema, const std::string &tableName);


    /**
     * @brief Get the current table
     * @return TableInfo The table
     */
    const TableInfo *getCurrentTable() const {
      return m_currentTable;
    }

    /**
     * @brief Get the current column layout
     * @return std::vector<std::string> The column layout
     */
    std::vector<std::string> getCurrentColumnLayout() const;


    /**
     * @brief Get connection status
     * @return bool True if connected
     */
    bool isConnected() const {
      return m_isConnected;
    }

    /**
     * @brief Get the current session ptr
     * @return Session ptr The session ptr
     */
    mysqlx::Session &getSession() const {
      return *session;
    }

private:
    /**
     * @brief Get an schema by name
     * @param schemaName The name of the schema
     * @return SchemaInfo ptr The schema
     * @return nullptr if the schema is not found
     */
    SchemaInfo *getSchemaPtr(const std::string &schemaName);

    /**
     * @brief Get an table by name
     * @param schema The schema to get the table from
     * @param tableName The name of the table
     * @return TableInfo ptr The table
     * @return nullptr if the table is not found
     */
    TableInfo *getTablePtr(SchemaInfo &schema, const std::string &tableName);

private:
    bool m_isConnected = false;

    SchemaInfo *m_currentSchema = nullptr;
    TableInfo *m_currentTable = nullptr;
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