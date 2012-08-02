#ifndef LOGPOOL_CONFIG_H_
#define LOGPOOL_CONFIG_H_

#cmakedefine HAVE_LIBMEMCACHED ${HAVE_LIBMEMCACHED}
#cmakedefine SIZEOF_VOIDP  ${SIZEOF_VOIDP}
#cmakedefine SIZEOF_LONG   ${SIZEOF_LONG}
#cmakedefine SIZEOF_INT    ${SIZEOF_INT}
#cmakedefine SIZEOF_FLOAT  ${SIZEOF_FLOAT}
#cmakedefine SIZEOF_DOUBLE ${SIZEOF_DOUBLE}
#cmakedefine LOGPOOL_USE_LLVM    ${LOGPOOL_USE_LLVM}
#cmakedefine LOGPOOL_USE_LLVM_31 ${LOGPOOL_USE_LLVM_31}
#cmakedefine HAVE_CLANG          ${HAVE_CLANG}

#endif /* end of include guard */
