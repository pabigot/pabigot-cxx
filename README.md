# pabigot-cxx

This package comprises utilities that assist in writing code targeted to C++17
and later language versions.  It has some overlap with Boost in terms of
functionality, but avoids Boost's tightly-coupled dependencies and need to
support older language versions.

Homepage: https://github.com/pabigot/pabigot-cxx
Documentation: https://pabigot.github.io/pabigot-cxx/index.html

## Notes

Configuration is with [meson](http://mesonbuild.com/index.html):

    meson build

Useful options:

    -Db_coverage=true

Then:

    ninja -C build test
    ninja -C build coverage-html

and review the content in `build/meson-logs/coveragereport`.
