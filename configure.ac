#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.58])
AC_INIT([arg3net], [0.2.0], [c0der78@gmail.com], [arg3net], [http://github.com/c0der78/arg3net/archive/0.2.0.tar.gz])
AC_CONFIG_SRCDIR([src/socket.cpp])
AC_CONFIG_HEADERS([src/config.h])

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE([enable])

LT_INIT

: ${CXXFLAGS=""}

AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX([clang++ g++])

if test "x$CXX" == "xclang++"; then
	AC_SUBST([AM_CPPFLAGS], ["-std=c++11 -Wno-error=deprecated-declarations -stdlib=libc++ -Wall -Werror"])
	AC_SUBST([AM_LDFLAGS], [])
else
	AC_SUBST([AM_CPPFLAGS], ["-std=c++11 -Wall -Werror"])
	AC_SUBST([AM_LDFLAGS], ["-pthread"])
fi

AC_ARG_WITH([curl],
            [AS_HELP_STRING([--without-curl],
            [use the curl library for http transport])],
            [with_curl=no],
            [with_curl=yes])

AC_ARG_WITH([ssl],
	    [AS_HELP_STRING([--with-ssl],
	    [use ssl library for https transport])],
	    [with_ssl=yes],
        [with_ssl=no])

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

AS_IF([test "x$with_curl" != "xno"],
	AC_CHECK_LIB(curl, curl_easy_init))

AS_IF([test "x$with_ssl" != "xno"],
	AC_CHECK_LIB(ssl, SSL_library_init))

AC_CHECK_LIB(uriparser, uriParseUriA)

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([gethostbyname gettimeofday inet_ntoa memset select socket strerror])

AC_CONFIG_FILES([arg3net.pc Makefile src/Makefile tests/Makefile])
AC_OUTPUT
