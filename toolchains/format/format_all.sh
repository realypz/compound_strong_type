# C++ files
## NOTE: The order matters, since clang format might change linespaces after header guard has run.
bash toolchains/format/header_guard.sh
bazelisk run //toolchains/format:clang_format_fix

# Bazel files
bazelisk run //toolchains/format:bazel_buildifier_fix

bazelisk run @rules_rust//:rustfmt

black ./
