#ifndef REACTIONINFOMETA_H
#define REACTIONINFOMETA_H

#include "Fields.h"
#include "Meta.h"
#include "entities/ReactionInfo.h"

template <>
struct Reflection<ReactionInfo> {
  static Meta meta() {
    return Meta{
        .table_name = MessageReactionInfoTable::Table,
        .fields = {make_field<ReactionInfo, long long>(MessageReactionInfoTable::Id, &ReactionInfo::id),
                   make_field<ReactionInfo, std::string>(MessageReactionInfoTable::Image, &ReactionInfo::image)}};
  }
};

template <>
struct Builder<ReactionInfo> {
  static ReactionInfo build(const QSqlQuery &query) {
    ReactionInfo reaction_info;
    int idx = 0;

    auto assign = [&](auto &field) -> void {  // todo: in utils this function
      using TField = std::decay_t<decltype(field)>;
      const QVariant value = query.value(idx++);
      if constexpr (std::is_same_v<TField, long long>) {
        field = value.toLongLong();
      } else if constexpr (std::is_same_v<TField, int>) {
        field = value.toInt();
      } else if constexpr (std::is_same_v<TField, std::string>)
        field = value.toString().toStdString();
      else {
        field = value.value<TField>();
      }
    };

    assign(reaction_info.id);
    assign(reaction_info.image);

    return reaction_info;
  }
};

inline constexpr auto ReactionInfoFields = std::make_tuple(&ReactionInfo::id, &ReactionInfo::image);

template <>
struct EntityFields<ReactionInfo> {
  static constexpr auto &fields = ReactionInfoFields;
};

#endif  // REACTIONINFOMETA_H
