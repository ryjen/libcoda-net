#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([arg3net], [0.2], [c0der78@gmail.com])
AC_CONFIG_SRCDIR([src/socket.cpp])
AC_CONFIG_HEADERS([src/config.h])
AM_INIT_AUTOMAKE
LT_INIT

: ${CXXFLAGS=""}

AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX([clang++ g++])

if test "x$CXX" == "xclang++"; then
	AC_SUBST([AM_CPPFLAGS], ["-std=c++11 -stdlib=libc++ -Wall -Werror"])
else
	AC_SUBST([AM_CPPFLAGS], ["-std=c++11 -Wall -Werror"])
	AC_SUBST([AM_LDFLAGS], ["-pthread"])
fi

AC_ARG_WITH([curl],
            [AS_HELP_STRING([--without-curl],
              [use the curl library for http transport])],
            [without_curl=true],
            [])

m4_include([m4/gcov.m4])
AC_TDD_GCOV
AC_SUBST(COVERAGE_CFLAGS)
AC_SUBST(COVERAGE_CXXFLAGS)
AC_SUBST(COVERAGE_LDFLAGS)

AC_CHECK_PROG(GIT_CHECK,git,yes)
if test x"$GIT_CHECK" != x"yes" ; then
    AC_MSG_ERROR([Required program 'git' not found.])
else
    git submodule update --init --recursive
fi

# Checks for libraries.

if test "x$without_curl" != xtrue; then
	AC_CHECK_LIB(curl, curl_easy_init)
fi
AC_CHECK_LIB(ssl, SSL_library_init)

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([gethostbyname gettimeofday inet_ntoa memset select socket strerror])

AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile])
AC_OUTPUT
