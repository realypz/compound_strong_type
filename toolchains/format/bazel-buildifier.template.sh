#!/usr/bin/env bash

MODE=@@MODE@@
BAZEL_BUILDIFIER=@@BAZEL_BUILDIFIER@@
bazel_buildifier=$(realpath "$BAZEL_BUILDIFIER")

cd ${BUILD_WORKSPACE_DIRECTORY} || exit 1

case "$MODE" in
    check)
        echo "The following bazel files need to be reformatted:"

        RESULT=0
        ${bazel_buildifier} -r -mode=check .
        RESULT=$(($RESULT | $?))

        if [[ $RESULT == 0 ]]; then
            echo "All bazel files are good formatted. ✔️"
        fi

        exit $RESULT
        ;;
    fix)
        ${bazel_buildifier} -r -mode=fix .
        ;;
    *)
        echo "Unknown mode: $MODE"
        exit 1
esac
