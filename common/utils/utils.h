#ifndef UTILS_H
#define UTILS_H

#include <cstddef>      // for ptrdiff_t, nullptr_t, size_t
#include <functional>   // for less, greater
#include <memory>       // for shared_ptr, unique_ptr, hash
#include <type_traits>  // for enable_if_t, is_convertible, is_assignable
#include <utility>      // for declval, forward
#include <iosfwd>        // for ostream

namespace utils{

// final_action allows you to ensure something gets run at the end of a scope
template <class F>
class final_action
{
  public:
    explicit final_action(const F& ff) noexcept : f{ff} { }
    explicit final_action(F&& ff) noexcept : f{std::move(ff)} { }

    ~final_action() noexcept { if (invoke) f(); }

    final_action(final_action&& other) noexcept
        : f(std::move(other.f)), invoke(std::exchange(other.invoke, false))
    { }

    final_action(const final_action&)   = delete;
    void operator=(const final_action&) = delete;
    void operator=(final_action&&)      = delete;

  private:
    F f;
    bool invoke = true;
};

// finally() - convenience function to generate a final_action
template <class F>
[[nodiscard]] auto finally(F&& f) noexcept
{
  return final_action<std::decay_t<F>>{std::forward<F>(f)};
}


template <class T>
class not_null
{
  public:
    static_assert(details::is_comparable_to_nullptr<T>::value, "T cannot be compared to nullptr.");

    using element_type = T;

    template <typename U, typename = std::enable_if_t<std::is_convertible<U, T>::value>>
    constexpr not_null(U&& u) noexcept(std::is_nothrow_move_constructible<T>::value) : ptr_(std::forward<U>(u))
    {
      Expects(ptr_ != nullptr);
    }

    template <typename = std::enable_if_t<!std::is_same<std::nullptr_t, T>::value>>
    constexpr not_null(T u) noexcept(std::is_nothrow_move_constructible<T>::value) : ptr_(std::move(u))
    {
      Expects(ptr_ != nullptr);
    }

    template <typename U, typename = std::enable_if_t<std::is_convertible<U, T>::value>>
    constexpr not_null(const not_null<U>& other) noexcept(std::is_nothrow_move_constructible<T>::value) : not_null(other.get())
    {}

    not_null(const not_null& other) = default;
    not_null& operator=(const not_null& other) = default;
    constexpr details::value_or_reference_return_t<T> get() const
        noexcept(noexcept(details::value_or_reference_return_t<T>(std::declval<T&>())))
    {
      return ptr_;
    }

    constexpr operator T() const { return get(); }
    constexpr decltype(auto) operator->() const { return get(); }
    constexpr decltype(auto) operator*() const { return *get(); }

    // prevents compilation when someone attempts to assign a null pointer constant
    not_null(std::nullptr_t) = delete;
    not_null& operator=(std::nullptr_t) = delete;

    // unwanted operators...pointers only point to single objects!
    not_null& operator++() = delete;
    not_null& operator--() = delete;
    not_null operator++(int) = delete;
    not_null operator--(int) = delete;
    not_null& operator+=(std::ptrdiff_t) = delete;
    not_null& operator-=(std::ptrdiff_t) = delete;
    void operator[](std::ptrdiff_t) const = delete;

    void swap(not_null<T>& other) { std::swap(ptr_, other.ptr_); }

  private:
    T ptr_;
};


template <class T, class U>
constexpr auto operator==(const not_null<T>& lhs,
                          const not_null<U>& rhs) noexcept(noexcept(lhs.get() == rhs.get()))
    -> decltype(lhs.get() == rhs.get())
{
  return lhs.get() == rhs.get();
}

template <class T, class U>
constexpr auto operator!=(const not_null<T>& lhs,
                          const not_null<U>& rhs) noexcept(noexcept(lhs.get() != rhs.get()))
    -> decltype(lhs.get() != rhs.get())
{
  return lhs.get() != rhs.get();
}

template <class T, class U>
constexpr auto operator<(const not_null<T>& lhs,
                         const not_null<U>& rhs) noexcept(noexcept(std::less<>{}(lhs.get(), rhs.get())))
    -> decltype(std::less<>{}(lhs.get(), rhs.get()))
{
  return std::less<>{}(lhs.get(), rhs.get());
}

template <class T, class U>
constexpr auto operator<=(const not_null<T>& lhs,
                          const not_null<U>& rhs) noexcept(noexcept(std::less_equal<>{}(lhs.get(), rhs.get())))
    -> decltype(std::less_equal<>{}(lhs.get(), rhs.get()))
{
  return std::less_equal<>{}(lhs.get(), rhs.get());
}

template <class T, class U>
constexpr auto operator>(const not_null<T>& lhs,
                         const not_null<U>& rhs) noexcept(noexcept(std::greater<>{}(lhs.get(), rhs.get())))
    -> decltype(std::greater<>{}(lhs.get(), rhs.get()))
{
  return std::greater<>{}(lhs.get(), rhs.get());
}

template <class T, class U>
constexpr auto operator>=(const not_null<T>& lhs,
                          const not_null<U>& rhs) noexcept(noexcept(std::greater_equal<>{}(lhs.get(), rhs.get())))
    -> decltype(std::greater_equal<>{}(lhs.get(), rhs.get()))
{
  return std::greater_equal<>{}(lhs.get(), rhs.get());
}


}  // utils

#endif // UTILS_H
