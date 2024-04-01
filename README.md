# Compound Unit
This C++ library implements **strong type** for **compound units**, with its numerical calculation (operator `+` `-` `*` `/`) and numerical comparison operator (`<=>`).

The examples of compound units: `Km/h`, `m/s^2`, `cm^2`.

A specialization of compound unit is determined by:
* Rep: an arithmetic type representing the number of ticks
* UnitSignatures: The signatures of one or several basic unit that consists the compound unit. E.g. The signatures of `m/s^2` are `meter^1` and `second^-2`.

A UnitSignature is determined by:
* Period: a std::ratio representing the tick period (same concept as the period in `std::chrono::duration`).
* Exp: The exponent of the unit. E.g. in `m/s^2`, the exp of `meter` is 1, and the exp of `second` is -2.
* Tag: The tag type of a physical unit (or dimension). E.g. The time signatures (second, millisecond, second^-2 etc.) share the same tag representing "Time". The length signatures (meter, centimeter^2, etc.) share the same tag representing "Length".

The examples of supported operations:
* `5(Km) * 2` => `10(Km)`
* `10(cm) * 1(m)` => `1000(cm^2)`
* `5(Km) / 500(m)` => `10`
* `0.5 * 10(m/s^2) * (2s)^2` => `20.0(m)`

The examples of supported numercial comparison:
* `36(Km/h) <=> 10(m/s)` => `std::partial_ordering::equivalent`
* `36.01(Km/h) <=> 10(m/s)` => `std::partial_ordering::greater`

## Where can I see the examples?
* The example compound units are defined in [`src/tests/compound_unit_examples.h`](https://github.com/realypz/ypz.compound_strong_type/blob/master/src/tests/compound_unit_examples.h).
* The example calculations are in [`src/tests/how_to_use.cpp`](https://github.com/realypz/ypz.compound_strong_type/blob/master/src/tests/how_to_use.cpp).

## Preconditions to build and run the tests
1. Install Bazel. The recommended way to install bazel is via [Bazelisk](https://github.com/bazelbuild/bazelisk).
2. The compiler shall support C++20 or higher C++ standard.

## Run tests
```shell
# Run all the tests
bazelisk test --config=default_cpp20 //...

# Run a single test, change the target name by yourself.
bazelisk run --config=default_cpp20 //src/tests:test_compound_unit
```

## How to format
To do all the formatting, run
```shell
bash toolchains/format/format_all.sh
```

To do formating separately,
1. Clang format
   ```shell
   bazelisk run //toolchains/format:clang_format_fix
   ```

2. Bazel-buildifier
   ```shell
   bazelisk run //toolchains/format:bazel_buildifier_fix
   ```

3. Header guard
   ```shell
   bash toolchains/format/header_guard.sh
   ```
