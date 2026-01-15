#ifndef REACTIONMETA_H
#define REACTIONMETA_H

#include "Fields.h"
#include "Meta.h"
#include "entities/Reaction.h"

template <>
struct Reflection<Reaction> {
  static Meta meta() {
    return Meta{.table_name = MessageReactionTable::Table,
                .fields = {make_field<Reaction, long long>(MessageReactionTable::MessageId, &Reaction::message_id),
                           make_field<Reaction, long long>(MessageReactionTable::ReceiverId, &Reaction::receiver_id),
                           make_field<Reaction, long long>(MessageReactionTable::ReactionId, &Reaction::reaction_id)}};
  }
};

template <>
struct Builder<Reaction> {
  static Reaction build(const QSqlQuery &query) {
    Reaction reaction;
    int idx = 0;

    auto assign = [&](auto &field) -> void {  // todo: in utils this function
      using TField = std::decay_t<decltype(field)>;
      const QVariant value = query.value(idx++);
      if constexpr (std::is_same_v<TField, long long>) {
        field = value.toLongLong();
      } else if constexpr (std::is_same_v<TField, int>) {
        field = value.toInt();
      } else {
        field = value.value<TField>();
      }
    };

    assign(reaction.message_id);
    assign(reaction.receiver_id);
    assign(reaction.reaction_id);

    return reaction;
  }
};

inline constexpr auto ReactionFields =
    std::make_tuple(&Reaction::message_id, &Reaction::receiver_id, &Reaction::reaction_id);

template <>
struct EntityFields<Reaction> {
  static constexpr auto &fields = ReactionFields;
};

template <>
struct EntityKey<Reaction> {
  static std::string get(const Reaction &entity) {
    return std::to_string(entity.message_id) + ", " + std::to_string(entity.receiver_id);
  }
};

#endif  // REACTIONMETA_H
