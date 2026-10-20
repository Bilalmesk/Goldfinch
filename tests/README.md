# Tests

The regression tests are plain C and only build the storage and engine
layers, so they do not need Raptor or the SPARQL parser.

## Run the tests

```sh
cd tests
make
```

Or run the same tests under AddressSanitizer and UndefinedBehaviorSanitizer:

```sh
cd tests
make clean
make SANITIZE=1
```

The sanitizer build is supported by GCC on Linux and MSYS2. If your
toolchain does not provide the sanitizer runtimes, use the default build.

## What is covered

`test_engine.c` covers every combination of bound and variable terms in
`getTriples()`:

- exact subject/predicate/object lookup
- one bound term
- two bound terms
- all-variable lookup, including growth past the initial result capacity
- terms that are not present in the dictionary
- a NULL triplestore
