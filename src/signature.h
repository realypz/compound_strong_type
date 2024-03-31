#ifndef SRC_SIGNATURE_H_
#define SRC_SIGNATURE_H_

#include "helpers/number_helper.h"

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

} // namespace compound_unit
#endif // SRC_SIGNATURE_H_
