/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2022/5/22.
//

#include "sql/stmt/update_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "common/log/log.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "sql/parser/date.h"

UpdateStmt::UpdateStmt(Table *table, const Value &value, const std::string &field_name, FilterStmt *filter_stmt)
    : table_(table), value_(value), field_name_(field_name), filter_stmt_(filter_stmt)
{}

RC UpdateStmt::create(Db *db, const UpdateSqlNode &update, Stmt *&stmt)
{
  const char *table_name = update.relation_name.c_str();
  if (nullptr == db || nullptr == table_name) {
    LOG_WARN("invalid argument. db=%p, table_name=%p", db, table_name);
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  Table *table = db->find_table(table_name);
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  const int field_num     = table->table_meta().field_num() - table->table_meta().sys_field_num();
  const int sys_field_num = table->table_meta().sys_field_num();
  int       i             = 0;
  for (i = 0; i < field_num; i++) {
    if (update.attribute_name == table->table_meta().field(i + sys_field_num)->name()) {
      break;
    }
  }
  if (i == field_num) {
    LOG_WARN("no such field. field_name=%s", update.attribute_name.c_str());
    return RC::SCHEMA_FIELD_NOT_EXIST;
  }

  const std::string field_name = update.attribute_name;

  Value          value      = update.value;
  const AttrType field_type = table->table_meta().field(i + sys_field_num)->type();
  const AttrType value_type = update.value.attr_type();
  if (field_type == AttrType::DATES && value_type == AttrType::CHARS) {
    Date date;
    if (!string_to_date(value.get_string().c_str(), date)) {
      LOG_WARN("invalid date format. table=%s, field=%s, value=%s",
            table_name, field_name, value.get_string().c_str());
      return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    } else {
      value.set_date(date);
    }
  } else {
    if (field_type != value_type) {
      LOG_WARN("field type mismatch. field type=%d, value type=%d", field_type, value_type);
      return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    }
  }

  std::unordered_map<std::string, Table *> table_map;
  table_map.insert(std::pair<std::string, Table *>(std::string(table_name), table));

  FilterStmt *filter_stmt = nullptr;
  RC          rc          = FilterStmt::create(
      db, table, &table_map, update.conditions.data(), static_cast<int>(update.conditions.size()), filter_stmt);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create filter statement. rc=%d:%s", rc, strrc(rc));
    return rc;
  }

  stmt = new UpdateStmt(table, value, field_name, filter_stmt);
  return RC::SUCCESS;
}
