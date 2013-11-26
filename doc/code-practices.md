# Project Processes and Coding Style

## Licensing and Copyright

All files shall begin with a SPDX-conforming [license
identifier](https://spdx.org/using-spdx-license-identifier), e.g.:

    /* SPDX-License-Identifier: Apache-2.0 */

in a style consistent with [Linux][linux-spdx].  Immediately following this
shall be a single-line authorship assertion, e.g.:

    /* Copyright YEAR(S) NAME */

Where the SPDX license disclaims copyright (as with `CC0-1.0`) the assertion
shall use `Written in YEAR(S) by NAME` in replacement of `Copyright YEAR(S)
NAME`.

Note that Linux uses distinct styles for header (`/* */`) and implementation
(`//`) files.  This shall be followed for both the SPDX and authorship lines.

Copies of the official license text for all used licenses shall be present in
the repository in the top-level `LICENSE` directory in text form named with the
license short identifier.  These files shall be obtained from the license
original source, or from the [SPDX first-level derived repository][spdx-text].

The bulk of code in this project is licensed under Apache-2.0, but examples and
tests generally use CC-BY-SA-4.0, under the belief that this simplifies the
common situation of modifying an example for a specific purpose in a project
that uses different but compatible licensing.  Material with little/no
intellectual value is generally labeled as CC0-1.0.

## Versioning

The project attempts to follow:
* [Semantic Versioning][semver] with release versions distinguished from
  pre-release versions with a `-dev.YYYYMMDDXX` suffix.
* [Changelog][]


## Documentation

* Code documentation uses [Doxygen](http://www.doxygen.nl/).  Non-code
  documentation uses [CommonMark](https://commonmark.org/) generally, and may
  use [GFM](https://github.github.com/gfm/) extensions.

* All documentation blocks shall begin with a short (one-sentence)
  command capitalized and closing with a period.  Doxygen is configured to
  treat these as if `@brief` had been provided.

* Markdown formatting shall be preferred over Doxygen commands where both
  exist.  For example, use `**now**` instead of `@b now` and `` `code` ``
  instead of `@c code`.  An exception is that `@p param` is preferred over
  `param` to style references to named parameters.

## Code Style

The point of departure for both style and implementation is the [Joint Strike
Fighter Air Vehicle C++ Coding Standards][jsf-av].  Below are a point-by-point
discussion of diversions from that standard.

### 4.4 Environment

* AV-8 is advanced to require conformance to ISO/IEC 14882:2017, allowing all
  features supported in C++17.

* AV-9 and AV-10 (charset restrictions) are relaxed to allow Unicode in UTF-8
  format within comments and, if necessary, strings.  Names will be restricted
  to ISO-10646-1.

  *NOTE:* In some cases use of a non-breaking space `U+00A0` may appear in
  comments to prevent generating confusing Doxygen output, e.g. where word wrap
  would otherwise cause a `*` in inline code to be interpreted as a bullet list
  marker, or a value would be separated from its unit.

### 4.5 Libraries

* AV-17 (no `errno`) is disregarded when using library functions that
  communicate errors through `errno`.  The value space from `<errno.h>` is also
  used for communicating errors within the library through return values.  The
  library itself shall not assign to `errno` except in situations where doing
  so is necessary to preserve an intermediate failure across invocation of
  functions that may change the value of `errno`.

* AV-18 (`offsetof` from `<cstddef>`) is permited where the latest GNUC release
  provides support (conditional in C++17).

* AV-22 (`<cstdio>`) applies subject to AV-6.  Deviation is generally granted
  for diagnostic methods and example applications.

### 4.6 Pre-processing Directives

* AV-26 (only `#ifndef/#define/#endif/#include`) is extended to support use of
  `#if` and `#pragma once`.

* AV-27 (multi-inclusion protection) is relaxed to allow use of:

      #pragma once

  within (not as a substitute for) `#ifndef/#define/#endif` protections.

* AV-28 (restricted pre-processor directives) may be relaxed to support feature
  testing.  In these cases boolean conditions shall be expressed as:

      #if (CONDITION - 0)
      #endif

  rather than:

      #ifdef CONDITION
      #endif

  as the former allows for explicit disabling of the condition by assigning a
  preprocess value that is interpreted as `false`.

### 4.7 Header Files

* AV-33 (angle-bracket include) shall be relaxed to allow `#include "file.h"`
  in implementation files where `file.h` is co-located with the implementation
  file and is not part of the public API.

#### Additional Expectations

* Include files shall be grouped into sections by increasing specificity of
  origin, e.g.:
  + headers defined in the C or POSIX standard
  + headers defined in the C++ standard
  + headers defined in externally referenced projects
  + headers defined within the project
  + headers within the implementation source directory

  Sections shall be separated by one blank line.

* Include files shall be sorted lexicographically within their sections.

* Any include file provided by this package that does not include another file
  provided by this package shall include `<pabigot/common.hpp>` unless it is
  itself included by `<pabigot/common.hpp>`.

### 4.9 Style

* AV-41 (120-char line length) is tightened to 79 characters, because this
  eliminates wrap in the 80-col terminals I still use.

  A further exception is made for URIs, e.g. in documentation comments.

* AV-43 (avoid tabs) is upgraded to "will".  Tabs are forbidden in all text
  files, whether source or documentation.

* AV-53 (header.h) is replaced.  Header files *will* always have a file
  extension of `.hpp`.

* AV-54 (impl.cpp) is replaced.  Implementations files *will* always have a
  file extension of `.cc`.

* AV-57 (public/protected/private) may be relaxed to allow the public,
  protected, section sequence to be repeated, *if* it turns out this is needed
  to express visibility in in-class function and variable definitions.

* AV-60 and AV-61 (brace placement) are overruled.

  Opening braces for type declarations and function definitions including
  lambdas shall appear alone on a line, indented at the level of the
  declaration.  Only when a function body is empty may its ending brace appear
  on the line with the opening brace:

      struct s
      {
         uint8_t member;
      };

      int fn (s * sp)
      {
        return sp->member;
      }

      ctor (int field) :
        field{field}
      { }

      auto fn = [](int c)
      {
        return c+1;
      }

  Opening braces for control constructs and namespaces shall appear on the same
  line as the keyword.  Closing braces for `else` shall appear on the same line
  as `else`, and shall indent at the level of the opening `if`.

      namespace {
        unsigned int flags;
      } // ns anonymous

      if (test) {
        // something
      } else {
        // otherwise
      }

### 4.10 Classes

* AV-67 (private data members) is relaxed for data members in a class where a
  subclass is expected to maintain the member value.

* AV-68 (exclude unused implicit definitions) shall be implemented through a
  *public* declaration with the `= delete;` function body .

* AV-76 and AV-77 (copy constructor/assignment) are extended to move
  constructor and assignment.

### 4.11 Namespaces

* AV-99 (namespace depth) is relaxed to three levels deep because the entire
  project is encapsulated in a namespace, and features in their own namespaces
  may require at least one level of internal namespace.

### 4.13 Functions

* AV-113 (single function exit point) may be relaxed to allow early return when
  input arguments fail validation.

### 4.15 Declarations and Definitions

* AV-137 (static file-scope declarations) is replaced by use of an anonymous
  namespace for file-scope declarations.

### 4.18 Constants

* AV-150 (upper-case hex) is accepted, but the prefix *will* be lower case.
  I.e. `0xABC` rather than `0XABC`.

### 4.20 Unions and Bit Fields

* AV-153 (no unions) is relaxed for the purposes of supporting
  alignment-constrained type aliasing and tagged variant data.

* AV-156 (no unnamed tags) is relaxed to allow unnamed unions within a struct
  or class.

### 4.21 Operators

* AV-163 (no unsigned arithmetic) is rejected as unnecessary given that
  overflow/underflow is well-defined and AV-162 disallows the cases where
  things go wrong.

### 4.22 Pointers & References

* AV-175 (null comparison) is overridden.  Use `nullptr` instead of `0`.

### 4.23 Type Conversions

* AV-180 (no implicit conversions) is relaxed where contextual conversion
  ([implicit explicit cast][i-e-cast]) is required.  We have `explicit operator
  bool()` and should be able to use it.

  Use of the idiom `!!expr` is allowed to force exactly this sort of contextual
  conversion to `bool` in cases where the contextual requirement is not
  obvious.

  Use of the idiom `!!expr` to convert a non-boolean to the value zero or one
  is not allowed.  Use `(expr ? 1 : 0)`.

### 4.24 Flow Control Structures

* AV-188 and AV-189 (no labels/goto) is explicitly overridden in support of
  AV-113.  A jump to the exit point is preferred over insanely complex
  conditional structures.

* AV-190 and AV-191 (no continue or break) may be ignored.

* AV-193 (no fall-thru) is replaced by a requirement that the [fallthrough
  attribute](https://en.cppreference.com/w/cpp/language/attributes/fallthrough)
  be used:

      case 'A':
        /* stuff goes here */
        [[fallthrough]]
      case 'B':

### 4.25 Expressions

* AV-203 (no overflow/underflow) is relaxed for the case of unsigned
  arithmetic.

### 4.28 Portable Code

* AV-209 (no generic basic types) is relaxed to allow use of `[unsigned] int`
  to hold ordinals.

* AV-212 (underflow/overflow behavior) is, again, well-defined for unsigned
  arithmetic and may be depended on.

* AV-215 (no pointer arithmetic) is relaxed in situations where the API is
  simplified by using pointers within a multi-element sequence.  Not everything
  can be wrapped in a container.

### Style Features Unaddressed by JSF-AV

* The `noexcept` qualifier should be used wherever appropriate.

* Use `int* p` over `int *p`, following the logic in the
  [FAQ](https://isocpp.org/wiki/faq/style-and-techniques) that argues C++ is
  type-oriented more than expression oriented (and to be consistent with
  `int& r` since `int &r` is just weird).

* A declaration or definition of a function/method shall have a space between
  the name and the opening parenthesis of the parameter list.  All invocations
  of the function/method shall join the name to the opening parenthesis without
  a space.

  *Rationale:* Supports searching for uses versus definitions of a function.

* A function definition that is not also a declaration shall separate the
  specifiers from the declarator id so the latter begins a line:

      int function (int arg); // declaration

      int
      function (arg) // definition
      {
        //....
      }

  *Rationale:* Supports finding the definition by searching for `^name`.

[i-e-cast]: https://stackoverflow.com/questions/6242768/is-the-safe-bool-idiom-obsolete-in-c11
[jsf-av]: http://www.stroustrup.com/JSF-AV-rules.pdf "JSF-AV"
[linux-spdx]: https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/Documentation/process/license-rules.rst
[spdx-text]: https://github.com/spdx/license-list-data/tree/master/text
[semver]: https://semver.org/
[Changelog]: https://keepachangelog.com/
