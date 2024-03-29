/*
bazelisk run --config=default_cpp20 //src/tests:test_compound_unit
*/
#include <gtest/gtest.h>

#include "src/compound_unit.h"
#include "src/signature.h"

#include <ratio>

struct TimeTag
{
    static constexpr std::string_view kName{"TimeTag"};
};

struct LengthTag
{
    static constexpr std::string_view kName{"LengthTag"};
};

namespace compound_unit
{
using KmPerHour = CompoundUnit<std::int64_t, UnitSignature<std::ratio<1000, 1>, 1, LengthTag>,
                               UnitSignature<std::ratio<3600, 1>, -1, TimeTag>>;

using KmPerHour_double = CompoundUnit<double, UnitSignature<std::ratio<1000, 1>, 1, LengthTag>,
                                      UnitSignature<std::ratio<3600, 1>, -1, TimeTag>>;
using MeterPerSecond = CompoundUnit<std::int64_t, UnitSignature<std::ratio<1, 1>, 1, LengthTag>,
                                    UnitSignature<std::ratio<1, 1>, -1, TimeTag>>;

using MeterPerSecond_square =
    CompoundUnit<std::int64_t, UnitSignature<std::ratio<1, 1>, 1, LengthTag>,
                 UnitSignature<std::ratio<1, 1>, -2, TimeTag>>;

using MeterPerSecond_double = CompoundUnit<double, UnitSignature<std::ratio<1, 1>, 1, LengthTag>,
                                           UnitSignature<std::ratio<1, 1>, -1, TimeTag>>;

using Km = CompoundUnit<std::int64_t, UnitSignature<std::ratio<1000, 1>, 1, LengthTag>>;
using Km_double = CompoundUnit<double, UnitSignature<std::ratio<1000, 1>, 1, LengthTag>>;
using Meter = CompoundUnit<std::int64_t, UnitSignature<std::ratio<1, 1>, 1, LengthTag>>;
using Meter_double = CompoundUnit<double, UnitSignature<std::ratio<1, 1>, 1, LengthTag>>;
using CentiMeter = CompoundUnit<std::int64_t, UnitSignature<std::ratio<1, 100>, 1, LengthTag>>;
using CentiMeter_double = CompoundUnit<double, UnitSignature<std::ratio<1, 100>, 1, LengthTag>>;

using Hour = CompoundUnit<std::int64_t, UnitSignature<std::ratio<3600, 1>, 1, TimeTag>>;
using Hour_double = CompoundUnit<double, UnitSignature<std::ratio<3600, 1>, 1, TimeTag>>;
using Minute = CompoundUnit<std::int64_t, UnitSignature<std::ratio<60, 1>, 1, TimeTag>>;
using Minute_double = CompoundUnit<double, UnitSignature<std::ratio<60, 1>, 1, TimeTag>>;
using Second = CompoundUnit<std::int64_t, UnitSignature<std::ratio<1, 1>, 1, TimeTag>>;
using Second_double = CompoundUnit<double, UnitSignature<std::ratio<1, 1>, 1, TimeTag>>;

using SquareMeter = CompoundUnit<std::int64_t, UnitSignature<std::ratio<1, 1>, 2, LengthTag>>;
using SquareCentiMeter =
    CompoundUnit<std::int64_t, UnitSignature<std::ratio<1, 100>, 2, LengthTag>>;
using SquareCentiMeter_double =
    CompoundUnit<double, UnitSignature<std::ratio<1, 100>, 2, LengthTag>>;
using SquareMillimeter =
    CompoundUnit<std::int64_t, UnitSignature<std::ratio<1, 1000>, 2, LengthTag>>;

using MeterPerSecondSquare =
    CompoundUnit<std::int64_t, UnitSignature<std::ratio<1, 1>, 1, LengthTag>,
                 UnitSignature<std::ratio<1, 1>, -2, TimeTag>>;

TEST(compound_unit_member_types, _)
{
    {
        EXPECT_TRUE((std::same_as<KmPerHour::Rep, std::int64_t>));
        EXPECT_TRUE((std::ratio_equal_v<KmPerHour::Period, std::ratio<1000, 3600>>));
    }
}

TEST(temp, extract_common_signature)
{
    using LSignaturesTuple = KmPerHour::Signatures;
    using RSignaturesTuple = Second::Signatures;
    constexpr auto length_signature =
        impl::extractCommonSignature(LengthTag{}, LSignaturesTuple{}, RSignaturesTuple{});
    EXPECT_EQ(length_signature.exp, 1);
    EXPECT_EQ(length_signature.num, 1000);
    EXPECT_EQ(length_signature.den, 1);

    constexpr auto time_signature =
        impl::extractCommonSignature(TimeTag{}, LSignaturesTuple{}, RSignaturesTuple{});
    EXPECT_EQ(time_signature.exp, 0);
}

TEST(compound_unit_cast, _)
{
    {
        constexpr auto ret = compound_unit_cast<KmPerHour>(MeterPerSecond{10});
        EXPECT_EQ(ret.count(), 36);
        EXPECT_TRUE((std::same_as<decltype(ret.count()), std::int64_t>));
    }

    {
        constexpr auto ret = compound_unit_cast<MeterPerSecond>(KmPerHour_double{36.0});
        EXPECT_EQ(ret.count(), 10);
        EXPECT_TRUE((std::same_as<decltype(ret.count()), std::int64_t>));
    }

    {
        constexpr auto ret = compound_unit_cast<MeterPerSecond_double>(KmPerHour{36});
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
        EXPECT_TRUE((are_compound_unit_equal_v<ReturnType, ExpectedRetType>));
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

        EXPECT_TRUE((are_compound_unit_equal_v<ReturnType, Meter>));
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
        EXPECT_TRUE((are_compound_unit_equal_v<ReturnType, ExpectedRetType>));
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
        EXPECT_TRUE((are_compound_unit_equal_v<ReturnType, ExpectedRetType>));
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
            EXPECT_TRUE((are_compound_unit_convertable_v<MeterPerSecond_double, ReturnType>));
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
        EXPECT_TRUE((are_compound_unit_equal_v<ReturnType, Km>));
    }

    { // WHEN two operands have different Rep and period.
        // THEN the return type shall have the common Rep and the smaller period.
        constexpr auto ret = Km{5} + Meter_double{300.0};
        using ReturnType = std::remove_cv_t<decltype(ret)>;
        EXPECT_DOUBLE_EQ(ret.count(), 5300.0);
        EXPECT_TRUE((are_compound_unit_equal_v<ReturnType, Meter_double>));
    }

    { // WHEN two operands have different Rep and period.
        constexpr auto ret = KmPerHour{360} - MeterPerSecond_double{120.0};
        using ReturnType = std::remove_cv_t<decltype(ret)>;
        EXPECT_DOUBLE_EQ(ret.count(), 360.0 - 120.0 * 3.6);
        EXPECT_TRUE((are_compound_unit_equal_v<ReturnType, KmPerHour_double>));
    }

    {
        constexpr auto ret = SquareMeter{100} - SquareCentiMeter{12} + SquareMillimeter{25};
        using ReturnType = std::remove_cv_t<decltype(ret)>;
        EXPECT_EQ(ret.count(), 100 * 1000 * 1000 - 12 * 10 * 10 + 25);
        EXPECT_TRUE((are_compound_unit_equal_v<ReturnType, SquareMillimeter>));
    }
}

TEST(compound_expression, _)
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
        constexpr MeterPerSecond v0{10};
        constexpr Minute t{1};
        constexpr MeterPerSecondSquare a{1};

        constexpr auto ret = v0 * t + a / 2.0 * t * t;
        using RetType = std::remove_cv_t<decltype(ret)>;

        EXPECT_EQ(ret.count(), 10 * 60.0 + 1 / 2.0 * 60 * 60);
        EXPECT_TRUE((are_compound_unit_equal_v<RetType, Meter_double>));
    }

    {
        // Compute area with all integers
        constexpr auto ret = Meter{17} / 2 * Meter{8} + CentiMeter{85} * CentiMeter_double{1.9};
        using RetType = std::remove_cv_t<decltype(ret)>;

        EXPECT_DOUBLE_EQ(ret.count(), 8 * 8 * 10000.0 + 85 * 1.9);
        EXPECT_TRUE((are_compound_unit_equal_v<RetType, SquareCentiMeter_double>));
    }

    {
        // Compute area with all double
        constexpr auto ret = Meter{17} / 2.0 * Meter{8} - CentiMeter{85} * CentiMeter_double{1.9};
        using RetType = std::remove_cv_t<decltype(ret)>;

        EXPECT_DOUBLE_EQ(ret.count(), 8.5 * 8.0 * 10000.0 - 85 * 1.9);
        EXPECT_TRUE((are_compound_unit_equal_v<RetType, SquareCentiMeter_double>));
    }
}

} // namespace compound_unit
