# C++ files
## NOTE: The order matters, since clang format might change linespaces after header guard has run.
set -e

bash toolchains/python/venv/create_venv.sh

source .venv/bin/activate
black ./
python toolchains/format/header_guard.py
deactivate

bazelisk run //toolchains/format:clang_format_fix

# Bazel files
bazelisk run //toolchains/format:bazel_buildifier_fix

bazelisk run @rules_rust//:rustfmt

git_diff=$(echo $(git diff))

if [ ! -z "$git_diff" -a "$git_diff" != " " ]; then
    BOLD='\033[1m'
    NONE='\033[00m'
    echo -e "${BOLD}Content has been modified. Please git commit the changes.${NONE}"
    exit 1
fi

exit 0