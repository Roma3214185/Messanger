#ifndef ENTITYCONCEPT_H
#define ENTITYCONCEPT_H

#include <nlohmann/json.hpp>

template <typename T>
concept JsonRoundTrip = requires(const nlohmann::json &j, const T &t) {
  { j.get<T>() } -> std::convertible_to<T>;
  { nlohmann::json(t) } -> std::same_as<nlohmann::json>;
};

// template <typename T>
// concept Entity = std::derived_from<T, IEntity>;

template <typename T>
concept Entity = requires(const T &t) { t.checkInvariants(); };

template <typename T>
concept EntityJson = Entity<T> && JsonRoundTrip<T>;

#endif  // ENTITYCONCEPT_H
