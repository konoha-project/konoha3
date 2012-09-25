/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H 1

/* Define to 1 if you have the <iconv.h> header file. */
#cmakedefine HAVE_ICONV_H 1

/* Define to 1 if you have the <syslog.h> header file. */
#cmakedefine HAVE_SYSLOG_H 1

/* Define to 1 if you have the `dl' library (-ldl). */
#cmakedefine HAVE_LIBDL 1

/* Define to 1 if you have the `m' library (-lm). */
#cmakedefine HAVE_LIBM 1

/* arch type */
#cmakedefine K_CONFIG_ARCH "${K_CONFIG_ARCH}"

/* cpu type */
#cmakedefine K_CONFIG_CPU "${K_CONFIG_CPU}"

/* os type */
#cmakedefine K_CONFIG_OS "${K_CONFIG_OS}"

/* build revision */
#cmakedefine K_REVISION  ${K_REVISION}

/* no debug opt */
#cmakedefine K_NODEBUG  ${K_NODEBUG}

/* dll extension */
#cmakedefine K_OSDLLEXT "${K_OSDLLEXT}"

/* Difining konoha library prefix with default value */
#cmakedefine K_PREFIX "${CMAKE_INSTALL_PREFIX}"

/* Define to the address where bug reports for this package should be sent. */
#cmakedefine PACKAGE_BUGREPORT ${PACKAGE_BUGREPORT}

/* Define to the full name of this package. */
#cmakedefine PACKAGE_NAME "${PACKAGE_NAME}"

/* Define to the full name and version of this package. */
#cmakedefine PACKAGE_STRING "${PACKAGE_STRING}"

/* Define to the home page for this package. */
#cmakedefine PACKAGE_URL "${PACKAGE_URL}"

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION "${PACKAGE_VERSION}"

/* Define to the full path of this build dir. */
#cmakedefine PACKAGE_BUILD_DIR "${PACKAGE_BUILD_DIR}"

/* The size of `int', as computed by sizeof. */
#cmakedefine SIZEOF_INT ${SIZEOF_INT}

/* The size of `long', as computed by sizeof. */
#cmakedefine SIZEOF_LONG ${SIZEOF_LONG}

/* The size of `void*', as computed by sizeof. */
#cmakedefine SIZEOF_VOIDP ${SIZEOF_VOIDP}

/* Define to 1 if you have the `posix_memalign' function. */
#cmakedefine HAVE_POSIX_MEMALIGN 1

/* Define to 1 if you have the `memalign' function. */
#cmakedefine HAVE_MEMALIGN 1

/* Define to 1 if you have the `__builtin_ctzl' function. */
#cmakedefine HAVE_BUILTIN_CTZL 1

/* Define to 1 if you have the `bzero' function. */
#cmakedefine HAVE_BZERO 1
