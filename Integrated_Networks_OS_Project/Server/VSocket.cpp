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

 #include <sys/socket.h>
 #include <arpa/inet.h>		// ntohs, htons
 #include <stdexcept>            // runtime_error
 #include <cstring>		// memset
 #include <netdb.h>		// getaddrinfo, freeaddrinfo
 #include <unistd.h>		// close
 #include <iostream>
 /*
 #include <cstddef>
 #include <cstdio>
 
 //#include <sys/types.h>
 */
#include "VSocket.h"


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
 void VSocket::BuildSocket( char t, bool IPv6 ){
 
   int type = SOCK_STREAM; 
   if(t == 's'){
     // Stream socket type
     type = SOCK_STREAM;
   }
   else if(t == 'd'){
     // Datagram socket type
     type = SOCK_DGRAM;
   }
   else{
     throw std::runtime_error( "VSocket::BuildSocket, invalid socket type" );
   }
 
   if(IPv6){
     // Domain IPv6
     std::cout << "IPv6" << std::endl;
     this->family = AF_INET6;
   }
   else if(!IPv6){
     // Domain IPv4 
     std::cout << "IPv4" << std::endl;
     this->family = AF_INET;
   }
   else{
      throw std::runtime_error( "VSocket::BuildSocket, invalid socket domain" );
   }
   // Create socket 
   this->idSocket = socket(this->family, type, 0 );
   if(this->idSocket == -1){
     throw std::runtime_error( "VSocket::BuildSocket, socket" );
   }
   
   std::cout << "Socket created "<< std::endl;
 
 }

 void VSocket::BuildSocketWthId(int id) {
    this->idSocket = id;
  }



/**
 * Class destructor
 *
**/
VSocket::~VSocket() {

   this->Close();
 
 }
 
 
 /**
  * Close method
  *    use Unix close system call (once opened a socket is managed like a file in Unix)
  *
 **/
 void VSocket::Close(){
 
   std::cout << "Closing socket connection" << std::endl;
   close(this->idSocket);
 
 }
 
 
 /**
  * EstablishConnection method
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dot notation, example "10.84.166.62"
  * @param      int port: process address, example 80
  *
 **/
 int VSocket::EstablishConnection( const char * hostip, int port ) {
  
  int connection = -1;

  if(this->family == AF_INET){
    // IPv4
    struct sockaddr_in  host4;
       // Initialize structure 
   memset( (char *) &host4, 0, sizeof( host4 ) );
   // Set structure values for domain
   host4.sin_family = this->family;
   // Convert the host address from dot notation to binary form
   connection = inet_pton( this->family, hostip, &host4.sin_addr );
   if ( connection == -1 ) {
     throw(std::runtime_error( "VSocket::DoConnect, inet_pton"));
   }
   // Set structure values for port
   host4.sin_port = htons( port );
   // Connect to the host
   connection = connect( this->idSocket, (sockaddr *) &host4, sizeof( host4 ) );
   if ( connection == -1 ) {
     throw( std::runtime_error( "VSocket::DoConnect, connect IPV4" ));
   }
  }
  else if(this->family == AF_INET6){
    // IPv6
    struct sockaddr_in6 host6;
       // Initialize structure
    memset( (char *) &host6, 0, sizeof( host6 ) );
    // Set structure values for domain
    host6.sin6_family = this->family;
    // Convert the host address from dot notation to binary form
    connection = inet_pton( this->family, hostip, &host6.sin6_addr );
    if ( connection == -1 ) {
      throw(std::runtime_error( "VSocket::DoConnect, inet_pton"));
    }
    // Set structure values for port
    host6.sin6_port = htons( port );
    // Connect to the host
    connection = connect( this->idSocket, (sockaddr *) &host6, sizeof( host6 ) );
    if ( connection == -1 ) {
      throw( std::runtime_error( "VSocket::DoConnect, connect IPV6" ));
    }
  }
  else{
    throw std::runtime_error( "VSocket::EstablishConnection, invalid socket domain" );
  }
  return connection;

 }
 
 
 /**
  * EstablishConnection method
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dns notation, example "os.ecci.ucr.ac.cr"
  * @param      char * service: process address, example "http"
  *
 **/
 int VSocket::EstablishConnection( const char *host, const char *service ) {
   int st = -1;  
   struct addrinfo hints, *result, *rp;
   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = this->family;    /* Allow IPv4 or IPv6 */
   hints.ai_socktype = SOCK_STREAM; /* Stream socket */
   hints.ai_flags = 0;
   hints.ai_protocol = 0;          /* Any protocol */
 
   std::cout << "Establishing connection" << std::endl;
   st = getaddrinfo( host, service, &hints, &result );
 
   if (st != 0) {
       std::cerr << "Error in getaddrinfo: " << gai_strerror(st) << std::endl;
       return -1;
   }
   bool connected = false;
   for (rp = result; rp; rp = rp->ai_next) {
     char host[NI_MAXHOST], service[NI_MAXSERV];
     getnameinfo(rp->ai_addr, rp->ai_addrlen, host, sizeof(host), service, sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV);
     std::cout << "Intentando conectar a " << host << " en el puerto " << service << std::endl;
 
     st = connect(idSocket, rp->ai_addr, rp->ai_addrlen);
     if (st == 0) {
         connected = true;
         break;
       }
   }
   freeaddrinfo( result );
   if (!connected) {
     std::cerr << "Error: Unable to connect to any address" << std::endl;
     return -1;  
   }
   std::cout << "Connection established successfully!" << std::endl;
     return 0;
 
 }


 /**
   * Bind method
   *    use "bind" Unix system call (man 3 bind) (server mode)
   *
   * @param      int port: bind a unamed socket to a port defined in sockaddr structure
   *
   *  Links the calling process to a service at port
   *
  **/
 int VSocket::Bind( int port ) {
   int st = -1;

    // Set socket options
   int opt = 1;
    // Set the socket option to allow address reuse
   if (setsockopt(this->idSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
       throw std::runtime_error("VSocket::Bind - setsockopt(SO_REUSEADDR) failed");
   }

  if(this->family == AF_INET){
    // IPv4
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = this->family;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    st = bind(this->idSocket, (struct sockaddr *)&addr, sizeof(addr));
    if( -1 == st ) {
     throw std::runtime_error( "VSocket::Bind" );
     }
  }
  else if(this->family == AF_INET6){
    // IPv6
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = this->family;
    addr.sin6_port = htons(port);
    addr.sin6_addr = in6addr_any;
    st = bind(this->idSocket, (struct sockaddr *)&addr, sizeof(addr)); 
    if ( -1 == st ) {
     throw std::runtime_error( "VSocket::Bind" );
  }
  }
  else{
    throw std::runtime_error( "VSocket::Bind, invalid socket domain" );   
  }
   return st;

}


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
size_t VSocket::sendTo( const void * buffer, size_t size, void * addr ) {
   int st = -1;

  socklen_t addr_len;
  if (this->family == AF_INET) {
     addr_len = sizeof(struct sockaddr_in);
  } else if (this->family == AF_INET6) {
     addr_len = sizeof(struct sockaddr_in6);
  } else {
     throw std::runtime_error("VSocket::recvFrom: Invalid socket family");
  }
  st = sendto(this->idSocket, buffer, size, 0, (struct sockaddr *)addr, addr_len);
  if ( -1 == st ) {
     throw std::runtime_error( "VSocket::sendTo" );
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
size_t VSocket::recvFrom( void * buffer, size_t size, void * addr ) {
   int st = -1;

  socklen_t addr_len;
  if (this->family == AF_INET) {
     addr_len = sizeof(struct sockaddr_in);
  } else if (this->family == AF_INET6) {
     addr_len = sizeof(struct sockaddr_in6);
  } else {
     throw std::runtime_error("VSocket::recvFrom: Invalid socket family");
  }
  st = recvfrom(this->idSocket, buffer, size, 0, (struct sockaddr *)addr, &addr_len);  
  if ( -1 == st ) {
     throw std::runtime_error( "VSocket::recvFrom" );
  }

   return st;

}

 /**
   * MarkPassive method
   *    use "listen" Unix system call (man listen) (server mode)
   *
   * @param      int backlog: defines the maximum length to which the queue of pending connections for this socket may grow
   *
   *  Establish socket queue length
   *
  **/
 int VSocket::MarkPassive( int backlog ) {
    int st = -1;

    std::cout << "Escuchando conexiones" << std::endl;
    st = listen(this->idSocket, backlog);
    if(st == -1){
        throw std::runtime_error( "VSocket::MarkPassive" );
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
 int VSocket::WaitForConnection( void ) {
    std::cout << "Esperando conexiones" << std::endl;
    // IPv4
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = this->family;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    socklen_t addr_len;
    addr_len = sizeof(struct sockaddr_in);

    int sockfd = accept(this->idSocket, (struct sockaddr *)&addr, &addr_len);
 

 
    return sockfd;
 
 }
 
 
 /**
   * Shutdown method
   *    use "shutdown" Unix system call (man 3 shutdown) (server mode)
   *
   *
   *  cause all or part of a full-duplex connection on the socket associated with the file descriptor socket to be shut down
   *
  **/
 int VSocket::Shutdown( int mode ) {
    int st = -1;

    st = shutdown(this->idSocket, mode);
    if ( -1 == st ) {
    throw std::runtime_error( "VSocket::Shutdown" );
    }
    std::cout << "Socket shutdown" << std::endl;
 
    return st;
 
 }
 