#ifndef SRC_HELPERS_TYPELIST_H_
#define SRC_HELPERS_TYPELIST_H_

#include "type.h"

#include <cstdint>
#include <optional>

namespace compound_unit
{
/// Class template for storing a list of types.
template <class... TArgs>
struct TypeList
{
    /// Get the size of the type list.
    static constexpr std::size_t size()
    {
        return sizeof...(TArgs);
    };

    /// Check whether the type list is empty.
    static constexpr bool empty()
    {
        return size() == 0;
    }

    /// Get the type at the given index.
    template <std::size_t idx> // NOTE: wait pack indexing in C++26
    using type_at = std::tuple_element_t<idx, std::tuple<TArgs...>>;

    /// Check whether the given type is in the list.
    template <class T>
    static constexpr bool has_type{(std::same_as<T, TArgs> || ...)};

    /// Append one or several types to the end of the list.
    template <class... T>
    using push_back_t = TypeList<TArgs..., T...>;

    /// Prepend one or several types to the beginning of the list.
    template <class... T>
    using push_front_t = TypeList<T..., TArgs...>;
};

/// Concept for TypeList.
template <typename T>
concept TypeListConcept = type_helper::is_specialization_v<T, TypeList>;

namespace typelist_helper
{

// NOTE: gcc does not support wrapping this function as a lambda.
template <class... AArgs, class... BArgs>
consteval TypeListConcept auto type_list_cat_impl(TypeList<AArgs...>, TypeList<BArgs...>)
{
    return TypeList<AArgs..., BArgs...>{};
}

/// Concatenate two TypeLists.
template <TypeListConcept ListA, TypeListConcept ListB>
using typelist_cat_t = decltype(type_list_cat_impl(ListA{}, ListB{}));

///@{
template <class TargetType, class TArg, class... Rest>
consteval std::optional<std::size_t> pos_of_type_impl(const std::size_t idx,
                                                      const TypeList<TArg, Rest...>);

/**
 * Find the index of a type in a TypeList.
 *  If the type list is empty, or the type is not in the list, return
 * std::nullopt. Otherwise, return the position of the first occurance in the
 * list.
 */
template <TypeListConcept TTypeList, class T>
constexpr std::optional<std::size_t> pos_of_type_v{
    TTypeList::empty() ? std::nullopt : pos_of_type_impl<T>(0U, TTypeList{})};
///@}

///@{
template <class... TArgs>
consteval TypeListConcept auto remove_duplicated_type_impl(TypeList<TArgs...>);

/// Remove duplicated types from the given TypeList.
template <TypeListConcept TTypeList>
using remove_duplicated_type_t = decltype(remove_duplicated_type_impl(TTypeList{}));
///@}

/// Boolean that denotes whether tuple has each element type unique.
template <TypeListConcept TTypeList>
constexpr bool is_each_type_unique{TTypeList::size() ==
                                   remove_duplicated_type_t<TTypeList>::size()};

/// Union type of two TypeLists.
template <TypeListConcept ListA, TypeListConcept ListB>
using typelist_union_t = remove_duplicated_type_t<typelist_cat_t<ListA, ListB>>;

/// Check whether two TypeLists have the same set of element types.
template <TypeListConcept TypeListA, TypeListConcept TypeListB>
constexpr bool are_typelists_interchangeable_v{[]() -> bool {
    using UnionA_B = typelist_union_t<TypeListA, TypeListB>;
    using UnionB_A = typelist_union_t<TypeListB, TypeListA>;
    return std::same_as<UnionA_B, TypeListA> && std::same_as<UnionB_A, TypeListB>;
}()};

/// Make sepcialization of a struct template with TypeList as template
/// arguments.
///@{
template <template <typename...> class T, TypeListConcept TTypeList>
struct make_specialization;

template <template <typename...> class T, class... Args>
struct make_specialization<T, TypeList<Args...>>
{
    using type = T<Args...>;
};

template <template <typename...> class T, TypeListConcept TTypeList>
using make_specialization_t = make_specialization<T, TTypeList>::type;
///@}

} // namespace typelist_helper
} // namespace compound_unit

#include "typelist_impl.h"

#endif // SRC_HELPERS_TYPELIST_H_
