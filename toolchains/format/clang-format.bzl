# Clang format rules
load("@bazel_skylib//lib:shell.bzl", "shell")

def _clang_format_impl(ctx):
    out_file = ctx.actions.declare_file(ctx.label.name + ".bash")
    exclude_patterns = ["\\! -path {}".format(shell.quote(p)) for p in ctx.attr.exclude_patterns]
    include_patterns = ["-name {}".format(shell.quote(p)) for p in ctx.attr.patterns]
    workspace = ctx.file.workspace.path if ctx.file.workspace else ""
    substitutions = {
        "@@EXCLUDE_PATTERNS@@": " ".join(exclude_patterns),
        "@@INCLUDE_PATTERNS@@": " -o ".join(include_patterns),
        "@@CLANG_FORMAT@@": shell.quote(ctx.executable.clang_format.short_path),
        "@@DIFF_COMMAND@@": shell.quote(ctx.attr.diff_command),
        "@@MODE@@": shell.quote(ctx.attr.mode),
        "@@WORKSPACE@@": workspace,
    }
    ctx.actions.expand_template(
        template = ctx.file._runner,
        output = out_file,
        substitutions = substitutions,
        is_executable = True,
    )

    files = [ctx.executable.clang_format]
    if ctx.file.workspace:
        files.append(ctx.file.workspace)

    return DefaultInfo(
        runfiles = ctx.runfiles(files = files),
        executable = out_file,
    )

_clang_format_exclude = [
    # Vendored source code dirs
    "./**/vendor/**",
    # Rust cargo build dirs
    "./**/target/**",
    # Directories used exclusively to store build artifacts are still copied into.
    "./build-out/**",
    "./build-bin/**",
    # fusesoc build dir
    "./build/**",
]

clang_format_attrs = {
    "patterns": attr.string_list(
        default = ["*.c", "*.h", "*.cc", "*.cpp"],
        doc = "Filename patterns for format checking",
    ),
    "exclude_patterns": attr.string_list(
        default = _clang_format_exclude,
        doc = "Filename patterns to exlucde from format checking",
    ),
    "mode": attr.string(
        default = "diff",
        values = ["diff", "fix"],
        doc = "Execution mode: display diffs or fix formatting",
    ),
    "diff_command": attr.string(
        default = "diff -u",
        doc = "Command to execute to display diffs",
    ),
    "clang_format": attr.label(
        default = "@llvm_toolchain//:clang-format",  # which is a `native_binary`.
        allow_single_file = True,
        cfg = "host",
        executable = True,
        doc = "The clang-format executable",
    ),
    "workspace": attr.label(
        allow_single_file = True,
        doc = "Label of the WORKSPACE file",
    ),
    "_runner": attr.label(
        default = "clang_format.template.sh",
        allow_single_file = True,
    ),
}

clang_format_check = rule(
    implementation = _clang_format_impl,
    attrs = clang_format_attrs,
    executable = True,
)
