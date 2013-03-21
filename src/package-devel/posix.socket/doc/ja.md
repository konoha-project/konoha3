posix.socket-パッケージの1行説明文
====================

### int System.accept(int socket, Map remoteInfo);
### int System.accept(int socket, SockAddr remoteInfo);
### int System.bind(int socket, String srcIP, int srcPort, int family);
### int System.close(int fd);
### int System.connect(int socket, String dstIP, int dstPort, int family);
### int System.listen(int socket, int backlog);
### String System.getsockname(int socket);
### int System.getsockopt(int socket, int option);
### int System.setsockopt(int socket, int option, int value);
### Map System.getpeername(int socket);
### int System.recvfrom(int socket, byte[] buffer, int flags, Map remoteInfo);
### int System.shutdown(int socket, int how);
### int System.sockatmark(int socket);
### int System.socket(int family, int type, int protocol);
### int System.socketpair(int family, int type, int protocol, int[] pairCSock);
