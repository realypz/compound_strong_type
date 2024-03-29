/*
bazelisk run --config=default_cpp20 //src/tests:test_signature
*/
#include <gtest/gtest.h>

#include "src/signature.h"
#include <cstdint>
#include <ratio>

namespace compound_unit
{
struct TimeTag
{};

struct LengthTag
{};

using Km = UnitSignature<std::ratio<1000, 1>, 1, LengthTag>;
using Meter = UnitSignature<std::ratio<1, 1>, 1, LengthTag>;
using PerHour = UnitSignature<std::ratio<3600, 1>, -1, TimeTag>;
using PerMinute = UnitSignature<std::ratio<60, 1>, -1, TimeTag>;
using PerSecond = UnitSignature<std::ratio<1, 1>, -1, TimeTag>;

TEST(signature_joint_period, _)
{
    {
        {
            using JointPeriod = signature_helper::joint_period_t<Km, PerHour>;
            EXPECT_TRUE((std::ratio_equal_v<JointPeriod, std::ratio<5, 18>>));
        }

        { // swap the two signature
            using JointPeriod = signature_helper::joint_period_t<PerHour, Km>;
            EXPECT_TRUE((std::ratio_equal_v<JointPeriod, std::ratio<5, 18>>));
        }
    }

    {
        using JointPeriod = signature_helper::joint_period_t<Meter, PerMinute, PerMinute>;
        EXPECT_TRUE((std::ratio_equal_v<JointPeriod, std::ratio<1, 3600>>));
    }

    {
        using CentiMeter = UnitSignature<std::ratio<1, 100>, 1, LengthTag>;
        using PerMicrosecondSquare = UnitSignature<std::ratio<1, 1000>, -2, LengthTag>;

        {
            using JointPeriod = signature_helper::joint_period_t<CentiMeter, PerMicrosecondSquare>;
            EXPECT_TRUE((std::ratio_equal_v<JointPeriod, std::ratio<10000, 1>>));
        }

        { // swap the signatures
            using JointPeriod = signature_helper::joint_period_t<PerMicrosecondSquare, CentiMeter>;
            EXPECT_TRUE((std::ratio_equal_v<JointPeriod, std::ratio<10000, 1>>));
        }
    }

    {
        using QubicCentiMeter = UnitSignature<std::ratio<1, 100>, 3, LengthTag>;
        using JointPeriod = signature_helper::joint_period_t<QubicCentiMeter>;
        EXPECT_TRUE((std::ratio_equal_v<JointPeriod, std::ratio<1, 1000000>>));
    }
}
} // namespace compound_unit
