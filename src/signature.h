#ifndef SRC_SIGNATURE_H_
#define SRC_SIGNATURE_H_

#include "helpers/number_helper.h"

namespace compound_unit
{
/**
 * Unit signature denotes a physical unit's property, numerical scale and its
 * dimension (or exponent).
 * @tparam _Period the numerical scale of the one-dimentional unit represented
 * by _Tag.
 * @tparam _Exp the exponent of this unit, e.g. for qubic meter (where tag is
 * length), _Exp == 3.
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

template <class T>
concept UnitSignatureConcept = is_unit_signature<T>::value;

template <UnitSignatureConcept T>
using inverse_signature = UnitSignature<typename T::Period, -T::Exp, typename T::Tag>;

namespace signature_helper
{
/**
 * The joint period of one or more unit signatures.
 * @warning The tag of each signature shall be unique. Otherwise the behaviour
 * is undefined.
 */
template <UnitSignatureConcept... USignatures>
requires(sizeof...(USignatures) > 0)
using joint_period_t = number_helper::ratios_multiply_t<
    typename number_helper::ratio_pow_t<typename USignatures::Period, USignatures::Exp>...>;

} // namespace signature_helper

} // namespace compound_unit
#endif // SRC_SIGNATURE_H_
