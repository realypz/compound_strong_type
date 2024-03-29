#ifndef SRC_HELPERS_TYPE_H_
#define SRC_HELPERS_TYPE_H_

#include <cstdint>
#include <optional>

namespace compound_unit::type_helper
{
/// Determine whether a class T is a specialization of a class template Ref.
///  Works only with "type" arguments, not for "non-type" arguments.
///@{
template <typename T, template <typename...> class Ref>
struct is_specialization : std::false_type
{};

template <template <typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type
{};

template <typename T, template <typename...> class Ref>
constexpr bool is_specialization_v{is_specialization<T, Ref>::value};

///@}
} // namespace compound_unit::type_helper

#endif // SRC_HELPERS_TYPE_H_
