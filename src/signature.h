#ifndef SRC_SIGNATURE_H_
#define SRC_SIGNATURE_H_

#include "helpers/number.h"
#include "helpers/type.h"

namespace compound_unit
{
/**
 * Unit signature denotes a physical unit's property, numerical scale and its dimension (or
 * exponent).
 * @tparam _Period the numerical scale of the one-dimentional unit represented by _Tag.
 * @tparam _Exp the exponent of this unit, e.g. for qubic meter (where tag is length) and _Exp == 3.
 * @tparam _Tag the tag for its one dimentional physical unit.
 */
template <number_helper::RatioConcept _Period, std::int32_t _Exp, class _Tag>
requires(_Exp != 0)
struct UnitSignature
{
    using Period = _Period;
    static constexpr std::int32_t Exp{_Exp};
    using Tag = _Tag;
};

template <class T>
struct is_unit_signature : std::false_type
{};

template <number_helper::RatioConcept _Period, std::int32_t _Exp, class _Tag>
struct is_unit_signature<UnitSignature<_Period, _Exp, _Tag>> : std::true_type
{};

/**
 * Concept for unit signature.
 * @tparam T the type to check.
 */
template <class T>
concept UnitSignatureConcept = is_unit_signature<T>::value;

/**
 * Inverse of a unit signature.
 * @details The exponent is negated. The period and Tag is unchanged.
 * @tparam T the unit signature.
 */
template <UnitSignatureConcept T>
using inverse_signature_t = UnitSignature<typename T::Period, -T::Exp, typename T::Tag>;

namespace signature_helper
{
struct NullSignature
{};

template <class XTag, UnitSignatureConcept... Signatures>
consteval auto extractSignatureFromList(XTag, type_helper::TypeList<Signatures...>)
{
    using SignaturesList = type_helper::TypeList<Signatures...>;
    using TagsList = type_helper::TypeList<typename Signatures::Tag...>;

    if constexpr (TagsList::template has_type<XTag>)
    {
        constexpr std::size_t pos{*type_helper::pos_of_type_v<TagsList, XTag>};
        using RetType = SignaturesList::template type_at<pos>;
        return RetType{};
    }
    else
    {
        return NullSignature{};
    };
}

template <class XTag, UnitSignatureConcept... LSignatures, UnitSignatureConcept... RSignatures>
consteval auto extractCommonSignature(XTag, type_helper::TypeList<LSignatures...>,
                                      type_helper::TypeList<RSignatures...>)
{
    using LhsSignature =
        decltype(extractSignatureFromList(XTag{}, type_helper::TypeList<LSignatures...>{}));
    using RhsSignature =
        decltype(extractSignatureFromList(XTag{}, type_helper::TypeList<RSignatures...>{}));

    if constexpr (!std::same_as<NullSignature, LhsSignature> &&
                  !std::same_as<NullSignature, RhsSignature>)
    {
        constexpr std::int32_t exp_sum{LhsSignature::Exp + RhsSignature::Exp};

        if constexpr (exp_sum == 0)
        {
            return NullSignature{};
        }
        else
        {
            using CommonPeriod = number_helper::ratio_gcd<typename LhsSignature::Period,
                                                          typename RhsSignature::Period>::type;
            using CommonSignature = UnitSignature<CommonPeriod, exp_sum, XTag>;
            return CommonSignature{};
        }
    }
    else if constexpr (!std::same_as<NullSignature, LhsSignature> &&
                       std::same_as<NullSignature, RhsSignature>)
    {
        return LhsSignature{};
    }
    else if constexpr (std::same_as<NullSignature, LhsSignature> &&
                       !std::same_as<NullSignature, RhsSignature>)
    {
        return RhsSignature{};
    }
    else
    {
        return NullSignature{};
    }
}

template <class XTag, UnitSignatureConcept... LSignatures, UnitSignatureConcept... RSignatures>
consteval number_helper::RatioConcept auto extractScalingRatioContributionByTag(
    XTag, type_helper::TypeList<LSignatures...>, type_helper::TypeList<RSignatures...>)
{
    using LhsSignature =
        decltype(extractSignatureFromList(XTag{}, type_helper::TypeList<LSignatures...>{}));
    using RhsSignature =
        decltype(extractSignatureFromList(XTag{}, type_helper::TypeList<RSignatures...>{}));

    if constexpr (!std::same_as<NullSignature, LhsSignature> &&
                  !std::same_as<NullSignature, RhsSignature>)
    {
        using CommonPeriod = number_helper::ratio_gcd<typename LhsSignature::Period,
                                                      typename RhsSignature::Period>::type;

        using LeftContribution = number_helper::ratio_pow_t<
            std::ratio_divide<typename LhsSignature::Period, CommonPeriod>, LhsSignature::Exp>;
        using RightContribution = number_helper::ratio_pow_t<
            std::ratio_divide<typename RhsSignature::Period, CommonPeriod>, RhsSignature::Exp>;

        using ScalingRatio = std::ratio_multiply<LeftContribution, RightContribution>;
        return ScalingRatio{};
    }
    else
    {
        return std::ratio<1, 1>{};
    }
}

template <class _SavedSignaturesList, class _TSignature, class... _USignatures>
consteval type_helper::TypeListConcept auto remove_cancelled_out_signatures_impl()
{
    using NextSavedSignatures =
        std::conditional_t<UnitSignatureConcept<_TSignature>,
                           typename _SavedSignaturesList::template push_back_t<_TSignature>,
                           _SavedSignaturesList>;

    if constexpr (sizeof...(_USignatures) != 0)
    {
        return remove_cancelled_out_signatures_impl<NextSavedSignatures, _USignatures...>();
    }
    else
    {
        return NextSavedSignatures{};
    }
}

template <class... Signatures>
consteval auto removeCancelledOutSignatures(type_helper::TypeList<Signatures...> input)
{
    return remove_cancelled_out_signatures_impl<type_helper::TypeList<>, Signatures...>();
}

// Returns a TypeList of UnitSignatures (can be empty).
template <UnitSignatureConcept... _LSignatures, UnitSignatureConcept... _RSignatures>
consteval type_helper::TypeListConcept auto computeMultiplicationSignatures_impl(
    type_helper::TypeList<_LSignatures...>, type_helper::TypeList<_RSignatures...>)
{
    using LTagsList = type_helper::TypeList<typename _LSignatures::Tag...>;
    using RTagsList = type_helper::TypeList<typename _RSignatures::Tag...>;
    using TagsList = type_helper::typelist_union_t<LTagsList, RTagsList>;

    constexpr auto impl = []<std::size_t... Is>(std::index_sequence<Is...>) -> auto {
        // TypeList of UnitSignatures or NullSignatures.
        using CombinedSignatures = type_helper::TypeList<decltype(extractCommonSignature(
            typename TagsList::template type_at<Is>{}, // Tag[Is]
            type_helper::TypeList<_LSignatures...>{},
            type_helper::TypeList<_RSignatures...>{}))...>;

        using ReturnSignatures = decltype(removeCancelledOutSignatures(CombinedSignatures{}));
        return ReturnSignatures{};
    };

    return impl(std::make_index_sequence<TagsList::size()>());
}
} // namespace signature_helper

} // namespace compound_unit
#endif // SRC_SIGNATURE_H_
