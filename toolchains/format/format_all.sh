# C++ files
## NOTE: The order matters, since clang format might change linespaces after header guard has run.
set -e

bash toolchains/python/venv/create_venv.sh

source .venv/bin/activate
black ./
deactivate

bazelisk run //toolchains/external:header_guard -- --workspace-root=$(pwd)

bazelisk run //toolchains/external:clang_format_fix

# Bazel files
bazelisk run //toolchains/external:bazel_buildifier_fix

bazelisk run @rules_rust//:rustfmt

git_diff=$(echo $(git diff))

if [ ! -z "$git_diff" -a "$git_diff" != " " ]; then
    BOLD='\033[1m'
    NONE='\033[00m'
    echo -e "❌ ${BOLD}The following files have been modified after formatting. Please git commit the changes.${NONE}"
    echo $(git diff --name-only)
    exit 1
fi

echo "✅ All files are unchanged after formatting!"
exit 0