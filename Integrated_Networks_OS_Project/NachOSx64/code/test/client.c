#include "syscall.h"

int main() {
   int id;
   char a[ 512 ];
   int n;

   id = Socket( AF_INET_NachOS, SOCK_STREAM_NachOS );
   Connect( id, "127.0.0.1", 8080 );
   Write( "GET /get/Goku HTTP/1.1\r\nHost: localhost\r\n\r\n", 64, id );
   while ((n = Read( a, 512, id )) > 0){
    Write( a, n, 1 );
   }

   Close( id );

}

