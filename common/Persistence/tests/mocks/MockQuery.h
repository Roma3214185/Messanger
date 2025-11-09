#ifndef MOCKQUERY_H
#define MOCKQUERY_H

//#include <QSqlQuery>

// class MockQuery : public QSqlQuery {
//   public:
//     MockQuery() {
//       index_ = -1;
//     }

//     void setMockRecords(const std::vector<std::vector<QVariant>>& records)
//         : records_(records) {}

//     bool next() override {
//       ++index_;
//       return index_ < records_.size();
//     }

//     QVariant value(int column) override {
//       return records_[index_][column];
//     }

//   private:
//     std::vector<std::vector<QVariant>> records_;
//     int index_;
// };

#endif // MOCKQUERY_H
