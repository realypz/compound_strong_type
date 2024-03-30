# Bazel buildifier

load("@bazel_skylib//lib:shell.bzl", "shell")
load("//toolchains/common:common.bzl", "BUILDIFIER_REPO_NAME")

def _bazel_buildifier_impl(ctx):
    out_file = ctx.actions.declare_file(ctx.label.name + ".bash")

    substitutions = {
        "@@BAZEL_BUILDIFIER@@": shell.quote(ctx.executable.bazel_buildifier.short_path),
        "@@MODE@@": shell.quote(ctx.attr.mode),
    }

    ctx.actions.expand_template(
        template = ctx.file.runner_script,
        output = out_file,
        substitutions = substitutions,
        is_executable = True,
    )

    files = [ctx.executable.bazel_buildifier]

    return DefaultInfo(
        runfiles = ctx.runfiles(files = files),
        executable = out_file,
    )

bazel_buildifier_attrs = {
    "bazel_buildifier": attr.label(
        default = "@{}//file".format(BUILDIFIER_REPO_NAME),  # which is a `file_group`.
        allow_single_file = True,
        cfg = "host",
        executable = True,
        doc = "The bazel buildifier executable",
    ),
    "mode": attr.string(
        default = "check",
        values = ["check", "fix"],
        doc = "Execution mode: display diffs or fix formatting",
    ),
    "runner_script": attr.label(
        default = "bazel-buildifier.template.sh",
        allow_single_file = True,
    ),
}

bazel_buildifier = rule(
    implementation = _bazel_buildifier_impl,
    attrs = bazel_buildifier_attrs,
    executable = True,
)
