## Run all the tests
```shell
bazelisk test --config=default_cpp20 //...
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

4. Rust format
   ```shell
   bazelisk run @rules_rust//:rustfmt
   ```

5. Python format
   **NOTE** Please install your black by `pip install black` in your system built-in python. 
   ```shell
   black ./
   ```
