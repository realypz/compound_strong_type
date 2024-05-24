#ifndef _SRC_SRC_COMPOUND_UNIT_IMPL_H_
#define _SRC_SRC_COMPOUND_UNIT_IMPL_H_

#include "src/compound_unit.h"
#include "src/helpers/typelist.h"

namespace compound_unit::impl
{
struct NullSignature
{};

template <class XTag, UnitSignatureConcept... Signatures>
consteval auto extractSignatureFromList(XTag, TypeList<Signatures...>)
{
    using SignaturesList = TypeList<Signatures...>;
    using TagsList = TypeList<typename Signatures::Tag...>;

    if constexpr (TagsList::template has_type<XTag>)
    {
        constexpr std::size_t pos{*typelist_helper::pos_of_type_v<TagsList, XTag>};
        using RetType = SignaturesList::template type_at<pos>;
        return RetType{};
    }
    else
    {
        return NullSignature{};
    };
}

template <class XTag, UnitSignatureConcept... LSignatures, UnitSignatureConcept... RSignatures>
consteval auto extractCommonSignature(XTag, TypeList<LSignatures...>, TypeList<RSignatures...>)
{
    using LhsSignature = decltype(extractSignatureFromList(XTag{}, TypeList<LSignatures...>{}));
    using RhsSignature = decltype(extractSignatureFromList(XTag{}, TypeList<RSignatures...>{}));

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
    XTag, TypeList<LSignatures...>, TypeList<RSignatures...>)
{
    using LhsSignature = decltype(extractSignatureFromList(XTag{}, TypeList<LSignatures...>{}));
    using RhsSignature = decltype(extractSignatureFromList(XTag{}, TypeList<RSignatures...>{}));

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
consteval TypeListConcept auto remove_cancelled_out_signatures_impl()
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
consteval auto removeCancelledOutSignatures(TypeList<Signatures...> input)
{
    return remove_cancelled_out_signatures_impl<TypeList<>, Signatures...>();
}

// Returns a TypeList of UnitSignatures (can be empty).
template <UnitSignatureConcept... _LSignatures, UnitSignatureConcept... _RSignatures>
consteval TypeListConcept auto computeMultiplicationSignatures_impl(TypeList<_LSignatures...>,
                                                                    TypeList<_RSignatures...>)
{
    using LTagsList = TypeList<typename _LSignatures::Tag...>;
    using RTagsList = TypeList<typename _RSignatures::Tag...>;
    using TagsList = typelist_helper::typelist_union_t<LTagsList, RTagsList>;

    constexpr auto impl = []<std::size_t... Is>(std::index_sequence<Is...>) -> auto {
        // TypeList of UnitSignatures or NullSignatures.
        using CombinedSignatures = TypeList<decltype(extractCommonSignature(
            typename TagsList::template type_at<Is>{}, // Tag[Is]
            TypeList<_LSignatures...>{}, TypeList<_RSignatures...>{}))...>;

        using ReturnSignatures = decltype(removeCancelledOutSignatures(CombinedSignatures{}));
        return ReturnSignatures{};
    };

    return impl(std::make_index_sequence<TagsList::size()>());
}

template <number_helper::SignedNumberConcept _LRep, UnitSignatureConcept... _LSignatures,
          number_helper::SignedNumberConcept _RRep, UnitSignatureConcept... _RSignatures>
consteval auto determineMultiplyReturnType(CompoundUnit<_LRep, _LSignatures...> lhs,
                                           CompoundUnit<_RRep, _RSignatures...> rhs)
{
    using CommonRep = std::common_type_t<_LRep, _RRep>;
    using LSignaturesList = TypeList<_LSignatures...>;
    using RSignaturesList = TypeList<_RSignatures...>;

    using SignaturesList =
        decltype(impl::computeMultiplicationSignatures_impl(LSignaturesList{}, RSignaturesList{}));

    if constexpr (SignaturesList::size() > 0)
    {
        using ReturnType = typelist_helper::make_specialization_t<
            CompoundUnit, typename SignaturesList::template push_front_t<CommonRep>>;
        return ReturnType{};
    }
    else
    {
        return CommonRep{};
    }
}

template <number_helper::SignedNumberConcept _LRep, UnitSignatureConcept... _LSignatures,
          number_helper::SignedNumberConcept _RRep, UnitSignatureConcept... _RSignatures>
consteval auto determineScalingRatio(CompoundUnit<_LRep, _LSignatures...> lhs,
                                     CompoundUnit<_RRep, _RSignatures...> rhs)
{
    using LSignaturesList = TypeList<_LSignatures...>;
    using LTagsList = TypeList<typename _LSignatures::Tag...>;

    using RSignaturesList = TypeList<_RSignatures...>;
    using RTagsList = TypeList<typename _RSignatures::Tag...>;

    // Common tags of two compound units, the duplication has been removed.
    using TagsList = typelist_helper::typelist_union_t<LTagsList, RTagsList>;

    constexpr auto compute_scaling_ratio =
        []<class... Tags>(TypeList<Tags...>) -> number_helper::RatioConcept auto {
        using ScalingRatio =
            number_helper::ratios_multiply_t<decltype(extractScalingRatioContributionByTag(
                Tags{}, LSignaturesList{}, RSignaturesList{}))...>;
        return ScalingRatio{};
    };

    return decltype(compute_scaling_ratio(TagsList{})){};
}

template <CompoundUnitConcept LeftType, CompoundUnitConcept RightType>
consteval CompoundUnitConcept auto determineCommonCompoundUnit(const LeftType& lhs,
                                                               const RightType& rhs)
{
    static_assert(are_compound_units_castable_v<LeftType, RightType>, "The two compound units "
                                                                      "shall be castable to each "
                                                                      "other.");

    using CommonSignatures = std::conditional_t<
        std::ratio_less_equal_v<typename LeftType::Period, typename RightType::Period>,
        typename LeftType::Signatures, typename RightType::Signatures>;

    using CommonRep = std::common_type_t<typename LeftType::Rep, typename RightType::Rep>;
    using ReturnType = typelist_helper::make_specialization_t<
        CompoundUnit, typename CommonSignatures::template push_front_t<CommonRep>>;

    return ReturnType{};
}

} // namespace compound_unit::impl
#endif // _SRC_SRC_COMPOUND_UNIT_IMPL_H_
