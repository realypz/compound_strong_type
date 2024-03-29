#ifndef SRC_COMPOUND_UNIT_H_
#define SRC_COMPOUND_UNIT_H_

#include <cstdint>
#include <ratio>

#include "src/helpers/number_helper.h"
#include "src/helpers/type.h"
#include "src/helpers/typelist.h"
#include "src/signature.h"

namespace compound_unit
{
/**
 * Compound Unit
 * @details A compound unit consists of several one or several unit signatures
 * with their respective exponents.
 * @tparam _Rep the underlying representation type. Must be a signed number.
 * @tparam _Signatures the unit signatures.
 */
template <number_helper::SignedNumberConcept _Rep, UnitSignatureConcept... _Signatures>
requires(sizeof...(_Signatures) > 0 &&
         typelist_helper::is_each_type_unique<TypeList<typename _Signatures::Tag...>>)
class CompoundUnit
{
  public:
    /// @brief The underlying representation type.
    using Rep = _Rep;

    /// @brief The unit signatures.
    using Signatures = TypeList<_Signatures...>;

    /// @brief The period of the compound unit.
    using Period = signature_helper::joint_period_t<_Signatures...>;

    /// @brief Constructors.
    ///@{
    /// @brief Default constructor.
    constexpr CompoundUnit() : count_{0} {};

    /// @brief Construct from count.
    /// @param count
    explicit constexpr CompoundUnit(const _Rep count) : count_{count}
    {}

    /// @brief Construct from another convertable compound unit.
    template <number_helper::SignedNumberConcept _XRep, UnitSignatureConcept... _XSignatures>
    explicit constexpr CompoundUnit(const CompoundUnit<_XRep, _XSignatures...>& from)
        : CompoundUnit{compound_unit_cast<CompoundUnit<_Rep, _Signatures...>>(from)}
    {}

    ///@}

    /// @brief Get the count of the underlying data.
    constexpr _Rep count() const
    {
        return count_;
    }

  private:
    _Rep count_;
};

/// Concept for CompoundUnit.
template <class T>
concept CompoundUnitConcept = type_helper::is_specialization_v<T, CompoundUnit>;

template <number_helper::SignedNumberConcept TRep, UnitSignatureConcept... TSignatures,
          number_helper::SignedNumberConcept URep, UnitSignatureConcept... USignatures>
consteval bool are_compound_unit_convertable_impl(CompoundUnit<TRep, TSignatures...>,
                                                  CompoundUnit<URep, USignatures...>)
{
    if constexpr (sizeof...(TSignatures) != sizeof...(USignatures))
    {
        return false;
    }

    using DefaultRatio = std::ratio<1, 1>;

    return typelist_helper::are_typelists_interchangeable_v<
        TypeList<UnitSignature<DefaultRatio, TSignatures::Exp, typename TSignatures::Tag>...>,
        TypeList<UnitSignature<DefaultRatio, USignatures::Exp, typename USignatures::Tag>...>>;
}

/**
 * Helper boolean to determine whether two compound units are convertable or not.
 * @tparam T one compound unit specialization
 * @tparam U another compound unit specialization
 * @details When all of the following conditions are met, T and U are
 * convertable:
 *          * T and U has the same set of signatures.
 */
template <CompoundUnitConcept T, CompoundUnitConcept U>
constexpr bool are_compound_unit_convertable_v{are_compound_unit_convertable_impl(T{}, U{})};

/**
 * Helper boolean to determine whether two compound units are equal or not.
 * @tparam T one compound unit specialization
 * @tparam U another compound unit specialization
 * @details When all of the following conditions are met, T and U are equal:
 *          1. T and U are convertibe with each other.
 *          2. T and U has the same underlying Rep type and T::Period ==
 * U::Period.
 * @attention Equality of two compound units does NOT require their signatures to
 * be in the same order, or the period of each signature to be the same.
 */
template <CompoundUnitConcept T, CompoundUnitConcept U>
constexpr bool are_compound_unit_equal_v{[]() {
    if constexpr (!(are_compound_unit_convertable_v<T, U> && are_compound_unit_convertable_v<U, T>))
    {
        return false;
    }

    return (std::same_as<typename T::Rep, typename U ::Rep> &&
            std::ratio_equal_v<typename T::Period, typename U::Period>);
}()};

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
requires(are_compound_unit_convertable_v<TargetType, CompoundUnit<_FromRep, _FromSignatures...>>)
constexpr TargetType compound_unit_cast(const CompoundUnit<_FromRep, _FromSignatures...>& source)
{
    using SourceType = CompoundUnit<_FromRep, _FromSignatures...>;
    using CommonRep = std::common_type_t<_FromRep, typename TargetType::Rep>;

    using ScalingRatio =
        std::ratio_divide<typename TargetType::Period, typename SourceType::Period>;

    return TargetType(static_cast<CommonRep>(source.count()) *
                      static_cast<CommonRep>(ScalingRatio::den) /
                      static_cast<CommonRep>(ScalingRatio::num));
}

namespace impl
{
template <number_helper::SignedNumberConcept _LRep,
          UnitSignatureConcept... _LSignatures, // left
          number_helper::SignedNumberConcept _RRep,
          UnitSignatureConcept... _RSignatures // right
          >
consteval auto determineReturnType(CompoundUnit<_LRep, _LSignatures...> lhs,
                                   CompoundUnit<_RRep, _RSignatures...> rhs);

template <number_helper::SignedNumberConcept _LRep,
          UnitSignatureConcept... _LSignatures, // left
          number_helper::SignedNumberConcept _RRep,
          UnitSignatureConcept... _RSignatures // right
          >
consteval auto determineScalingRatio(CompoundUnit<_LRep, _LSignatures...> lhs,
                                     CompoundUnit<_RRep, _RSignatures...> rhs);

template <number_helper::SignedNumberConcept _Rep, class T>
struct make_compound_unit;

template <number_helper::SignedNumberConcept _Rep, UnitSignatureConcept... _Signatures>
struct make_compound_unit<_Rep, TypeList<_Signatures...>>
{
    using type = CompoundUnit<_Rep, _Signatures...>;
};
} // namespace impl

/// Operator* overloads for CompoundUnit.
///@{
/**
 * Multiply two compound units.
 * @param lhs the left compound unit.
 * @param rhs the right compound unit.
 * @return the result of the multiplication. Can be a compound unit or a number.
 *         The underlying Rep is the common Rep of lhs and rhs.
 */
template <number_helper::SignedNumberConcept _LRep,
          UnitSignatureConcept... _LSignatures, // left
          number_helper::SignedNumberConcept _RRep,
          UnitSignatureConcept... _RSignatures // right
          >
constexpr auto operator*(const CompoundUnit<_LRep, _LSignatures...>& lhs,
                         const CompoundUnit<_RRep, _RSignatures...>& rhs)
{
    using ReturnType = decltype(impl::determineReturnType(lhs, rhs));
    using ScalingRatio = decltype(impl::determineScalingRatio(lhs, rhs));

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
template <number_helper::SignedNumberConcept _LRep,
          UnitSignatureConcept... _LSignatures, // left
          number_helper::SignedNumberConcept _Rhs // right
          >
constexpr auto operator*(const CompoundUnit<_LRep, _LSignatures...>& lhs, const _Rhs rhs)
{
    using CommonRep = std::common_type_t<_LRep, _Rhs>;
    using ReturnType = CompoundUnit<CommonRep, _LSignatures...>;
    return ReturnType{static_cast<CommonRep>(lhs.count()) * static_cast<CommonRep>(rhs)};
}

/// Multiply a number with a compound unit.
template <number_helper::SignedNumberConcept _Lhs, // left
          number_helper::SignedNumberConcept _RRep, // right
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
template <number_helper::SignedNumberConcept _LRep,
          UnitSignatureConcept... _LSignatures, // left
          number_helper::SignedNumberConcept _RRep,
          UnitSignatureConcept... _RSignatures // right
          >
constexpr auto operator/(const CompoundUnit<_LRep, _LSignatures...>& lhs,
                         const CompoundUnit<_RRep, _RSignatures...>& rhs)
{
    using RInverseCompoundUnit = CompoundUnit<_RRep, inverse_signature<_RSignatures>...>;

    using ReturnType = decltype(impl::determineReturnType(lhs, RInverseCompoundUnit{}));
    using ScalingRatio = decltype(impl::determineScalingRatio(lhs, RInverseCompoundUnit{}));

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
template <number_helper::SignedNumberConcept _LRep,
          UnitSignatureConcept... _LSignatures, // left
          number_helper::SignedNumberConcept _Rhs // right
          >
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
template <number_helper::SignedNumberConcept _LRep,
          UnitSignatureConcept... _LSignatures, // left
          number_helper::SignedNumberConcept _RRep,
          UnitSignatureConcept... _RSignatures // right
          >
requires(are_compound_unit_convertable_v<CompoundUnit<_LRep, _LSignatures...>,
                                         CompoundUnit<_RRep, _RSignatures...>>)
constexpr auto operator+(const CompoundUnit<_LRep, _LSignatures...>& lhs,
                         const CompoundUnit<_RRep, _RSignatures...>& rhs)
{
    using LeftType = CompoundUnit<_LRep, _LSignatures...>;
    using RightType = CompoundUnit<_RRep, _RSignatures...>;

    using CommonSignatures = std::conditional_t<
        std::ratio_less_equal_v<typename LeftType::Period, typename RightType::Period>,
        typename LeftType::Signatures, typename RightType::Signatures>;

    using CommonRep = std::common_type_t<_LRep, _RRep>;
    using ReturnType = impl::make_compound_unit<CommonRep, CommonSignatures>::type;

    return ReturnType{compound_unit_cast<ReturnType>(lhs).count() +
                      compound_unit_cast<ReturnType>(rhs).count()};
}

/// Operator- overloads for CompoundUnit.
template <number_helper::SignedNumberConcept _LRep,
          UnitSignatureConcept... _LSignatures, // left
          number_helper::SignedNumberConcept _RRep,
          UnitSignatureConcept... _RSignatures // right
          >
requires(are_compound_unit_convertable_v<CompoundUnit<_LRep, _LSignatures...>,
                                         CompoundUnit<_RRep, _RSignatures...>>)
constexpr auto operator-(const CompoundUnit<_LRep, _LSignatures...>& lhs,
                         const CompoundUnit<_RRep, _RSignatures...>& rhs)
{
    return lhs + (-rhs);
}

} // namespace compound_unit

#include "src/compound_unit_impl.h"

#endif // SRC_COMPOUND_UNIT_H_
