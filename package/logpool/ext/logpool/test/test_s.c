#include "io.h"

int main(int argc, char **argv)
{
    extern struct io_api server_api;
    struct io *io = io_open("0.0.0.0", 14801, IO_MODE_READ|IO_MODE_WRITE, &server_api);
    return io_dispatch(io);
}
