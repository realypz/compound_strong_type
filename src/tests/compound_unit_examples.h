#ifndef SRC_TESTS_COMPOUND_UNIT_EXAMPLES_H_
#define SRC_TESTS_COMPOUND_UNIT_EXAMPLES_H_

#include "src/compound_unit.h"
#include "src/signature.h"

#include <ratio>

struct TimeTag
{};

struct LengthTag
{};

namespace compound_unit
{
using RatioOne = std::ratio<1, 1>;
/// Velocity and acceleration units.
///@{
using KmPerHour = CompoundUnit<std::int64_t, UnitSignature<std::kilo, 1, LengthTag>,
                               UnitSignature<std::ratio<3600, 1>, -1, TimeTag>>;

using KmPerHour_double = CompoundUnit<double, UnitSignature<std::kilo, 1, LengthTag>,
                                      UnitSignature<std::ratio<3600, 1>, -1, TimeTag>>;
using MeterPerSecond = CompoundUnit<std::int64_t, UnitSignature<RatioOne, 1, LengthTag>,
                                    UnitSignature<RatioOne, -1, TimeTag>>;

using MeterPerSecondSquare = CompoundUnit<std::int64_t, UnitSignature<RatioOne, 1, LengthTag>,
                                          UnitSignature<RatioOne, -2, TimeTag>>;

using MeterPerSecond_double = CompoundUnit<double, UnitSignature<RatioOne, 1, LengthTag>,
                                           UnitSignature<RatioOne, -1, TimeTag>>;
///@}

/// Length units.
///@{
using Km = CompoundUnit<std::int64_t, UnitSignature<std::kilo, 1, LengthTag>>;
using Km_double = CompoundUnit<double, UnitSignature<std::kilo, 1, LengthTag>>;
using Meter = CompoundUnit<std::int64_t, UnitSignature<RatioOne, 1, LengthTag>>;
using Meter_double = CompoundUnit<double, UnitSignature<RatioOne, 1, LengthTag>>;
using CentiMeter = CompoundUnit<std::int64_t, UnitSignature<std::centi, 1, LengthTag>>;
using CentiMeter_double = CompoundUnit<double, UnitSignature<std::centi, 1, LengthTag>>;
using MilliMeter = CompoundUnit<std::int64_t, UnitSignature<std::milli, 1, LengthTag>>;
using MilliMeter_double = CompoundUnit<double, UnitSignature<std::milli, 1, LengthTag>>;
///@}

/// Time units.
///@{
using Hour = CompoundUnit<std::int64_t, UnitSignature<std::ratio<3600, 1>, 1, TimeTag>>;
using Hour_double = CompoundUnit<double, UnitSignature<std::ratio<3600, 1>, 1, TimeTag>>;
using Minute = CompoundUnit<std::int64_t, UnitSignature<std::ratio<60, 1>, 1, TimeTag>>;
using Minute_double = CompoundUnit<double, UnitSignature<std::ratio<60, 1>, 1, TimeTag>>;
using Second = CompoundUnit<std::int64_t, UnitSignature<RatioOne, 1, TimeTag>>;
using Second_double = CompoundUnit<double, UnitSignature<RatioOne, 1, TimeTag>>;
///@}

/// Area units.
///@{
using SquareMeter = CompoundUnit<std::int64_t, UnitSignature<RatioOne, 2, LengthTag>>;
using SquareMeter_double = CompoundUnit<double, UnitSignature<RatioOne, 2, LengthTag>>;
using SquareCentiMeter = CompoundUnit<std::int64_t, UnitSignature<std::centi, 2, LengthTag>>;
using SquareCentiMeter_double = CompoundUnit<double, UnitSignature<std::centi, 2, LengthTag>>;
using SquareMillimeter = CompoundUnit<std::int64_t, UnitSignature<std::milli, 2, LengthTag>>;
///@}

} // namespace compound_unit

#endif // SRC_TESTS_COMPOUND_UNIT_EXAMPLES_H_
