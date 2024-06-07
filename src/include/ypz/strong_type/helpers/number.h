#ifndef SRC_INCLUDE_YPZ_STRONG_TYPE_HELPERS_NUMBER_H_
#define SRC_INCLUDE_YPZ_STRONG_TYPE_HELPERS_NUMBER_H_

#include <numeric>
#include <ratio>

namespace cpu::number_helper
{
template <class _Tp>
struct is_std_ratio : std::false_type
{};

template <intmax_t _Num, intmax_t _Den>
struct is_std_ratio<std::ratio<_Num, _Den>> : std::true_type
{};

template <typename T>
concept RatioConcept = is_std_ratio<T>::value;

template <typename T>
concept SignedNumberConcept = std::signed_integral<T> || std::floating_point<T>;

template <typename T>
concept NumberConcept = std::integral<T> || std::floating_point<T>;

template <std::int32_t base, std::int32_t exp>
requires(exp >= 0)
constexpr std::int32_t intPow()
{
    if constexpr (exp == 0)
    {
        return 1;
    }
    else
    {
        return base * intPow<base, exp - 1>();
    }
}

/// Greatest common denominator of two ratios.
template <RatioConcept _R1, RatioConcept _R2>
struct ratio_gcd
{
    typedef std::ratio<std::gcd(_R1::num, _R2::num), std::lcm(_R1::den, _R2::den)> type;
};

/// The power of ratio.
///@{
template <RatioConcept R, std::int32_t pow>
struct ratio_pow;

template <RatioConcept R, std::int32_t pow>
requires(pow >= 0)
struct ratio_pow<R, pow>
{
    using type = std::ratio<intPow<R::num, pow>(), intPow<R::den, pow>()>;
};

template <RatioConcept R, std::int32_t pow>
requires(pow < 0)
struct ratio_pow<R, pow>
{
    using type = std::ratio<intPow<R::den, -pow>(), intPow<R::num, -pow>()>;
};

template <RatioConcept R, std::int32_t pow>
using ratio_pow_t = ratio_pow<R, pow>::type;
///@

/// The multiplication of ratios.
///@{
template <RatioConcept... R>
requires(sizeof...(R) > 0)
struct ratios_multiply;

template <RatioConcept Rx, RatioConcept Ry, RatioConcept... Rs>
struct ratios_multiply<Rx, Ry, Rs...> : ratios_multiply<std::ratio_multiply<Rx, Ry>, Rs...>
{};

template <RatioConcept Rx>
struct ratios_multiply<Rx>
{
    using type = Rx;
};

template <RatioConcept... R>
using ratios_multiply_t = ratios_multiply<R...>::type;

///@}

} // namespace cpu::number_helper

#endif // SRC_INCLUDE_YPZ_STRONG_TYPE_HELPERS_NUMBER_H_
