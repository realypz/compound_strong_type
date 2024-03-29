# See releases at https://github.com/bazelbuild/rules_rust/releases/
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def load_rust():
    http_archive(
        name = "rules_rust",
        sha256 = "c46bdafc582d9bd48a6f97000d05af4829f62d5fee10a2a3edddf2f3d9a232c1",
        urls = ["https://github.com/bazelbuild/rules_rust/releases/download/0.28.0/rules_rust-v0.28.0.tar.gz"],
    )
