#include <mpi.h>

/* If we use libmpichcxx.dylib, we need to compile
 * this package as c++ project because libmpichcxx.dylib
 * need c++ runtimes.
 * this file makes dependency for c++ runtime.
 */
void mpicxx_dummy() {}
