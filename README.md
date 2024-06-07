# Compound Unit
## Introduction
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

## What header files shall I use?
The public headers are
* [`ypz/strong_type/compound_unit.h`](src/include/ypz/strong_type/compound_unit.h), which provies the strong type class template `CompoundUnit`and operator `+-*/` overloading.
* [`ypz/strong_type/signature.h`](src/include/ypz/strong_type/signature.h), which provides class template `UnitSignature`.

The public APIs are under namespace `cpu`, the helper namespaces under `cpu` are not intended for public usage.

## Where can I see the examples?
* The example compound units are defined in [`src/tests/compound_unit_def.h`](./src/tests/compound_unit_def.h).
* The example calculations of compound units are in [`src/tests/how_to_use.cpp`](./src/tests/how_to_use.cpp).

## Steps to use the strong type library in your Bazel project
1. Install Bazel. The recommended way to install bazel is via [Bazelisk](https://github.com/bazelbuild/bazelisk).
2. In your own bazel project, copy paste the following to `MODULE.bazel` of your project and replace the commit to the latest commit hash from main.
   ```
   bazel_dep(name = "ypz.compound_strong_type", version = "0.0.0")
   git_override(
      module_name = "ypz.compound_strong_type",
      commit = "...",
      remote = "https://github.com/realypz/ypz.compound_strong_type.git",
   )
   ```
3. Add `"@ypz.compound_strong_type//:lib"` to the `dep` list of `cc_library` or `cc_binary` rules.
4. `#include <ypz/strong_type/compound_unit.h>` and `#include <ypz/strong_type/signature.h>`
5. Build with **`--cxxopt="std=c++20"`** or higher C++ standard.

## Run all tests
```shell
# Run all the tests
bazelisk test --config=cpp20 //...
```

## How to format everything in this repo?
```shell
bash toolchains/format/format_all.sh
```
