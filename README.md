# just-parser

**just-parser** is a lightweight C++17 library for parsing and serializing hierarchical data in the Just text format. It provides a JSON-like object model with named nodes, nested blocks, typed values, and primitive arrays.

Just is its own format and is **not JSON-compatible**.

## Features

- Parse data from a `std::string`, a pointer and length, or a file.
- Represent nested objects as named blocks and navigate them with slash-separated paths.
- Read strings, signed 64-bit integers, real numbers, and booleans.
- Read arrays of strings, integers, real numbers, and booleans. Integer elements are promoted when an array also contains real numbers; incompatible element types are rejected.
- Use `//` line comments and optional commas between values.
- Find nodes by path or search recursively by node name.
- Access values through typed getters, `value<T>()`, explicit conversions, and `operator<<` navigation.
- Serialize parsed data in compact or human-readable form.
- Reuse a parser instance to replace its previously parsed document and query its reported storage usage.

## Format

Values are written after their property names. Braces create nested blocks or arrays; arrays contain primitive values of a compatible type.

```text
application {
	name "just-parser"
	retries 3
	threshold 0.75
	enabled true
	tags { "parser", "configuration" }

	database {
		host "localhost"
		port 5432
	}
}
```

Strings use double quotes. A backslash escapes the following character, including a quote or another backslash.

## Usage

```cpp
#include <iostream>
#include <justparser>

int main() {
	just::just_object_parser parser;
	parser.deserialize(R"(
		application {
			name "just-parser"
			retries 3
			tags { "parser", "configuration" }
		}
	)");

	if (auto* name = parser.at("application/name")) {
		std::cout << name->value<just::jstring>() << '\n';
	}

	if (auto* retries = parser.at("application/retries")) {
		std::cout << retries->value<int>() << '\n';
	}

	if (auto* tags = parser.at("application/tags")) {
		for (const auto& tag : tags->get_array<just::jstring>()) {
			std::cout << tag << '\n';
		}
	}

	std::cout << parser.serialize(just::JustSerializeFormat::JustBeautify);
}
```

The main API is in the `just` namespace and declared in `include/justparser`.

### Parsing and lookup

- `deserialize(text)` parses a string.
- `deserialize(data, length)` parses a bounded character buffer.
- `deserialize_from(path)` reads and parses a file.
- `at("parent/child")` returns a node at a slash-separated path, or `nullptr` if it does not exist.
- `contains(path)` checks whether a path exists.
- `search(name)` recursively finds a node by name.
- `treeCount()` returns the number of top-level nodes.

### Node values

Nodes expose their `JustType` through `type()`, and their structure through `has_tree()`, `has_value()`, and `is_array()`. Use `value<T>()` or the corresponding getters (`get_int()`, `get_real()`, `get_bool()`, and `get_str()`) for scalar values. Use `get_array<T>()` for primitive arrays. The `<<` operator provides chained navigation through child nodes.

### Serialization and storage

`serialize()` defaults to compact output. Pass `JustSerializeFormat::JustBeautify` for indented output. `occupied_memory()` reports the parser's tracked storage usage; it is an estimate of parser-managed allocations, not total process memory.

## Build and test

Requirements: CMake 3.10 or newer and a C++17 compiler.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The CMake project builds the `justio` library, the `just-example` executable, and the `just-test` test suite.

## Current limitations

- Arrays contain primitive values; nested arrays and arrays of objects are not supported.
- Arrays may mix integers and real numbers, but other incompatible element types are rejected.
- `JustAllocationMethod::fixed_allocation` is reserved for future work; the current implementation uses dynamic allocation.
- The library currently provides no install or package-export rules.

## License

This project is licensed under the GNU General Public License v3.0. See [LICENSE](LICENSE) for the full text.
