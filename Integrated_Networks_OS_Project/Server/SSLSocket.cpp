/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2025-i
  *  Grupos: 1 y 3
  *
  *  Socket class implementation
  *
  * (Fedora version)
  *
 **/
 
// SSL includes
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <iostream>

#include <stdexcept>

#include "SSLSocket.h"
#include "Socket.h"

/**
  *  Class constructor
  *     use base class
  *
  *  @param     char t: socket type to define
  *     's' for stream
  *     'd' for datagram
  *  @param     bool ipv6: if we need a IPv6 socket
  *
 **/
SSLSocket::SSLSocket( bool IPv6 , bool ClientContext) {

   this->BuildSocket( 's', IPv6 );


   this->SSLContext = nullptr;
   this->SSLStruct = nullptr;

   if (ClientContext) {
      this->Init(); // Initializes to client context
   }
}

SSLSocket::SSLSocket( char t , bool IPv6 , bool ClientContext) {

   this->BuildSocket( t, IPv6 );

   this->SSLContext = nullptr;
   this->SSLStruct = nullptr;

   if (ClientContext) {
      this->Init();
   }
}


/**
  *  Class constructor
  *
  *  @param     int id: socket descriptor
  *
 **/
SSLSocket::SSLSocket( int id ) {

   this->BuildSocketWthId( id );

}


/**
  * Class destructor
  *
 **/
SSLSocket::~SSLSocket() {
   if (this->SSLContext) {
      SSL_CTX_free(reinterpret_cast<SSL_CTX *>(this->SSLContext)); // Liberar contexto SSL
   }
   if (this->SSLStruct) {
      SSL_free(reinterpret_cast<SSL *>(this->SSLStruct)); // Liberar estructura SSL
   }
   this->Close(); // Cerrar el socket
}

/**
 *  SSLInit
 *     use SSL_new with a defined context
 *
 *  Create a SSL object
 *
 **/
void SSLSocket::Init() {
   this->InitClientContext(); // Crear el contexto SSL
   std::cout << "SSL context created" << std::endl;

   SSL_CTX *context = reinterpret_cast<SSL_CTX *>(this->SSLContext);
   SSL *ssl = SSL_new(context); // Crear la estructura SSL
   if (!ssl) {
      throw std::runtime_error("SSLSocket::Init( bool ): Error al crear SSL");
   }

   this->SSLStruct = reinterpret_cast<void *>(ssl);
}

/**
 *  InitContext
 *     use SSL_library_init, OpenSSL_add_all_algorithms, SSL_load_error_strings, TLS_server_method, SSL_CTX_new
 *
 *  Creates a new SSL server context to start encrypted comunications, this context is stored in class instance
 *
 **/
void SSLSocket::InitClientContext() {
   const SSL_METHOD *method = TLS_client_method();
   std::cout << "SSL client context created" << std::endl;
   if (!method) {
      throw std::runtime_error("SSLSocket::InitContext( bool )");
   }

   SSL_CTX *context = SSL_CTX_new(method); // Crear contexto SSL
   if (!context) {
      throw std::runtime_error("SSLSocket::InitContext( bool )");
   }

   this->SSLContext = reinterpret_cast<void *>(context);
}

void SSLSocket::InitServer(const char * certFileName, const char * keyFileName){
    this->InitServerContext();
    SSL_CTX *context = reinterpret_cast<SSL_CTX *>(this->SSLContext);
    SSL *ssl = SSL_new(context); // Crear la estructura SSL
    if (!ssl) {
       throw std::runtime_error("SSLSocket::Init( bool ): Error al crear SSL");
    }
    this->LoadCertificates(certFileName, keyFileName);
}

void SSLSocket::InitServerContext(){
    const SSL_METHOD *method;
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    method = TLS_server_method();
    SSL_CTX *context = SSL_CTX_new(method); // Crear contexto SSL
    if (!context) {
       throw std::runtime_error("SSLSocket::InitContext( bool )");
    }
 
    this->SSLContext = reinterpret_cast<void *>(context);
    std::cout << "SSL server context created" << std::endl;
}


/**
 *  Load certificates
 *    verify and load certificates
 *
 *  @param	const char * certFileName, file containing certificate
 *  @param	const char * keyFileName, file containing keys
 *
 **/
void SSLSocket::LoadCertificates(const char * certFileName, 
    const char * keyFileName ) {
    SSL_CTX * context = (SSL_CTX*)(this->SSLContext);
   std::cout << "Certificate file: " << certFileName << std::endl;
   std::cout << "Key file: " << keyFileName << std::endl;

   if ( SSL_CTX_use_certificate_file( context, certFileName, SSL_FILETYPE_PEM ) <= 0 ) {	 // set the local certificate from CertFile
      ERR_print_errors_fp( stderr );
      abort();
   }

   if ( SSL_CTX_use_PrivateKey_file( context, keyFileName, SSL_FILETYPE_PEM ) <= 0 ) {	// set the private key from KeyFile (may be the same as CertFile)
      ERR_print_errors_fp( stderr );
      abort();
   }

   if ( ! SSL_CTX_check_private_key( context ) ) {	// verify private key
      ERR_print_errors_fp( stderr );
      abort();
   }
}
 

/**
 *  Connect
 *     use SSL_connect to establish a secure conection
 *
 *  Create a SSL connection
 *
 *  @param	char * hostName, host name
 *  @param	int port, service number
 *
 **/
int SSLSocket::MakeConnection( const char * hostName, int port ) {
   int st = -1;

   st = this->EstablishConnection( hostName, port );
   // Establecer conexion sin SSL primero.
   st = SSL_set_fd((SSL *)this->SSLStruct, this->idSocket);
   if(st == -1){
      throw std::runtime_error("SSLSocket::MakeConnection( Error ), SSL_set_fd");
   }
   st = SSL_connect((SSL *)this->SSLStruct);
   if(st == -1){
      throw std::runtime_error("SSLSocket::MakeConnection( Error ), SSL_connect");
   }
   return st;

   return st;

}


/**
 *  Connect
 *     use SSL_connect to establish a secure conection
 *
 *  Create a SSL connection
 *
 *  @param	char * hostName, host name
 *  @param	char * service, service name
 *
 **/
int SSLSocket::MakeConnection( const char * host, const char * service ) {
   int st = -1;

   st = this->EstablishConnection( host, service );
   // Establecer conexion sin SSL primero.
   st = SSL_set_fd((SSL *)this->SSLStruct, this->idSocket);
   if(st == -1){
      throw std::runtime_error("SSLSocket::MakeConnection( Error ), SSL_set_fd");
   }
   st = SSL_connect((SSL *)this->SSLStruct);
   if(st == -1){
      throw std::runtime_error("SSLSocket::MakeConnection( Error ), SSL_connect");
   }

   return st;

}


/**
  *  Read
  *     use SSL_read to read data from an encrypted channel
  *
  *  @param	void * buffer to store data read
  *  @param	size_t size, buffer's capacity
  *
  *  @return	size_t byte quantity read
  *
  *  Reads data from secure channel
  *
 **/
size_t SSLSocket::Read( void * buffer, size_t size ) {
   int st = -1;
   st = SSL_read((SSL *)this->SSLStruct, buffer, size);;

   if ( -1 == st ) {
      throw std::runtime_error( "SSLSocket::Read( SSL Error )" );
   }

   return st;

}


/**
  *  Write
  *     use SSL_write to write data to an encrypted channel
  *
  *  @param	void * buffer to store data read
  *  @param	size_t size, buffer's capacity
  *
  *  @return	size_t byte quantity written
  *
  *  Writes data to a secure channel
  *
 **/
size_t SSLSocket::Write( const char * string ) {
   int st = -1;
   st = SSL_write((SSL *)this->SSLStruct, (void *)string, strlen(string));

   if ( -1 == st ) {
      throw std::runtime_error( "SSLSocket::Write( SSL Error )" );
   }

   return st;

}


/**
  *  Write
  *     use SSL_write to write data to an encrypted channel
  *
  *  @param	void * buffer to store data read
  *  @param	size_t size, buffer's capacity
  *
  *  @return	size_t byte quantity written
  *
  *  Reads data from secure channel
  *
 **/
size_t SSLSocket::Write( const void * buffer, size_t size ) {
   int st = -1;
   st = SSL_write((SSL *)this->SSLStruct, buffer, size);

   if ( -1 == st ) {
      throw std::runtime_error( "SSLSocket::Write( SSL Error )" );
   }
   return st;

}

void SSLSocket::Copy(SSLSocket* original) {
   int st;
   SSL_CTX *context = reinterpret_cast<SSL_CTX *>(original->SSLContext);

   SSL *ssl = SSL_new(context); // Crear la estructura SSL
   if (!ssl) {
       ERR_print_errors_fp(stderr);
       throw std::runtime_error("SSLSocket::Copy: Error al crear SSL");
   }

   this->SSLStruct = ssl;

   st = SSL_set_fd((SSL*)this->SSLStruct, this->idSocket);
   if (st != 1) {
       ERR_print_errors_fp(stderr);
       throw std::runtime_error("SSLSocket::Copy: Error al asociar socket con SSL");
   }
}

void SSLSocket::Accept(){
    int st = -1;
    st = SSL_accept((SSL *)this->SSLStruct);
    std::cout << "SSL accept" << std::endl;
    if ( -1 == st ) {
        throw std::runtime_error( "SSLSocket::Write( SSL Error )" );
     }
}

/**
* AcceptiConnection method
*    use base class to accept connections
*
*  @returns   a new class instance
*
*  Waits for a new connection to service (TCP mode: stream)
*
**/
SSLSocket * SSLSocket::AcceptConnection(){
   int id;
   SSLSocket * peer;

   id = this->WaitForConnection();
   std::cout << "Conexion entrante" << std::endl;

   peer = new SSLSocket( id );
   return peer;

}



/**
 *   Show SSL certificates
 *
 **/
void SSLSocket::ShowCerts() {
   X509 *cert;
   char *line;

   cert = SSL_get_peer_certificate( (SSL *) this->SSLStruct );		 // Get certificates (if available)
   if ( nullptr != cert ) {
      printf("Server certificates:\n");
      line = X509_NAME_oneline( X509_get_subject_name( cert ), 0, 0 );
      printf( "Subject: %s\n", line );
      free( line );
      line = X509_NAME_oneline( X509_get_issuer_name( cert ), 0, 0 );
      printf( "Issuer: %s\n", line );
      free( line );
      X509_free( cert );
   } else {
      printf( "No certificates.\n" );
   }

}


/**
 *   Return the name of the currently used cipher
 *
 **/
const char * SSLSocket::GetCipher() {

   return SSL_get_cipher( reinterpret_cast<SSL *>( this->SSLStruct ) );

}