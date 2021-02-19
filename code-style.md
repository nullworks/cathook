# NullWorks code style (Work in progress)

## C/C++

### General

- Code must be formatted with `clang-format`, the `.clang-format` settings file is provided within this repo.
- `using namespace` is strictly forbidden.
- [Hungarian notation](https://en.wikipedia.org/wiki/Hungarian_notation) is forbidden.
- Comments should be above the relevant code.
- Use [modern c++](http://www.modernescpp.com/index.php/what-is-modern-c). (Usage of `make_unique` instead of `new`, etc)

### File names

`PascalCase`.

### Header files

Header files must have extension of `.hpp` and be guarded with `#pragma once` in the beginning.

### Variable names

- Variable names must be `lower_snake_case`, member variable names **must not** be prefixed by `m_` or any other prefixes.
- Constants must be in `UPPER_SNAKE_CASE` and use `constexpr`, not `#define`. For example: `constexpr int MAX = 100;`.

### Namespace names

Namespace names follow the same rules as *Variable names*.

### Function names

Function/method names must be in `lowerCamelCase`.

### Class names

- Class names must be in `PascalCase`.
- Struct names must be in `lower_snake_case`.
