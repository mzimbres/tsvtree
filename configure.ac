AC_PREREQ([2.69])
AC_INIT([tsvtree], [1.0.5], [mzimbres@gmail.com])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/tsvtree.cpp])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall foreign])

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S

AX_BOOST_BASE([1.70],, AC_MSG_ERROR[Boost not found])
AX_BOOST_PROGRAM_OPTIONS

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT64_T
AC_CHECK_TYPES([ptrdiff_t])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
