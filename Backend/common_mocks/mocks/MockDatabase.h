#ifndef MOCKDATABASE_H
#define MOCKDATABASE_H

#include "SQLiteDataBase.h"

class MockDatabase : public IDataBase {
 public:
  using IDataBase::IDataBase;
};

#endif  // MOCKDATABASE_H
