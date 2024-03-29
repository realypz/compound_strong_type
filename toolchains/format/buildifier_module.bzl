load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_file")
load("//toolchains/common:common.bzl", "BUILDIFIER_REPO_NAME")

def _bazel_buildifier_impl(module_ctx):
    sha256 = None
    url = None

    if module_ctx.os.name == "mac os x" and module_ctx.os.arch == "aarch64":
        sha256 = "fa07ba0d20165917ca4cc7609f9b19a8a4392898148b7babdf6bb2a7dd963f05"
        url = "https://github.com/bazelbuild/buildtools/releases/download/v6.4.0/buildifier-darwin-arm64"

    elif module_ctx.os.name == "linux" and module_ctx.os.arch == "amd64":
        sha256 = "be63db12899f48600bad94051123b1fd7b5251e7661b9168582ce52396132e92"
        url = "https://github.com/bazelbuild/buildtools/releases/download/v6.4.0/buildifier-linux-amd64"

    else:
        fail("The os is not supported yet with buildifier!")

    # NOTE: http_file is a repo rule that creates a bazel repo!
    http_file(
        name = BUILDIFIER_REPO_NAME,
        executable = True,
        sha256 = sha256,
        urls = [url],
    )

bazel_buildifier = module_extension(
    implementation = _bazel_buildifier_impl,
)
