//
// Created by Kasper de Bruin on 29/01/2024.
//

#pragma once

#include <exception>
#include <optional>
#include <string>

#include <assert.h>
#include <iostream>

#ifndef HUMMINGBIRD_SQL_ASSERT
#define HUMMINGBIRD_SQL_ASSERT(_EXPR) assert(_EXPR)// You can override in your Config.h
#endif

namespace Logging {
  struct source_loc {
    constexpr source_loc() = default;
    constexpr source_loc(const char *filename_in, int line_in, const char *funcname_in)
        : filename{filename_in},
          line{line_in},
          funcname{funcname_in} {}

    constexpr bool empty() const noexcept { return line == 0; }
    const char *filename{nullptr};
    int line{0};
    const char *funcname{nullptr};

    std::string toString() const {
      return std::string(filename) + ":" + std::string(funcname) + ":" + std::to_string(line) + " |";
    }
  };

  static void showTrace(source_loc src, const std::string& msg) {
    std::cout << "TRACE: " << src.toString() << " " << msg << std::endl;
  }

  static void showLog(source_loc src, const std::string& msg) {
    std::cout << "LOG: " << src.toString() << " " << msg << std::endl;
  }

  static void showError(source_loc src, const  std::string& msg) {
    std::cerr << "ERROR: " << src.toString() << " " << msg << std::endl;
  }

  static void showFatal(source_loc src, const std::string &msg) {
    std::cerr << "FATAL: " << src.toString() << " " << msg << std::endl;
  }
}// namespace Logging

#ifndef HUMMINGBIRD_SQL_LOG_LEVEL
#define HUMMINGBIRD_SQL_LOG_LEVEL_TRACE 0
#define HUMMINGBIRD_SQL_LOG_LEVEL_LOG 1
#define HUMMINGBIRD_SQL_LOG_LEVEL_ERROR 2
#define HUMMINGBIRD_SQL_LOG_LEVEL_FATAL 3
#define HUMMINGBIRD_SQL_LOG_LEVEL_NONE 4
#define HUMMINGBIRD_SQL_LOG_LEVEL HUMMINGBIRD_SQL_LOG_LEVEL_TRACE
#endif

#ifndef HUMMINGBIRD_SQL_LOG_ACTIVE
#define HUMMINGBIRD_SQL_LOG_ACTIVE true
#endif

#ifndef HUMMINGBIRD_SQL_TRACE_FUNCTION
#define HUMMINGBIRD_SQL_TRACE_FUNCTION(...)                                                \
  if (HUMMINGBIRD_SQL_LOG_ACTIVE && HUMMINGBIRD_SQL_LOG_LEVEL <= HUMMINGBIRD_SQL_LOG_LEVEL_TRACE) \
  ::Logging::showTrace(::Logging::source_loc(__FILE__, __LINE__, __FUNCTION__),                   \
                       __VA_ARGS__)
#endif

#ifndef HUMMINGBIRD_SQL_LOG_FUNCTION
#define HUMMINGBIRD_SQL_LOG_FUNCTION(...)                                                \
  if (HUMMINGBIRD_SQL_LOG_ACTIVE && HUMMINGBIRD_SQL_LOG_LEVEL <= HUMMINGBIRD_SQL_LOG_LEVEL_LOG) \
  ::Logging::showLog(::Logging::source_loc(__FILE__, __LINE__, __FUNCTION__),                   \
                     __VA_ARGS__)
#endif

#ifndef HUMMINGBIRD_SQL_ERROR_FUNCTION
#define HUMMINGBIRD_SQL_ERROR_FUNCTION(...)                                                \
  if (HUMMINGBIRD_SQL_LOG_ACTIVE && HUMMINGBIRD_SQL_LOG_LEVEL <= HUMMINGBIRD_SQL_LOG_LEVEL_ERROR) \
  ::Logging::showError(::Logging::source_loc(__FILE__, __LINE__, __FUNCTION__),                   \
                       __VA_ARGS__)
#endif

#ifndef HUMMINGBIRD_SQL_FATAL_FUNCTION
#define HUMMINGBIRD_SQL_FATAL_FUNCTION(...)                                                \
  if (HUMMINGBIRD_SQL_LOG_ACTIVE && HUMMINGBIRD_SQL_LOG_LEVEL <= HUMMINGBIRD_SQL_LOG_LEVEL_FATAL) \
  ::Logging::showFatal(::Logging::source_loc(__FILE__, __LINE__, __FUNCTION__),                   \
                       __VA_ARGS__)
#endif


namespace Settings {
#ifndef HUMMINGBIRD_SQL_SETTINGS_FETCH_TABLES_ON_CONNECT
#define HUMMINGBIRD_SQL_SETTINGS_FETCH_TABLES_ON_CONNECT true
#endif

#ifndef HUMMINGBIRD_SQL_SETTINGS_FETCH_COLUMNS_ON_CONNECT
#define HUMMINGBIRD_SQL_SETTINGS_FETCH_COLUMNS_AND_ROWS_ON_CONNECT false
#endif

#ifndef HUMMINGBIRD_SQL_SETTINGS_DEFAULT_ROW_LIMIT
#define HUMMINGBIRD_SQL_SETTINGS_DEFAULT_ROW_LIMIT 10
#endif

  static struct {
    bool FetchTablesOnConnect = HUMMINGBIRD_SQL_SETTINGS_FETCH_TABLES_ON_CONNECT;
    bool FetchColumnsAndRowsOnConnect = HUMMINGBIRD_SQL_SETTINGS_FETCH_COLUMNS_AND_ROWS_ON_CONNECT;
  } OnConnect;

  static struct {
    uint32_t DefaultRowLimit = HUMMINGBIRD_SQL_SETTINGS_DEFAULT_ROW_LIMIT;
    uint32_t CurrentRowLimit = DefaultRowLimit;
  } Limits;
}// namespace Settings