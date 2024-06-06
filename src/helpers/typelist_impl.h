#ifndef SRC_HELPERS_TYPELIST_IMPL_H_
#define SRC_HELPERS_TYPELIST_IMPL_H_

#include "type.h"

namespace compound_unit::type_helper
{
///@{
template <TypeListConcept TTypeList, class... Args>
struct remove_duplicated_type_base;

template <class... TArgs, class U, class... UArgs>
struct remove_duplicated_type_base<TypeList<TArgs...>, U, UArgs...>
    : std::conditional_t<TypeList<TArgs...>::template has_type<U>,
                         remove_duplicated_type_base<TypeList<TArgs...>,
                                                     UArgs...>, // True
                         remove_duplicated_type_base<TypeList<TArgs..., U>,
                                                     UArgs...> // False
                         >
{};

template <class... TArgs>
struct remove_duplicated_type_base<TypeList<TArgs...>>
{
    using type = TypeList<TArgs...>;
};

template <class... TArgs>
consteval TypeListConcept auto remove_duplicated_type_impl(TypeList<TArgs...>)
{
    return typename remove_duplicated_type_base<TypeList<>, TArgs...>::type{};
}

///@}

template <class TargetType, class TArg, class... Rest>
consteval std::optional<std::size_t> pos_of_type_impl(const std::size_t idx,
                                                      const TypeList<TArg, Rest...>)
{
    if constexpr (std::same_as<TArg, TargetType>)
    {
        return idx;
    }
    else if constexpr (sizeof...(Rest) == 0)
    {
        return std::nullopt;
    }
    else
    {
        return pos_of_type_impl<TargetType>(idx + 1, TypeList<Rest...>{});
    }
}

} // namespace compound_unit::type_helper
#endif // SRC_HELPERS_TYPELIST_IMPL_H_
