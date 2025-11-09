#ifndef SQLBUILDER_H
#define SQLBUILDER_H

#include <QList>
#include <QString>

#include "Meta.h"

struct SqlStatement {
  QString         query;
  QList<QVariant> values;
};

template <typename T>
class SqlBuilder {
 public:
  SqlStatement buildInsert(const Meta& meta, const T& entity, bool withReturningId);
};

#include "SqlBuilder.inl"

#endif  // SQLBUILDER_H
