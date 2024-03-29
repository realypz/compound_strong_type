#ifndef SRC_SRC_COMPOUND_UNIT_IMPL_H_
#define SRC_SRC_COMPOUND_UNIT_IMPL_H_

#include "src/compound_unit.h"

namespace compound_unit::impl
{
struct _UnitSignature
{
    std::int32_t num;
    std::int32_t den;
    std::int32_t exp;
};

template <class XTag, UnitSignatureConcept... Signatures>
consteval std::optional<_UnitSignature> extractSignatureByTag(XTag, TypeList<Signatures...>)
{
    using SignaturesList = TypeList<Signatures...>;
    using TagsList = TypeList<typename Signatures::Tag...>;

    if constexpr (TagsList::template has_type<XTag>)
    {
        constexpr std::size_t pos{*typelist_helper::pos_of_type_v<TagsList, XTag>};
        constexpr std::int32_t exp{SignaturesList::template type_at<pos>::Exp};
        constexpr std::int32_t num{SignaturesList::template type_at<pos>::Period::num};
        constexpr std::int32_t den{SignaturesList::template type_at<pos>::Period::den};

        return _UnitSignature{.num = num, .den = den, .exp = exp};
    }
    return std::nullopt;
}

template <class XTag, UnitSignatureConcept... LSignatures, UnitSignatureConcept... RSignatures>
consteval _UnitSignature extractCommonSignature(XTag, TypeList<LSignatures...>,
                                                TypeList<RSignatures...>)
{
    constexpr std::optional<_UnitSignature> lhs_signature{
        extractSignatureByTag(XTag{}, TypeList<LSignatures...>{})};
    constexpr std::optional<_UnitSignature> rhs_signature{
        extractSignatureByTag(XTag{}, TypeList<RSignatures...>{})};

    if constexpr (lhs_signature.has_value() && rhs_signature.has_value())
    {
        using LeftRatio = std::ratio<lhs_signature->num, lhs_signature->den>;
        using RightRatio = std::ratio<rhs_signature->num, rhs_signature->den>;
        using CommonRatio = number_helper::ratio_gcd<RightRatio, LeftRatio>::type;
        constexpr _UnitSignature common_signature{CommonRatio::num, CommonRatio::den,
                                                  lhs_signature->exp + rhs_signature->exp};

        return common_signature;
    }
    else if (lhs_signature.has_value() && !rhs_signature.has_value())
    {
        return *lhs_signature;
    }
    else if (!lhs_signature.has_value() && rhs_signature.has_value())
    {
        return *rhs_signature;
    }
    else
    {
        return _UnitSignature{0, 0, 0};
    }
}

template <class XTag, UnitSignatureConcept... LSignatures, UnitSignatureConcept... RSignatures>
consteval auto extractScalingRatioByTag_impl(XTag, TypeList<LSignatures...>,
                                             TypeList<RSignatures...>)
{
    constexpr std::optional<_UnitSignature> lhs_signature{
        extractSignatureByTag(XTag{}, TypeList<LSignatures...>{})};
    constexpr std::optional<_UnitSignature> rhs_signature{
        extractSignatureByTag(XTag{}, TypeList<RSignatures...>{})};

    if constexpr (lhs_signature.has_value() && rhs_signature.has_value())
    {
        using LeftRatio = std::ratio<lhs_signature->num, lhs_signature->den>;
        using RightRatio = std::ratio<rhs_signature->num, rhs_signature->den>;
        using CommonRatio = number_helper::ratio_gcd<RightRatio, LeftRatio>::type;

        using ScalingRatioLeft =
            number_helper::ratio_pow_t<std::ratio_divide<LeftRatio, CommonRatio>,
                                       lhs_signature->exp>;
        using ScalingRatioRight =
            number_helper::ratio_pow_t<std::ratio_divide<RightRatio, CommonRatio>,
                                       rhs_signature->exp>;

        using ScalingRatio = std::ratio_multiply<ScalingRatioLeft, ScalingRatioRight>;

        return ScalingRatio{};
    }
    else
    {
        using ScalingRatio = std::ratio<1, 1>;
        return ScalingRatio{};
    }
}

struct CancelledOutSignature
{};

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

template <UnitSignatureConcept... _LSignatures, UnitSignatureConcept... _RSignatures>
consteval auto computeReturnSignatures_impl(TypeList<_LSignatures...> l_sig_tuple,
                                            TypeList<_RSignatures...> r_sig_tuple)
{
    using LTagsList = TypeList<typename _LSignatures::Tag...>;
    using RTagsList = TypeList<typename _RSignatures::Tag...>;
    using TagsList = typelist_helper::typelist_union_t<LTagsList, RTagsList>;

    constexpr auto impl = []<std::size_t... Is>(std::index_sequence<Is...>) -> auto {
        // Return a UnitSignature or CancelledOutSignature.
        constexpr auto _extract_unit_signature = []<class Tag>(Tag) -> auto {
            constexpr _UnitSignature common_signature{impl::extractCommonSignature(
                Tag{}, TypeList<_LSignatures...>{}, TypeList<_RSignatures...>{})};

            if constexpr (common_signature.exp != 0)
            {
                static_assert(common_signature.num != 0 && common_signature.den != 0,
                              "The common signature shall not have zero numerator or "
                              "denominator.");
                using RetType =
                    UnitSignature<std::ratio<common_signature.num, common_signature.den>,
                                  common_signature.exp, Tag>;

                return RetType{};
            }
            else
            {
                return CancelledOutSignature{};
            }
        };

        using CombinedSignatures = TypeList<decltype(_extract_unit_signature(
            typename TagsList::template type_at<Is>{}))...>;

        using ReturnSignatures = decltype(removeCancelledOutSignatures(CombinedSignatures{}));

        return ReturnSignatures{};
    };

    return impl(std::make_index_sequence<TagsList::size()>());
}

template <number_helper::SignedNumberConcept _LRep,
          UnitSignatureConcept... _LSignatures, // left
          number_helper::SignedNumberConcept _RRep,
          UnitSignatureConcept... _RSignatures // right
          >
consteval auto determineReturnType(CompoundUnit<_LRep, _LSignatures...> lhs,
                                   CompoundUnit<_RRep, _RSignatures...> rhs)
{
    using CommonRep = std::common_type_t<_LRep, _RRep>;
    using LSignaturesList = TypeList<_LSignatures...>;
    using RSignaturesList = TypeList<_RSignatures...>;

    using SignaturesList =
        decltype(impl::computeReturnSignatures_impl(LSignaturesList{}, RSignaturesList{}));

    if constexpr (SignaturesList::size() > 0)
    {
        using ReturnType = make_compound_unit<CommonRep, SignaturesList>::type;
        return ReturnType{};
    }
    else
    {
        return CommonRep{};
    }
}

template <number_helper::SignedNumberConcept _LRep,
          UnitSignatureConcept... _LSignatures, // left
          number_helper::SignedNumberConcept _RRep,
          UnitSignatureConcept... _RSignatures // right
          >
consteval auto determineScalingRatio(CompoundUnit<_LRep, _LSignatures...> lhs,
                                     CompoundUnit<_RRep, _RSignatures...> rhs)
{
    using LSignaturesList = TypeList<_LSignatures...>;
    using LTagsList = TypeList<typename _LSignatures::Tag...>;

    using RSignaturesList = TypeList<_RSignatures...>;
    using RTagsList = TypeList<typename _RSignatures::Tag...>;

    // Common tags of two compound units, the duplication has been removed.
    using TagsList = typelist_helper::typelist_union_t<LTagsList, RTagsList>;

    constexpr auto compute_scaling_ratio = []<class... Tags>(TypeList<Tags...>) -> auto {
        using ScalingRatio =
            number_helper::ratios_multiply_t<decltype(impl::extractScalingRatioByTag_impl(
                Tags{}, LSignaturesList{}, RSignaturesList{}))...>;
        return ScalingRatio{};
    };

    return decltype(compute_scaling_ratio(TagsList{})){};
}

} // namespace compound_unit::impl
#endif // SRC_SRC_COMPOUND_UNIT_IMPL_H_
