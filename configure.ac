AC_PREREQ([2.71])
AC_INIT([PIC-FPGA], [1.0])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src/hw_if.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries and header files.
have_pthread=no
have_m=no
have_wiringpi=no

AC_SEARCH_LIBS([pow], [m], [have_m=yes])
if test "x${have_m}" = xyes; then
    AC_CHECK_HEADERS([math.h], [], [have_m=no])
fi

if test "x${have_m}" = xno; then
    AC_MSG_ERROR([
    ----------------------------------------
    Libm library and / or header files were
    not found, stopping...
    Check 'config.log' for more information.
    ----------------------------------------])
fi

AC_SEARCH_LIBS([pthread_create], [pthread], [have_pthread=yes])
if test "x${have_pthread}" = xyes; then
    AC_CHECK_HEADERS([pthread.h], [], [have_pthread=no])
fi

if test "x${have_pthread}" = xno; then
    AC_MSG_ERROR([
    ----------------------------------------
    Pthread library and / or header files
    were not found, stopping...
    Check 'config.log' for more information.
    ----------------------------------------])
fi
AC_SEARCH_LIBS([wiringPiSetup], [wiringPi], [have_wiringpi=yes])
if test "x${have_wiringpi}" = xyes; then
    AC_CHECK_HEADERS([wiringPi.h wiringSerial.h], [], [have_wiringpi=no])
fi

if test "x${have_wiringpi}" = xno; then
    AC_MSG_ERROR([
    ----------------------------------------
    Wiringpi library and / or header files
    were not found, stopping...
    Check 'config.log' for more information.
    ----------------------------------------])
fi

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
