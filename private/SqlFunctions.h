//
// Created by Kasper de Bruin on 31/01/2024.
//

#ifndef HUMMINGBIRD_SQL_TOOL_SQLFUNCTIONS_H
#define HUMMINGBIRD_SQL_TOOL_SQLFUNCTIONS_H
#include <Hummingbird_SQL/Structures.h>

namespace HummingBird::Sql {
  namespace Server {

    std::vector<SchemaInfo>
    fetchSchemas(const Connection &connection, const bool fetchTables = true, const bool fetchRows = false);

    std::vector<TableInfo>
    fetchTables(const Connection &connection, const std::string &schemaName, const bool fetchRows = false);

    std::vector<Row>
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
