#include <gtest/gtest.h>

#include "compound_unit_examples.h"
#include "src/compound_unit.h"
#include "src/signature.h"

namespace compound_unit
{

TEST(how_to_use, operator_plus_minus_multiply_divide)
{
    { // A correct example of compound expression.
        // A double number determines that the intermediate compuation result will
        // not become zero.
        constexpr MeterPerSecond v0{10};
        constexpr Minute t{1};
        constexpr MeterPerSecondSquare a{1};

        constexpr auto ret = v0 * t + 0.5 * a * t * t;
        using RetType = std::remove_cv_t<decltype(ret)>;

        EXPECT_DOUBLE_EQ(ret.count(), 10 * 60.0 + 0.5 * 1 * 60 * 60);
        EXPECT_TRUE((are_compound_unit_equal_v<RetType, Meter_double>));
    }

    { // An incorrect example of compound expression.
        // a / 2 => 0
        // Recommend using double type to keep intermediate computation results.
        constexpr MeterPerSecond v0{10};
        constexpr Minute t{1};
        constexpr MeterPerSecondSquare a{1};

        constexpr auto ret = v0 * t + a / 2 * t * t;
        using RetType = std::remove_cv_t<decltype(ret)>;

        EXPECT_EQ(ret.count(), 10 * 60 + 0);
        EXPECT_TRUE((are_compound_unit_equal_v<RetType, Meter>));
    }

    { // A correct example of compound expression.
        // a / 2.0 => non zero value
        // Explicitly specifying the return type will cast the computation result from double to
        // std::int64_t.
        constexpr MeterPerSecond v0{10};
        constexpr Minute t{1};
        constexpr MeterPerSecondSquare a{1};

        constexpr Meter ret{v0 * t + a / 2.0 * t * t};
        EXPECT_EQ(ret.count(), 2400); // 10 * 60.0 + 1 / 2.0 * 60 * 60
    }

    {
        // Compute area with all integers
        // The integer computation results from operator/ will be truncated,
        // i.e. Meter{17} / 2 => Meter{8}.
        //
        // The auto keyword will be deduced to the signature with the smallest period type.
        // i.e. The expression involves two different periods of length tag, 1:100 from
        // CentiMeter(_double) and 1:1 from Meter, thus the length tag in the return type will have
        // a period of 1:100. This is why the return type is SquareCentiMeter_double.
        constexpr auto ret = Meter{17} / 2 * Meter{8} + CentiMeter{85} * CentiMeter_double{1.9};
        using RetType = std::remove_cv_t<decltype(ret)>;

        EXPECT_DOUBLE_EQ(ret.count(), 8 * 8 * 10000.0 + 85 * 1.9);
        EXPECT_TRUE((are_compound_unit_equal_v<RetType, SquareCentiMeter_double>));
    }

    {
        // Compute area with double
        // The double operands for operator/ will not be truncated,
        // i.e. Meter{17} / 2.0 => Meter_double{8.5}.
        //
        // The return type is explicitly specified as SquareMeter.
        constexpr SquareMeter ret =
            Meter{17} / 2.0 * Meter{8} - CentiMeter{85} * CentiMeter_double{1.9};
        constexpr double count_before_cast{(8.5 * 8.0 * 10000.0 - 85 * 1.9) / 10000.0};
        EXPECT_NEAR(67.98385, count_before_cast, 1e-6);
        EXPECT_EQ(ret.count(), 67);
    }

    {
        // Compute result is a scalar without unit.
        constexpr auto ret = Meter{1} * MilliMeter{250} / SquareCentiMeter{25} - 5;
        using RetType = std::remove_cv_t<decltype(ret)>;
        EXPECT_EQ(ret, 95);
        EXPECT_TRUE((std::same_as<RetType, std::int64_t>));
    }
}

TEST(how_to_use, comparison_operators)
{
    EXPECT_EQ((KmPerHour{36} <=> MeterPerSecond{10}), std::partial_ordering::equivalent);
    EXPECT_EQ((KmPerHour{36} <=> KmPerHour{40}), std::partial_ordering::less);
    EXPECT_EQ((KmPerHour_double{36.0} <=> MeterPerSecond{10}), std::partial_ordering::equivalent);

    // EXPECT_EQ((Km{10} <=> 10), std::partial_ordering::less); // Does not compile
    // EXPECT_EQ((Km{10} <=> MeterPerSecond{10}), std::partial_ordering::less); // Does not compile
}

} // namespace compound_unit
