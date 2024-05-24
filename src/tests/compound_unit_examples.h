#ifndef _SRC_TESTS_COMPOUND_UNIT_EXAMPLES_H_
#define _SRC_TESTS_COMPOUND_UNIT_EXAMPLES_H_

#include "src/compound_unit.h"
#include "src/signature.h"

#include <ratio>

struct TimeTag
{};

struct LengthTag
{};

struct MassTag
{};

namespace compound_unit
{
using RatioOne = std::ratio<1, 1>;

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

/// Velocity and acceleration units.
///@{
using KmPerHour = DivUnit<Km, Hour>;
using KmPerHour_double = DivUnit<Km_double, Hour_double>;
using MeterPerSecond = DivUnit<Meter, Second>;
using MeterPerSecondSquare = DivUnit<MeterPerSecond, Second>;
using MeterPerSecond_double = CompoundUnit<double, UnitSignature<RatioOne, 1, LengthTag>,
                                           UnitSignature<RatioOne, -1, TimeTag>>;
// or DivUnit<MeterPerSecond_double, Second_double>
///@}

/// Area units.
///@{
using SquareMeter = MulUnit<Meter, Meter>;
using SquareMeter_double = MulUnit<Meter_double, Meter_double>;
using SquareCentiMeter = MulUnit<CentiMeter, CentiMeter>;
using SquareCentiMeter_double = MulUnit<CentiMeter_double, CentiMeter_double>;
using SquareMillimeter = MulUnit<MilliMeter, MilliMeter>;
///@}

/// Mass units.
using Kg = CompoundUnit<std::int64_t, UnitSignature<RatioOne, 1, MassTag>>;

/// Force units.
// clang-format off
using Newton = CompoundUnit<std::int64_t,
                            UnitSignature<RatioOne, 1, MassTag>,
                            UnitSignature<RatioOne, 1, LengthTag>, 
                            UnitSignature<RatioOne, -2, TimeTag>>;
// clang-format on
using Newton_alias = MulUnit<Kg, MeterPerSecondSquare>; // same as Newton

} // namespace compound_unit

#endif // _SRC_TESTS_COMPOUND_UNIT_EXAMPLES_H_
