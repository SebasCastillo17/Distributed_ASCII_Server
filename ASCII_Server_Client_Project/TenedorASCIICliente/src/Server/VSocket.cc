/**
 *  Universidad de Costa Rica
 *  ECCI
 *  CI0123 Proyecto integrador de redes y sistemas operativos
 *  2025-i
 *  Grupos: 1 y 3
 *
 ****** VSocket base class implementation
 *
 * (Fedora version)
 *
 **/

#include <arpa/inet.h> // ntohs, htons
#include <cstring>     // memset
#include <net/if.h>    // if_nametoindex
#include <netdb.h>     // getaddrinfo, freeaddrinfo
#include <stdexcept>   // runtime_error
#include <sys/socket.h>
#include <unistd.h> // close

#include <cstddef>

#include "../../include/Server/VSocket.h"

/**
 *  Class creator (constructor)
 *     use Unix socket system call
 *
 *  @param     char t: socket type to define
 *     's' for stream
 *     'd' for datagram
 *  @param     bool ipv6: if we need a IPv6 socket
 *
 **/

void VSocket::BuildSocket(char t, bool IPv6) {
  int Type = 'd' == t ? SOCK_DGRAM : SOCK_STREAM;
  int Domain = IPv6 ? AF_INET6 : AF_INET;
  this->IPv6 = IPv6;
  this->idSocket = socket(Domain, Type, 0);
  if (-1 == this->idSocket) {
    throw std::runtime_error("VSocket::BuildSocket, (can't create the socket)");
  }
}

/**
 * Class destructor
 *
 **/
VSocket::~VSocket() { this->Close(); }

/**
 * Close method
 *    use Unix close system call (once opened a socket is managed like a file in
 *Unix)
 *
 **/
void VSocket::Close() {
  int st = close(this->idSocket);

  if (-1 == st) {
    throw std::runtime_error("VSocket::Close()");
  }
}

/**
 * EstablishConnection method
 *   use "connect" Unix system call
 *
 * @param char * host: host address in dot notation, example "10.84.166.62"
 * @param int port: process address, example 80
 *
 **/
int VSocket::EstablishConnection(const char *hostip, int port) {
  int st = -1;
  if (!this->IPv6) {
    this->port = port;
    sockaddr_in v4;
    memset((char *)&v4, 0, sizeof(v4));
    v4.sin_family = AF_INET;
    st = inet_pton(AF_INET, hostip, &v4.sin_addr);
    if (0 >= st) {
      throw std::runtime_error("VSocket::EstablishConnection, inet_pton");
    }
    v4.sin_port = htons(port);
    st = connect(this->idSocket, (sockaddr *)&v4, sizeof(v4));

    if (-1 == st) {
      throw std::runtime_error("VSocket::EstablishConnection");
    }
    return st;
  }
  this->port = port;
  struct sockaddr_in6 host6;
  memset(&host6, 0, sizeof(host6));
  host6.sin6_family = AF_INET6;
  // se necesita formatear la dirección ipv6 para que esta funcione
  char host_copy[256];
  strncpy(host_copy, hostip, sizeof(host_copy));
  // se deja la dirección hasta que se encuentra un '%'
  // y se copia la interfaz en la variable zone
  char *interface = strchr(host_copy, '%');
  if (interface != NULL) {
    *interface = '\0';
    interface++;
  }

  st = inet_pton(AF_INET6, host_copy, &host6.sin6_addr);
  if (st <= 0) {
    if (st == 0) {
      throw std::runtime_error(
          "VSocket::EstablishConnection, inet_pton: Invalid address format");
    } else {
      throw std::runtime_error(
          "VSocket::EstablishConnection, inet_pton: Conversion error");
    }
  }

  if (interface != NULL) {
    // se convierte la interfaz
    host6.sin6_scope_id = if_nametoindex(interface);
    if (host6.sin6_scope_id == 0) {
      throw std::runtime_error(
          "VSocket::EstablishConnection, invalid interface");
    }
  }

  host6.sin6_port = htons(port);

  st = connect(this->idSocket, (struct sockaddr *)&host6, sizeof(host6));
  if (st == -1) {
    throw std::runtime_error("VSocket::EstablishConnection, connect");
  }
  return st;
}

/**
 * EstablishConnection method
 *   use "connect" Unix system call
 *
 * @param      char * host: host address in dns notation, example
 *"os.ecci.ucr.ac.cr"
 * @param      char * service: process address, example "http"
 *
 **/
int VSocket::EstablishConnection(const char *host, const char *service) {
  struct addrinfo hints, *res, *rp;
  int st = -1;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  st = getaddrinfo(host, service, &hints, &res);
  if (st != 0) {
    throw std::runtime_error("VSocket::EstablishConnection, getaddrinfo");
  }
  for (rp = res; rp; rp = rp->ai_next) {
    st = connect(this->idSocket, rp->ai_addr, rp->ai_addrlen);
    if (0 == st)
      break;
  }
  freeaddrinfo(res);
  if (st == -1) {
    throw std::runtime_error("VSocket::EstablishConnection");
  }
  return st;
}

/**
 * Bind method
 *    use "bind" Unix system call (man 3 bind) (server mode)
 *
 * @param      int port: bind a unamed socket to a port defined in sockaddr
 *structure
 *
 *  Links the calling process to a service at port
 *
 **/
int VSocket::Bind(int port) {
  int st = -1;
  if (!this->IPv6) {
    struct sockaddr_in host4;

    host4.sin_family = AF_INET;
    host4.sin_addr.s_addr = htonl(INADDR_ANY);
    host4.sin_port = htons(port);
    memset(host4.sin_zero, '\0', sizeof(host4.sin_zero));
    st = bind(this->idSocket, (sockaddr *)&host4, sizeof(sockaddr_in));
    if (st == -1) {
      throw std::runtime_error("VSocket::Bind");
    }
    return st;
  }
  struct sockaddr_in6 host6;
  host6.sin6_family = AF_INET6;
  host6.sin6_port = htons(port);
  host6.sin6_addr = in6addr_any;
  st = bind(this->idSocket, (sockaddr *)&host6, sizeof(sockaddr_in6));
  if (st == -1) {
    throw std::runtime_error("VSocket::Bind");
  }
  return st;
}

/**
 * MarkPassive method
 *    use "listen" Unix system call (man listen) (server mode)
 *
 * @param      int backlog: defines the maximum length to which the queue of
 *pending connections for this socket may grow
 *
 *  Establish socket queue length
 *
 **/
int VSocket::MarkPassive(int backlog) {
  int st = listen(this->idSocket, backlog);

  if (st == -1) {
    throw std::runtime_error("VSocket::MarkPassive");
  }

  return st;
}

/**
 * WaitForConnection method
 *    use "accept" Unix system call (man 3 accept) (server mode)
 *
 *
 *  Waits for a peer connections, return a sockfd of the connecting peer
 *
 **/
int VSocket::WaitForConnection(void) {
  struct sockaddr_storage their_addr;
  socklen_t addr_size = sizeof their_addr;

  int new_socket =
      accept(this->idSocket, (struct sockaddr *)&their_addr, &addr_size);

  if (new_socket == -1) {
    throw std::runtime_error("Socket::WaitForConnection() - accept failed");
  }
  return new_socket;
}

/**
 * Shutdown method
 *    use "shutdown" Unix system call (man 3 shutdown) (server mode)
 *
 *
 *  cause all or part of a full-duplex connection on the socket associated with
 *the file descriptor socket to be shut down
 *
 **/
int VSocket::Shutdown(int mode) {
  int st = shutdown(this->idSocket, mode);

  if (st == -1) {
    throw std::runtime_error("VSocket::Shutdown");
  }

  return st;
}

// UDP methods 2025

/**
 *  sendTo method
 *
 *  @param	const void * buffer: data to send
 *  @param	size_t size data size to send
 *  @param	void * addr address to send data
 *
 *  Send data to another network point (addr) without connection (Datagram)
 *
 **/
size_t VSocket::sendTo(const void *buffer, size_t size, void *addr) {
  int st = -1;
  socklen_t addr_len = this->IPv6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in);
  st = sendto(this->idSocket, buffer, size, 0, (sockaddr *)addr, addr_len);
  if (st == -1) {
    throw std::runtime_error("VSocket::sendTo");
  }
  return st;
}

/**
 *  recvFrom method
 *
 *  @param	const void * buffer: data to send
 *  @param	size_t size data size to send
 *  @param	void * addr address to receive from data
 *
 *  @return	size_t bytes received
 *
 *  Receive data from another network point (addr) without connection (Datagram)
 *
 **/
size_t VSocket::recvFrom(void *buffer, size_t size, void *addr) {
  int st = -1;
  socklen_t addr_len = !this->IPv6 ? sizeof(sockaddr) : sizeof(sockaddr_in6);
  st = recvfrom(this->idSocket, buffer, size, 0, (sockaddr *)addr, &addr_len);
  if (st == -1) {
    throw std::runtime_error("VSocket::recvFrom");
  }
  return st;
}

int VSocket::setsockopt(int level, int optname, const void *optval, socklen_t optlen) {
    return ::setsockopt(idSocket, level, optname, optval, optlen);
}

