AC_PREREQ(2.61)
AC_INIT([repsheet], [1.0.0], [aaron@aaronbedra.com])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([-Wall foreign subdir-objects])
AC_CONFIG_MACRO_DIR([m4])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL

#AC_CHECK_LIB([apr-1], [apr_pool_pre_cleanup_register], [], [AC_MSG_ERROR([apr_pool_pre_cleanup_register was not found in libapr-1])])

AC_CHECK_FILE([build/apache/include/httpd.h], [APACHE_INCLUDE_DIR="build/apache/include"], [
  AC_MSG_ERROR([Could not locate Apache include directory])
])
AC_SUBST([APACHE_INCLUDE_DIR])

AC_CHECK_FILE([build/apache/bin/apxs], [APXS="build/apache/bin/apxs"], [
  AC_MSG_ERROR([Could not locate Apache apxs binary])
])
AC_SUBST([APXS])

PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    test/Makefile
])

AC_OUTPUT
