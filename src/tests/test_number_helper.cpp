/*
bazelisk run --config=cpp20 //src/tests:test_helpers
*/
#include <gtest/gtest.h>

#include "src/helpers/number.h"
#include <cstdint>
#include <ratio>

namespace compound_unit::number_helper
{
TEST(ratio_gcd, _)
{
    {
        using Ratio1 = std::ratio<1000, 1>;
        using Ratio2 = std::ratio<5, 18>;

        using NewRatio = ratio_gcd<Ratio1, Ratio2>::type;
        EXPECT_EQ(NewRatio::num, 5);
        EXPECT_EQ(NewRatio::den, 18);
    }

    {
        using Ratio1 = std::ratio<60, 1>;
        using Ratio2 = std::ratio<1, 25>;

        {
            using NewRatio = ratio_gcd<Ratio1, Ratio2>::type;
            EXPECT_EQ(NewRatio::num, 1);
            EXPECT_EQ(NewRatio::den, 25);
        }
        {
            using NewRatio = ratio_gcd<Ratio2, Ratio1>::type;
            EXPECT_EQ(NewRatio::num, 1);
            EXPECT_EQ(NewRatio::den, 25);
        }
    }
}

TEST(ratios_multiply, _)
{
    {
        using R0 = std::ratio<1, 1>;
        using R1 = std::ratio<60, 1>;

        using JointRatio = ratios_multiply_t<R0, R1>;
        EXPECT_EQ(JointRatio::num, 60);
        EXPECT_EQ(JointRatio::den, 1);
    }

    {
        using R0 = std::ratio<1, 1>;
        using R1 = std::ratio<1, 60>;
        using JointRatio = ratios_multiply_t<R0, R1>;
        EXPECT_EQ(JointRatio::num, 1);
        EXPECT_EQ(JointRatio::den, 60);
    }

    {
        using R0 = std::ratio<58, 37>;
        using JointRatio = ratios_multiply_t<R0>;
        EXPECT_EQ(JointRatio::num, 58);
        EXPECT_EQ(JointRatio::den, 37);
    }

    {
        using R0 = std::ratio<1, 1>;
        using R1 = std::ratio<60, 1>;
        using R2 = std::ratio<1, 60>;
        using JointRatio = ratios_multiply_t<R0, R1, R2>;
        EXPECT_EQ(JointRatio::num, 1);
        EXPECT_EQ(JointRatio::den, 1);
    }

    {
        using R0 = std::ratio<1, 1000>;
        using R1 = std::ratio<60, 1>;
        using R2 = std::ratio<5, 18>;
        using JointRatio = ratios_multiply_t<R0, R1, R2>;
        EXPECT_EQ(JointRatio::num, 1);
        EXPECT_EQ(JointRatio::den, 60);
    }

    {
        // using JointRatio = ratios_multiply_t<>; Error, does not support empty ratio list.
    }
}

TEST(ratio_pow, case_positive_exp)
{
    {
        using R0 = std::ratio<1, 1>;
        using JointRatio = ratio_pow_t<R0, 3>;
        EXPECT_EQ(JointRatio::num, 1);
        EXPECT_EQ(JointRatio::den, 1);
    }

    {
        using R0 = std::ratio<5, 7>;
        using JointRatio = ratio_pow_t<R0, 3>;
        EXPECT_TRUE((std::ratio_equal_v<JointRatio, std::ratio<125, 343>>));
    }

    {
        using R0 = std::ratio<5, 7>;
        using JointRatio = ratio_pow_t<R0, -2>;
        EXPECT_TRUE((std::ratio_equal_v<JointRatio, std::ratio<49, 25>>));
    }
}
} // namespace compound_unit::number_helper
