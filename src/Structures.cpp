//
// Created by Kasper de Bruin on 29/01/2024.
//
#include <Hummingbird_SQL/Connection.h>
#include <Hummingbird_SQL/Structures.h>
#include "SqlFunctions.h"

namespace HummingBird::Sql{
  void TableInfo::fetchRows(const Connection& connection){
    std::vector<Row> _rows;
    _rows = Server::fetchRows(connection, schemaName, name);

    columnCount = _rows[0].getColumnCount();

    for(auto& row : _rows){
      rows.push_back(std::make_shared<Row>(row));
    }
  }
}
