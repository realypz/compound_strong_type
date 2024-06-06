/*
bazelisk run --config=cpp20 //src/tests:test_compound_unit
*/
#include <gtest/gtest.h>

#include "compound_unit_examples.h"
#include "src/compound_unit.h"
#include "src/signature.h"
#include <cmath>
#include <ratio>

namespace compound_unit
{
TEST(compound_unit_member_types, _)
{
    {
        EXPECT_TRUE((std::same_as<KmPerHour::Rep, std::int64_t>));
        EXPECT_TRUE((std::ratio_equal_v<KmPerHour::Period, std::ratio<1000, 3600>>));
    }
}

TEST(constructor, _)
{
    { // Construct from count.
        constexpr KmPerHour v{10};
        EXPECT_EQ(v.count(), 10);
    }

    { // Construct from the same CompoundUnit type.
        constexpr KmPerHour v = KmPerHour{10};
        EXPECT_EQ(v.count(), 10);
    }

    { // Construct from the different CompoundUnit type.
        constexpr KmPerHour v = MeterPerSecond_double{10.0};
        EXPECT_EQ(v.count(), 36);
    }
}

TEST(special_member_functions, _)
{
    EXPECT_TRUE((std::is_trivially_copy_assignable_v<KmPerHour>));

    { // Copy assignment operator.
        constexpr MeterPerSecond v{10};
        KmPerHour v0{};
        v0 = v;
        EXPECT_EQ(v0.count(), 36);
    }
}

TEST(comparison_operator, _)
{
    EXPECT_EQ((KmPerHour{36} <=> MeterPerSecond{10}), std::partial_ordering::equivalent);
    EXPECT_EQ((KmPerHour_double{36.0} <=> MeterPerSecond{10}), std::partial_ordering::equivalent);
    EXPECT_EQ((KmPerHour_double{36.0} <=> KmPerHour_double{NAN}), std::partial_ordering::unordered);
    EXPECT_EQ((KmPerHour_double{36.001} <=> MeterPerSecond{10}), std::partial_ordering::greater);
    EXPECT_EQ((KmPerHour_double{35.999} <=> MeterPerSecond{10}), std::partial_ordering::less);
}

TEST(compound_unit_cast, _)
{
    {
        constexpr auto ret = static_cast<KmPerHour>(MeterPerSecond{10});
        EXPECT_EQ(ret.count(), 36);
        EXPECT_TRUE((std::same_as<decltype(ret.count()), std::int64_t>));
    }

    {
        constexpr auto ret = static_cast<MeterPerSecond>(KmPerHour_double{36.0});
        EXPECT_EQ(ret.count(), 10);
        EXPECT_TRUE((std::same_as<decltype(ret.count()), std::int64_t>));
    }

    {
        constexpr auto ret = static_cast<MeterPerSecond_double>(KmPerHour{36});
        EXPECT_EQ(ret.count(), 10.0);
        EXPECT_TRUE((std::same_as<decltype(ret.count()), double>));
    }
}

TEST(operator_multiply_auto_return, _)
{
    {
        using PerHour = CompoundUnit<std::int64_t, UnitSignature<std::ratio<3600, 1>, -1, TimeTag>>;

        constexpr auto ret = Km{5} * PerHour{1};

        using ReturnType = std::remove_cv_t<decltype(ret)>;

        EXPECT_EQ(ret.count(), 5);

        EXPECT_TRUE((std::ratio_equal_v<ReturnType::Period, std::ratio<5, 18>>));
        EXPECT_EQ(ReturnType::Signatures::size(), 2);
        EXPECT_TRUE((std::same_as<typename ReturnType::Signatures::type_at<0>,
                                  UnitSignature<std::ratio<1000, 1>, 1, LengthTag>>));
        EXPECT_TRUE((std::same_as<typename ReturnType::Signatures::type_at<1>,
                                  UnitSignature<std::ratio<3600, 1>, -1, TimeTag>>));
    }

    // WHEN multiply two compound units with same tag but different period per tag.
    // THEN expect the return value's period is chosen as least ratio per tag.

    { // e.g. lhs: (Km) has ratio 1000:1, (hour) has ratio 3600:1, (Km/h) has
        // ratio 5:18
        //      rhs: (meter) has ratio 1:1, (second^-1) has ratio 1:1, (m/s) has
        //      ratio 1:1 return type: shall be meter^2 * second^-2.
        constexpr auto ret = KmPerHour{36} * MeterPerSecond{20};
        using ReturnType = std::remove_cv_t<decltype(ret)>;
        EXPECT_TRUE((std::ratio_equal_v<ReturnType::Period, std::ratio<1, 1>>));

        EXPECT_EQ(ret.count(), 200);
    }

    { // e.g. lhs: (meter) has ratio 1:1
        //      rhs: (cm) has ratio 1:100
        //      return type: shall be cm^2.
        constexpr auto ret = Meter{7} * CentiMeter{4};
        using ReturnType = std::remove_cv_t<decltype(ret)>;
        EXPECT_TRUE((std::ratio_equal_v<ReturnType::Period, std::ratio<1, 10000>>));
        EXPECT_EQ(ret.count(), 2800);

        using ExpectedRetType =
            CompoundUnit<std::int64_t, UnitSignature<std::ratio<1, 100>, 2, LengthTag>>;
        EXPECT_TRUE((compound_unit_helper::are_compound_unit_equal_v<ReturnType, ExpectedRetType>));
    }

    // WHEN two compound units which offsets each other multiply
    // THEN expect the return value to be a common number type of two operands.
    { // e.g. lhs: (km) has ratio 1000:1
        //      rhs: (m^-1) has ratio 1:1
        //      return type: the common scalar type of two operands
        using Km = CompoundUnit<std::int64_t, UnitSignature<std::ratio<1000, 1>, 1, LengthTag>>;
        using MeterMinusOne =
            CompoundUnit<std::int64_t, UnitSignature<std::ratio<1, 1>, -1, LengthTag>>;

        constexpr auto ret = Km{5} * MeterMinusOne{1};
        using ReturnType = std::remove_cv_t<decltype(ret)>;

        EXPECT_TRUE((std::same_as<ReturnType, std::int64_t>));
        EXPECT_EQ(ret, 5000);
    }

    { // e.g. lhs: (Km) has ratio 1000:1, (hour^-1) has ratio 1:1
        //      rhs: (meter^-1) has ratio 1:1, (second) has ratio 1:1
        //      return type: the common scalar type of two operands
        using MeterPerSecond_inverse_double =
            CompoundUnit<double, UnitSignature<std::ratio<1, 1>, -1, LengthTag>,
                         UnitSignature<std::ratio<1, 1>, 1, TimeTag>>;
        constexpr auto ret = KmPerHour{100} * MeterPerSecond_inverse_double{0.2};
        using ReturnType = std::remove_cv_t<decltype(ret)>;

        EXPECT_TRUE((std::same_as<ReturnType, double>));
        EXPECT_DOUBLE_EQ(ret, 100 * 0.2 / 3.6);
    }

    {
        constexpr auto ret = MeterPerSecond{10} * Minute{1};
        using ReturnType = std::remove_cv_t<decltype(ret)>;

        EXPECT_TRUE((compound_unit_helper::are_compound_unit_equal_v<ReturnType, Meter>));
        EXPECT_EQ(ret.count(), 10 * 60);
    }
}

TEST(operator_divide, _)
{
    { // WHEN three operands and two operator/ are used in one expression
        //      The underlying Rep of the operands are the same.
        // THEN expect the return value and the return type to be correct.
        constexpr auto ret = Meter{20} / Second{2} / Second{2};
        using ReturnType = std::remove_cv_t<decltype(ret)>;

        EXPECT_TRUE((std::ratio_equal_v<ReturnType::Period, std::ratio<1, 1>>));
        EXPECT_EQ(ret.count(), 5);
    }

    { // WHEN three operands and two operator/ are used in one expression.
        //      The underlying Rep of the operands are different.
        //      The operands of same tag have different period.
        // THEN expect the return value and the return type to be correct.
        constexpr auto ret = Meter_double{100} / Second{2} / Minute{1};
        using ReturnType = std::remove_cv_t<decltype(ret)>;

        EXPECT_TRUE((std::ratio_equal_v<ReturnType::Period, std::ratio<1, 1>>));
        EXPECT_DOUBLE_EQ(ret.count(), 100.0 / 2 / 60); // TODO: Fix

        using ExpectedRetType = CompoundUnit<double, UnitSignature<std::ratio<1, 1>, 1, LengthTag>,
                                             UnitSignature<std::ratio<1, 1>, -2, TimeTag>>;
        EXPECT_TRUE((compound_unit_helper::are_compound_unit_equal_v<ReturnType, ExpectedRetType>));
    }

    { // WHEN three operands and two operator/ are used in one expression.
        //      The underlying Rep of the operands are different.
        //      The operands of same tag have same period.
        // THEN expect the return value and the return type to be correct.
        constexpr Meter_double distance{300};
        constexpr Second time{3};

        constexpr auto ret = distance / time / time;
        using ReturnType = std::remove_cv_t<decltype(ret)>;

        EXPECT_DOUBLE_EQ(ret.count(), 100.0 / 3.0);

        using ExpectedRetType = CompoundUnit<double, UnitSignature<std::ratio<1, 1>, 1, LengthTag>,
                                             UnitSignature<std::ratio<1, 1>, -2, TimeTag>>;
        EXPECT_TRUE((compound_unit_helper::are_compound_unit_equal_v<ReturnType, ExpectedRetType>));
    }

    { // WHEN two operands are used in one expression.
        //      The underlying Rep of the operands are different.
        //      The operands types are convertible with each other.
        // THEN expect a scalar type return value.
        {
            constexpr auto ret = Meter{300} / Meter_double{9.0};
            EXPECT_DOUBLE_EQ(ret, 100.0 / 3.0);
        }
        {
            constexpr auto ret = KmPerHour_double{360.0} / MeterPerSecond{500};
            EXPECT_DOUBLE_EQ(ret, 0.2);
        }
    }

    {
        constexpr Meter distance{1500};
        constexpr Minute_double time{6};
        {
            constexpr auto ret_0 = distance / time;
            using ReturnType = std::remove_cv_t<decltype(ret_0)>;

            EXPECT_EQ(ret_0.count(), 250.0);
            EXPECT_TRUE((std::ratio_equal_v<ReturnType::Period, std::ratio<1, 60>>));

            constexpr MeterPerSecond_double ret_1{distance / time}; // 250 m/min = 25/6 m/s
            EXPECT_DOUBLE_EQ(ret_1.count(), 25.0 / 6.0);
            EXPECT_TRUE((compound_unit_helper::are_compound_units_castable_v<MeterPerSecond_double,
                                                                             ReturnType>));
        }
    }
}

TEST(operator_plus_minus, _)
{
    { // WHEN two operands have the identical compound unit type.
        // THEN expect the return value and the return type to be correct.
        constexpr auto ret = -Km{5} + Km{3};
        using ReturnType = std::remove_cv_t<decltype(ret)>;
        EXPECT_EQ(ret.count(), -2);
        EXPECT_TRUE((compound_unit_helper::are_compound_unit_equal_v<ReturnType, Km>));
    }

    { // WHEN two operands have different Rep and period.
        // THEN the return type shall have the common Rep and the smaller period.
        constexpr auto ret = Km{5} + Meter_double{300.0};
        using ReturnType = std::remove_cv_t<decltype(ret)>;
        EXPECT_DOUBLE_EQ(ret.count(), 5300.0);
        EXPECT_TRUE((compound_unit_helper::are_compound_unit_equal_v<ReturnType, Meter_double>));
    }

    { // WHEN two operands have different Rep and period.
        constexpr auto ret = KmPerHour{360} - MeterPerSecond_double{120.0};
        using ReturnType = std::remove_cv_t<decltype(ret)>;
        EXPECT_DOUBLE_EQ(ret.count(), 360.0 - 120.0 * 3.6);
        EXPECT_TRUE(
            (compound_unit_helper::are_compound_unit_equal_v<ReturnType, KmPerHour_double>));
    }

    {
        constexpr auto ret = SquareMeter{100} - SquareCentiMeter{12} + SquareMillimeter{25};
        using ReturnType = std::remove_cv_t<decltype(ret)>;
        EXPECT_EQ(ret.count(), 100 * 1000 * 1000 - 12 * 10 * 10 + 25);
        EXPECT_TRUE(
            (compound_unit_helper::are_compound_unit_equal_v<ReturnType, SquareMillimeter>));
    }
}
} // namespace compound_unit
