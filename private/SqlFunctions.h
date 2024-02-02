//
// Created by Kasper de Bruin on 31/01/2024.
//

#ifndef HUMMINGBIRD_SQL_TOOL_SQLFUNCTIONS_H
#define HUMMINGBIRD_SQL_TOOL_SQLFUNCTIONS_H
#include <xdevapi.h>//at top for forward declare

#include <Hummingbird_SQL/Config.h>
#include <Hummingbird_SQL/Connection.h>
#include <Hummingbird_SQL/Structures.h>
#include <iostream>

namespace HummingBird::Sql {
  namespace Server {
    std::unordered_map<std::string, std::unique_ptr<SchemaInfo>, case_insensitive_unordered_map::hash, case_insensitive_unordered_map::comp>
    fetchSchemas(const Connection &connection, const bool getTables = true, const bool getColumnsAndRows = false);

    std::unordered_map<std::string, std::unique_ptr<TableInfo>>
    fetchTables(const Connection &connection, const std::string &schemaName, const bool getColumnsAndRows = false);

    std::vector<std::unique_ptr<ColumnInfo>>
    fetchColumns(const Connection &connection, const std::string &schemaName, const std::string &tableName);

    std::vector<std::unique_ptr<Row>>
    fetchRows(const Connection &connection, const std::string &schemaName, const std::string &tableName);

  };// namespace Server

  namespace Local {
    void getDatabases();
    void getTables();
    void getTableColumns();
    void getTableRows();
  }// namespace Local
}// namespace HummingBird::Sql
#endif//HUMMINGBIRD_SQL_TOOL_SQLFUNCTIONS_H
