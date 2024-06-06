#ifndef SRC_COMPOUND_UNIT_H_
#define SRC_COMPOUND_UNIT_H_

#include <compare>
#include <cstdint>
#include <ratio>

#include "src/helpers/number.h"
#include "src/helpers/type.h"
#include "src/signature.h"

namespace compound_unit
{
/**
 * Compound Unit
 * @details A compound unit consists of several one or several unit signatures
 *          with their respective exponents.
 * @tparam _Rep the underlying representation type. Must be a signed number.
 * @tparam _Signatures the unit signatures.
 * @pre     The number of signatures must be greater than 0.
 * @pre     The tags of each signature must be unique.
 */
template <number_helper::SignedNumberConcept _Rep, UnitSignatureConcept... _Signatures>
class CompoundUnit
{
    static_assert(sizeof...(_Signatures) > 0);
    static_assert(
        type_helper::is_each_type_unique<type_helper::TypeList<typename _Signatures::Tag...>>);

  public:
    /// @brief The underlying representation type.
    using Rep = _Rep;

    /// @brief The unit signatures.
    using Signatures = type_helper::TypeList<_Signatures...>;

    /// @brief The period of the compound unit.
    using Period = number_helper::ratios_multiply_t<
        typename number_helper::ratio_pow_t<typename _Signatures::Period, _Signatures::Exp>...>;

    /// @brief Constructors.
    ///@{
    /// @brief Default constructor.
    explicit constexpr CompoundUnit() : count_{0} {};

    /// @brief Construct from count.
    explicit constexpr CompoundUnit(const _Rep count) : count_{count} {}

    /// @brief Construct from another castable compound unit.
    template <number_helper::SignedNumberConcept _XRep, UnitSignatureConcept... _XSignatures>
    constexpr CompoundUnit(const CompoundUnit<_XRep, _XSignatures...>& from);
    ///@}

    /// @brief Operator<=>
    constexpr std::partial_ordering operator<=>(const CompoundUnit&) const = default;

    /// @brief Get the count of the underlying data.
    constexpr _Rep count() const { return count_; }

  private:
    _Rep count_;
};

/// Concept for CompoundUnit.
template <class T>
concept CompoundUnitConcept = type_helper::is_specialization_v<T, CompoundUnit>;

/// Type to get the multiplication result of two compound unit types.
template <CompoundUnitConcept L, CompoundUnitConcept R>
using MultiplyUnit = decltype(L{} * R{});

/// Type to get the division result of two compound unit types.
template <CompoundUnitConcept L, CompoundUnitConcept R>
using DivideUnit = decltype(L{} / R{1});

namespace compound_unit_helper
{
/**
 * Helper boolean to determine whether two compound units are castable or not.
 * @tparam T one compound unit specialization
 * @tparam U another compound unit specialization
 * @details When all of the following conditions are met, T and U are castable:
 *              * T and U has the same set of (Tag, Exp) pairs.
 */
template <CompoundUnitConcept T, CompoundUnitConcept U>
constexpr bool are_compound_units_castable_v{
    []<number_helper::SignedNumberConcept TRep, UnitSignatureConcept... TSignatures,
       number_helper::SignedNumberConcept URep, UnitSignatureConcept... USignatures>(
        CompoundUnit<TRep, TSignatures...>, CompoundUnit<URep, USignatures...>) -> bool {
        constexpr bool size_equal{sizeof...(TSignatures) == sizeof...(USignatures)};
        constexpr bool signatures_equal{type_helper::are_typelists_interchangeable_v<
            type_helper::TypeList<
                UnitSignature<std::ratio<1, 1>, TSignatures::Exp, typename TSignatures::Tag>...>,
            type_helper::TypeList<
                UnitSignature<std::ratio<1, 1>, USignatures::Exp, typename USignatures::Tag>...>>};
        return size_equal && signatures_equal;
    }(T{}, U{})};

/**
 * Cast a compound unit to another compound unit.
 * @tparam TargetType the target compound unit specialization.
 * @tparam _FromRep the underlying representation type of the source compound
 * unit.
 * @tparam _FromSignatures the unit signatures of the source compound unit.
 * @param source the source compound unit.
 * @return the converted compound unit.
 */
template <CompoundUnitConcept TargetType, number_helper::SignedNumberConcept _FromRep,
          UnitSignatureConcept... _FromSignatures>
requires(compound_unit_helper::are_compound_units_castable_v<
         TargetType, CompoundUnit<_FromRep, _FromSignatures...>>)
constexpr TargetType castAs(const CompoundUnit<_FromRep, _FromSignatures...>& source)
{
    using FromType = CompoundUnit<_FromRep, _FromSignatures...>;
    using CommonRep = std::common_type_t<_FromRep, typename TargetType::Rep>;

    using ScalingRatio = std::ratio_divide<typename TargetType::Period, typename FromType::Period>;

    return TargetType(static_cast<CommonRep>(source.count()) *
                      static_cast<CommonRep>(ScalingRatio::den) /
                      static_cast<CommonRep>(ScalingRatio::num));
}

/**
 * Helper boolean to determine whether two compound units are equal or not.
 * @tparam T one compound unit specialization
 * @tparam U another compound unit specialization
 * @details When all of the following conditions are met, T and U are equal:
 *          1. T and U are convertibe with each other.
 *          2. T and U has the same underlying Rep type and T::Rep == U::Rep.
 *          3. T and U has the same period and T::Period == U::Period.
 * @attention Equality of two compound units does NOT require their signatures to
 *            be in the same order, or the period of each signature to be the same.
 *            E.g. m/s is equal to mm/ms.
 */
template <CompoundUnitConcept T, CompoundUnitConcept U>
constexpr bool are_compound_unit_equal_v{[]() {
    constexpr bool is_castable{are_compound_units_castable_v<T, U> &&
                               are_compound_units_castable_v<U, T>};
    constexpr bool is_ratio_equal{std::ratio_equal_v<typename T::Period, typename U::Period>};
    return is_castable && std::same_as<typename T::Rep, typename U::Rep> && is_ratio_equal;
}()};

template <number_helper::SignedNumberConcept _LRep, UnitSignatureConcept... _LSignatures,
          number_helper::SignedNumberConcept _RRep, UnitSignatureConcept... _RSignatures>
consteval auto determineMultiplyReturnType(CompoundUnit<_LRep, _LSignatures...> lhs,
                                           CompoundUnit<_RRep, _RSignatures...> rhs)
{
    using CommonRep = std::common_type_t<_LRep, _RRep>;
    using LSignaturesList = type_helper::TypeList<_LSignatures...>;
    using RSignaturesList = type_helper::TypeList<_RSignatures...>;

    using SignaturesList = decltype(signature_helper::computeMultiplicationSignatures_impl(
        LSignaturesList{}, RSignaturesList{}));

    if constexpr (SignaturesList::size() > 0)
    {
        using ReturnType = type_helper::make_specialization_t<
            CompoundUnit, typename SignaturesList::template push_front_t<CommonRep>>;
        return ReturnType{};
    }
    else
    {
        return CommonRep{};
    }
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
    using ReturnType = type_helper::make_specialization_t<
        CompoundUnit, typename CommonSignatures::template push_front_t<CommonRep>>;

    return ReturnType{};
}

template <number_helper::SignedNumberConcept _LRep, UnitSignatureConcept... _LSignatures,
          number_helper::SignedNumberConcept _RRep, UnitSignatureConcept... _RSignatures>
consteval auto determineScalingRatio(CompoundUnit<_LRep, _LSignatures...> lhs,
                                     CompoundUnit<_RRep, _RSignatures...> rhs)
{
    using LSignaturesList = type_helper::TypeList<_LSignatures...>;
    using LTagsList = type_helper::TypeList<typename _LSignatures::Tag...>;

    using RSignaturesList = type_helper::TypeList<_RSignatures...>;
    using RTagsList = type_helper::TypeList<typename _RSignatures::Tag...>;

    // Common tags of two compound units, the duplication has been removed.
    using TagsList = type_helper::typelist_union_t<LTagsList, RTagsList>;

    constexpr auto compute_scaling_ratio =
        []<class... Tags>(type_helper::TypeList<Tags...>) -> number_helper::RatioConcept auto {
        using ScalingRatio = number_helper::ratios_multiply_t<
            decltype(signature_helper::extractScalingRatioContributionByTag(
                Tags{}, LSignaturesList{}, RSignaturesList{}))...>;
        return ScalingRatio{};
    };

    return decltype(compute_scaling_ratio(TagsList{})){};
}

} // namespace compound_unit_helper

template <number_helper::SignedNumberConcept _Rep, UnitSignatureConcept... _Signatures>
template <number_helper::SignedNumberConcept _XRep, UnitSignatureConcept... _XSignatures>
constexpr CompoundUnit<_Rep, _Signatures...>::CompoundUnit(
    const CompoundUnit<_XRep, _XSignatures...>& from)
    : CompoundUnit{compound_unit_helper::castAs<CompoundUnit<_Rep, _Signatures...>>(from)}
{}

/// Operator* overloads for CompoundUnit.
///@{
/**
 * Multiply two compound units.
 * @param lhs the left compound unit.
 * @param rhs the right compound unit.
 * @return the result of the multiplication. Can be a compound unit or a number.
 *         The underlying Rep is the common Rep of lhs and rhs.
 */
template <number_helper::SignedNumberConcept _LRep, UnitSignatureConcept... _LSignatures,
          number_helper::SignedNumberConcept _RRep, UnitSignatureConcept... _RSignatures>
constexpr auto operator*(const CompoundUnit<_LRep, _LSignatures...>& lhs,
                         const CompoundUnit<_RRep, _RSignatures...>& rhs)
{
    using ReturnType = decltype(compound_unit_helper::determineMultiplyReturnType(lhs, rhs));
    using ScalingRatio = decltype(compound_unit_helper::determineScalingRatio(lhs, rhs));

    if constexpr (CompoundUnitConcept<ReturnType>)
    {
        return ReturnType(static_cast<ReturnType::Rep>(lhs.count()) *
                          static_cast<ReturnType::Rep>(rhs.count()) * ScalingRatio::num /
                          ScalingRatio::den);
    }
    else
    {
        return ReturnType(static_cast<ReturnType>(lhs.count()) *
                          static_cast<ReturnType>(rhs.count()) * ScalingRatio::num /
                          ScalingRatio::den);
    }
}

/// Multiply a compound unit with a number.
template <number_helper::SignedNumberConcept _LRep, UnitSignatureConcept... _LSignatures,
          number_helper::SignedNumberConcept _Rhs>
constexpr auto operator*(const CompoundUnit<_LRep, _LSignatures...>& lhs, const _Rhs rhs)
{
    using CommonRep = std::common_type_t<_LRep, _Rhs>;
    using ReturnType = CompoundUnit<CommonRep, _LSignatures...>;
    return ReturnType{static_cast<CommonRep>(lhs.count()) * static_cast<CommonRep>(rhs)};
}

/// Multiply a number with a compound unit.
template <number_helper::SignedNumberConcept _Lhs, number_helper::SignedNumberConcept _RRep,
          UnitSignatureConcept... _RSignatures>
constexpr auto operator*(const _Lhs lhs, const CompoundUnit<_RRep, _RSignatures...>& rhs)
{
    return rhs * lhs;
}

///@}

/// Operator/ overloads for CompoundUnit.
///@{
/**
 * Divide two compound units.
 * @param lhs the left compound unit.
 * @param rhs the right compound unit.
 * @return the result of the division. Can be a compound unit or a number.
 *         The underlying Rep is the common Rep of lhs and rhs.
 */
template <number_helper::SignedNumberConcept _LRep, UnitSignatureConcept... _LSignatures,
          number_helper::SignedNumberConcept _RRep, UnitSignatureConcept... _RSignatures>
constexpr auto operator/(const CompoundUnit<_LRep, _LSignatures...>& lhs,
                         const CompoundUnit<_RRep, _RSignatures...>& rhs)
{
    using RInverseCompoundUnit = CompoundUnit<_RRep, inverse_signature_t<_RSignatures>...>;

    using ReturnType =
        decltype(compound_unit_helper::determineMultiplyReturnType(lhs, RInverseCompoundUnit{}));
    using ScalingRatio =
        decltype(compound_unit_helper::determineScalingRatio(lhs, RInverseCompoundUnit{}));

    if constexpr (CompoundUnitConcept<ReturnType>)
    {
        return ReturnType(static_cast<ReturnType::Rep>(lhs.count()) * ScalingRatio::num /
                          static_cast<ReturnType::Rep>(rhs.count()) / ScalingRatio::den);
    }
    else
    {
        return ReturnType(static_cast<ReturnType>(lhs.count()) * ScalingRatio::num /
                          static_cast<ReturnType>(rhs.count()) / ScalingRatio::den);
    }
}

/// Divide a compound unit by a number.
template <number_helper::SignedNumberConcept _LRep, UnitSignatureConcept... _LSignatures,
          number_helper::SignedNumberConcept _Rhs>
constexpr auto operator/(const CompoundUnit<_LRep, _LSignatures...>& lhs, const _Rhs rhs)
{
    using CommonRep = std::common_type_t<_LRep, _Rhs>;
    using ReturnType = CompoundUnit<CommonRep, _LSignatures...>;
    return ReturnType{static_cast<CommonRep>(lhs.count()) / static_cast<CommonRep>(rhs)};
}

///@}

/// Unary Operator- overloads for CompoundUnit.
template <number_helper::SignedNumberConcept _Rep, UnitSignatureConcept... _Signatures>
constexpr auto operator-(const CompoundUnit<_Rep, _Signatures...>& operand)
{
    return CompoundUnit<_Rep, _Signatures...>{-operand.count()};
}

/// Operator+ overloads for CompoundUnit.
template <number_helper::SignedNumberConcept _LRep, UnitSignatureConcept... _LSignatures,
          number_helper::SignedNumberConcept _RRep, UnitSignatureConcept... _RSignatures>
requires(compound_unit_helper::are_compound_units_castable_v<CompoundUnit<_LRep, _LSignatures...>,
                                                             CompoundUnit<_RRep, _RSignatures...>>)
constexpr auto operator+(const CompoundUnit<_LRep, _LSignatures...>& lhs,
                         const CompoundUnit<_RRep, _RSignatures...>& rhs)
{
    using ReturnType = decltype(compound_unit_helper::determineCommonCompoundUnit(lhs, rhs));

    return ReturnType{static_cast<ReturnType>(lhs).count() + static_cast<ReturnType>(rhs).count()};
}

/// Operator- overloads for CompoundUnit.
template <number_helper::SignedNumberConcept _LRep, UnitSignatureConcept... _LSignatures,
          number_helper::SignedNumberConcept _RRep, UnitSignatureConcept... _RSignatures>
requires(compound_unit_helper::are_compound_units_castable_v<CompoundUnit<_LRep, _LSignatures...>,
                                                             CompoundUnit<_RRep, _RSignatures...>>)
constexpr auto operator-(const CompoundUnit<_LRep, _LSignatures...>& lhs,
                         const CompoundUnit<_RRep, _RSignatures...>& rhs)
{
    return lhs + (-rhs);
}

/**
 * Operator<=> overloads for CompoundUnit.
 * @pre The two operands must be castable with each other.
 * @details Given possibility of NAN in the computation result, the return type is
 *          std::partial_ordering.
 */
template <CompoundUnitConcept LeftType, CompoundUnitConcept RightType>
requires(compound_unit_helper::are_compound_units_castable_v<LeftType, RightType>)
constexpr std::partial_ordering operator<=>(const LeftType& lhs, const RightType& rhs)
{

    using CommonType = decltype(compound_unit_helper::determineCommonCompoundUnit(lhs, rhs));
    static_assert(CompoundUnitConcept<CommonType>,
                  "The return type of the comparison must be a compound unit.");
    return static_cast<CommonType>(lhs) <=> static_cast<CommonType>(rhs);
}

} // namespace compound_unit

#endif // SRC_COMPOUND_UNIT_H_
