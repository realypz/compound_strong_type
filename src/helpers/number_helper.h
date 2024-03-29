#ifndef SRC_HELPERS_NUMBER_HELPER_H_
#define SRC_HELPERS_NUMBER_HELPER_H_

#include <numeric>
#include <ratio>

namespace compound_unit::number_helper
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
    // return exp == 0 ? 1 : base * intPow<base, exp - 1>();
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

template <RatioConcept Ratio, std::int32_t Exp, RatioConcept InitRatio = std::ratio<1, 1>>
struct joint_ratio
{
  private:
    struct NumDenPair
    {
        std::int32_t num;
        std::int32_t den;
    };

    static constexpr NumDenPair impl()
    {
        std::int32_t num{InitRatio::num};
        std::int32_t den{InitRatio::den};

        constexpr std::int32_t exp_abs{Exp >= 0 ? Exp : -Exp};
        // NOTE: std::abs() can be used as constexpr func since C++23.

        for (std::int32_t loop{}; loop < exp_abs; ++loop)
        {
            if constexpr (Exp > 0)
            {
                num *= Ratio::num;
                den *= Ratio::den;
            }
            else if (Exp < 0)
            {
                num *= Ratio::den;
                den *= Ratio::num;
            }
        }
        return NumDenPair{num, den};
    };

  public:
    using type = std::ratio<impl().num, impl().den>;
};

} // namespace compound_unit::number_helper

#endif // SRC_HELPERS_NUMBER_HELPER_H_
