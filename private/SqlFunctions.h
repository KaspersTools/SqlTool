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
    std::unordered_map<std::string, std::unique_ptr<SchemaInfo>> getSchemas(const Connection &connection,
                                                                            const bool getTables,
                                                                            const bool getColumnsAndRows = false);
    std::unordered_map<std::string, TableInfo> getTables(const Connection &connection,
                                                         const SchemaInfo &databaseInfo,
                                                         const bool getColumnsAndRows = false);

    std::unordered_map<std::string, ColumnInfo> getTableColumns(const Connection &connection,
                                            const SchemaInfo &databaseInfo,
                                            const TableInfo &tableInfo);
    std::vector<Row> getTableRows(const Connection &connection,
                                  const SchemaInfo &databaseInfo,
                                  const TableInfo &tableInfo,
                                  int limit = Settings::Limits.DefaultRowLimit);
  };// namespace Server

  namespace Local {
    void getDatabases();
    void getTables();
    void getTableColumns();
    void getTableRows();
  }// namespace Local
}// namespace HummingBird::Sql
#endif//HUMMINGBIRD_SQL_TOOL_SQLFUNCTIONS_H
