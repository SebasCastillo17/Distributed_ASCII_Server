// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include <sys/socket.h>
#include <arpa/inet.h>

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "machine.h"
#include "synch.h"
#include "thread.h"
#include "addrspace.h"
#include "nachostabla.h"

extern Machine *machine;
extern Semaphore *Console = new Semaphore("Console", 1);

void returnFromSystemCall();
void NachosForkThread(void *p);

/*
 *  System call interface: Halt()
 */
void NachOS_Halt() {		// System call 0

	DEBUG('a', "Shutdown, initiated by user program.\n");
   currentThread->Finish();
   	interrupt->Halt();

}


/*
 *  System call interface: void Exit( int )
 */
void NachOS_Exit() {		// System call 1
    int exitCode = machine->ReadRegister(4); // Leer el código de salida
    DEBUG('u', "Exit system call with code %d\n", exitCode);

    // Liberar el espacio de direcciones del hilo actual
    if (currentThread->space != NULL) {
        delete currentThread->space;
        currentThread->space = NULL;
    }

    // Terminar el hilo actual
    currentThread->Finish();
    returnFromSystemCall();
}


/*
 *  System call interface: SpaceId Exec( char * )
 */
void NachOS_Exec() {		// System call 2
}


/*
 *  System call interface: int Join( SpaceId )
 */
void NachOS_Join() {		// System call 3
}


/*
 *  System call interface: void Create( char * )
 */
void NachOS_Create() {		// System call 4
}


/*
 *  System call interface: OpenFileId Open( char * )
 */
void NachOS_Open() {		// System call 5
// Read the name from the user memory, see 5 below
	// Use NachosOpenFilesTable class to create a relationship
	// between user file and unix file
	// Verify for errors
   char name[128] = {0};
   int direc = machine->ReadRegister(4);
   int index = 0;
   int c = 1;

   while(c) {
        machine->ReadMem(direc, 1, &c);
        name[index] = c;
        index++;
        direc++;
   }
   name[index] = 0;
   int unixHandle = OpenForReadWrite(name, true);
   if (unixHandle == -1) {
       machine->WriteRegister(2, -1);  // Error al abrir el archivo
   } else {
       int nachosHandle = currentThread->OpenFilesTable->Open(unixHandle);
       machine->WriteRegister(2, nachosHandle);
   }
   returnFromSystemCall();
} // Nachos_Open

/*
 *  System call interface: OpenFileId Read( char *, int, OpenFileId )
 */
// System call 6
void NachOS_Read() {
   Console->P();
    int virtualAddr = machine->ReadRegister(4);
    int size = machine->ReadRegister(5);
    OpenFileId id = machine->ReadRegister(6);
    if (size < 0) {
        machine->WriteRegister(2, -1);
        return;
    }

    if (size == 0) {
        machine->WriteRegister(2, 0);
        return;
    }
    if (!currentThread->OpenFilesTable->isOpened(id)) {
      printf("el archivo %i, no esta abierto\n", id);
        machine->WriteRegister(2, -1);
        return;
    }

    int unixHandle = currentThread->OpenFilesTable->getUnixHandle(id);
    char *kernelBuffer = new char[size];
    if (kernelBuffer == NULL) {
        machine->WriteRegister(2, -1);
        return;
    }

    int bytesRead = ReadPartial(unixHandle, kernelBuffer, size);

    if (bytesRead > 0) {
        for (int i = 0; i < bytesRead; i++) {
            if (!machine->WriteMem(virtualAddr + i, 1, kernelBuffer[i])) {
                delete[] kernelBuffer;
                machine->WriteRegister(2, -1);
                returnFromSystemCall();
                return;
            }
        }
    }


    delete[] kernelBuffer;
    machine->WriteRegister(2, bytesRead);
   Console->V();

    returnFromSystemCall();
}


/*
 *  System call interface: OpenFileId Write( char *, int, OpenFileId )
 */
void NachOS_Write() { // System call 7
    int size = machine->ReadRegister(5);
    char *buffer = new char[size];
    OpenFileId id = machine->ReadRegister(6);
    Console->P();
    
    int direc = machine->ReadRegister(4);
    
    for(int i = 0; i < size; i++) {
        int c;
        if (!machine->ReadMem(direc + i, 1, &c)) {
            delete[] buffer;
            machine->WriteRegister(2, -1);
            Console->V();
            returnFromSystemCall();
            return;
        }
        buffer[i] = c;
    }
    
    switch (id) {
        case ConsoleInput:    // User could not write to standard input
            machine->WriteRegister(2, -1);
            break;
        case ConsoleOutput:
            printf("%.*s", size, buffer);
            machine->WriteRegister(2, size);
            break;
        case ConsoleError:    // This trick permits to write integers to console
            printf("%d\n", machine->ReadRegister(4));
            machine->WriteRegister(2, size);
            break;
        default:
            if(currentThread->OpenFilesTable->isOpened(id)) {
                int id2 = currentThread->OpenFilesTable->getUnixHandle(id);
                WriteFile(id2, buffer, size);
                machine->WriteRegister(2, id2);
            } else {
                machine->WriteRegister(2, -1);
            }
            break;
    }
    
    delete[] buffer;  // Liberar la memoria
    Console->V();
    returnFromSystemCall();
} // NachOS_Write

/*
 *  System call interface: void Close( OpenFileId )
 */
void NachOS_Close() {		// System call 8
    OpenFileId id = machine->ReadRegister(4);
    if(currentThread->OpenFilesTable->isOpened(id)) {
        int unixHandle = currentThread->OpenFilesTable->getUnixHandle(id);
        Close(unixHandle);
        currentThread->OpenFilesTable->Close(id);
    }
    returnFromSystemCall();
}


/*
 *  System call interface: void Fork( void (*func)())
 */
void NachOS_Fork() {		// System call 9

	DEBUG( 'u', "Entering Fork System call\n" );
   Console->P();
	// We need to create a new kernel thread to execute the user thread
	Thread * newT = new Thread( "child to execute Fork code" );

	// We need to share the Open File Table structure with this new child
   newT->OpenFilesTable = currentThread->OpenFilesTable;
   currentThread->OpenFilesTable->addThread(); // ¡Incrementar el contador!
	// Child and father will also share the same address space, except for the stack
	// Text, init data and uninit data are shared, a new stack area must be created
	// for the new child
	// We suggest the use of a new constructor in AddrSpace class,
	// This new constructor will copy the shared segments (space variable) from currentThread, passed
	// as a parameter, and create a new stack for the new child
	newT->space = new AddrSpace( currentThread->space );

	// We (kernel)-Fork to a new method to execute the child code
	// Pass the user routine address, now in register 4, as a parameter
	// Note: in 64 bits register 4 need to be casted to (void *)
   // long para 64
	newT->Fork( NachosForkThread, (void*) (long) machine->ReadRegister( 4 ) );

	returnFromSystemCall();	// This adjust the PrevPC, PC, and NextPC registers
   Console->V();
	DEBUG( 'u', "Exiting Fork System call\n" );

}



/*
 *  System call interface: void Yield()
 */
void NachOS_Yield() {		// System call 10
}


/*
 *  System call interface: Sem_t SemCreate( int )
 */

 #define MAX_SEMAPHORES 128
Semaphore* semTable[MAX_SEMAPHORES] = {nullptr};

void NachOS_SemCreate() {		// System call 11
   int initialValue = machine->ReadRegister(4);

   int id = -1;
   for (int i = 0; i < MAX_SEMAPHORES; i++) {
       if (semTable[i] == nullptr) {
           semTable[i] = new Semaphore("S", initialValue);
           id = i;
           break;
       }
   }
   machine->WriteRegister(2, id);
   returnFromSystemCall();
}


/*
 *  System call interface: int SemDestroy( Sem_t )
 */
void NachOS_SemDestroy() {		// System call 12
   int id = machine->ReadRegister(4);
   if (id < 0 || id >= MAX_SEMAPHORES || semTable[id] == nullptr) {
       machine->WriteRegister(2, -1);
   } else {
       delete semTable[id];
       semTable[id] = nullptr;
       machine->WriteRegister(2, 0);
   }
   returnFromSystemCall();
}


/*
 *  System call interface: int SemSignal( Sem_t )
 */
void NachOS_SemSignal() {		// System call 13
   int id = machine->ReadRegister(4);
   if (id < 0 || id >= MAX_SEMAPHORES || semTable[id] == nullptr) {
       machine->WriteRegister(2, -1);
   } else {
       semTable[id]->V();
       machine->WriteRegister(2, 0);
   }
   returnFromSystemCall();
}


/*
 *  System call interface: int SemWait( Sem_t )
 */
void NachOS_SemWait() {		// System call 14
   int id = machine->ReadRegister(4);
   if (id < 0 || id >= MAX_SEMAPHORES || semTable[id] == nullptr) {
       machine->WriteRegister(2, -1);
   } else {
       semTable[id]->P();
       machine->WriteRegister(2, 0);
   }
   returnFromSystemCall();
}


/*
 *  System call interface: Lock_t LockCreate( int )
 */
void NachOS_LockCreate() {		// System call 15
}


/*
 *  System call interface: int LockDestroy( Lock_t )
 */
void NachOS_LockDestroy() {		// System call 16
}


/*
 *  System call interface: int LockAcquire( Lock_t )
 */
void NachOS_LockAcquire() {		// System call 17
}


/*
 *  System call interface: int LockRelease( Lock_t )
 */
void NachOS_LockRelease() {		// System call 18
}


/*
 *  System call interface: Cond_t LockCreate( int )
 */
void NachOS_CondCreate() {		// System call 19
}


/*
 *  System call interface: int CondDestroy( Cond_t )
 */
void NachOS_CondDestroy() {		// System call 20
}


/*
 *  System call interface: int CondSignal( Cond_t )
 */
void NachOS_CondSignal() {		// System call 21
}


/*
 *  System call interface: int CondWait( Cond_t )
 */

void NachOS_CondWait() {		// System call 22
}


/*
 *  System call interface: int CondBroadcast( Cond_t )
 */
void NachOS_CondBroadcast() {		// System call 23
}

extern bool IPv6 = false;

/*
 *  System call interface: Socket_t Socket( int, int )
 */
void NachOS_Socket() {			// System call 30
      int ipv6 = machine->ReadRegister(4) == AF_INET_NachOS ? AF_INET : AF_INET6;
      IPv6 = ipv6 == AF_INET6 ? true : false;
      int type = machine->ReadRegister(5) == SOCK_STREAM_NachOS ? SOCK_STREAM : SOCK_DGRAM;
      Console->P();
      int Socket = socket(ipv6, type, 0);
      printf("Socket: %d\n", Socket);
      if (Socket == -1) {
          machine->WriteRegister(2, -1);
      } else {
          machine->WriteRegister(2, Socket);
          currentThread->OpenFilesTable->Open(Socket);
      }
      Console->V();
      returnFromSystemCall();
}


/*
 *  System call interface: Socket_t Connect( char *, int )
 */
void NachOS_Connect() {		// System call 31
   int port = machine->ReadRegister(6);
   int id = machine->ReadRegister(4);
   char *buffer = new char[17];
   int direc = machine->ReadRegister(5);
   Console->P();
   for(int i = 0; i < 16; i++) {
      int c;
      if (!machine->ReadMem(direc + i, 1, &c)) {
          printf("Error leyendo memoria en byte %d\n", i);
          delete[] buffer;
          machine->WriteRegister(2, -1);
          Console->V();
          returnFromSystemCall();
          return;
      }
      buffer[i] = c;
      if (c == 0) break;
   }
   buffer[16] = 0;
   
   printf("IP leída: %s\n", buffer);
   printf("Port leído: %d\n", port);
   printf("IPv6: %d\n", IPv6);
   printf("ID: %d\n", id);
   if (IPv6) {
      struct sockaddr_in6 server;
      server.sin6_family = AF_INET6;
      server.sin6_port = htons(port);
      server.sin6_addr = in6addr_any;
      int st = connect(id, (struct sockaddr *)&server, sizeof(server));
      if (st == -1) {
         machine->WriteRegister(2, -1);
      } else {
         machine->WriteRegister(2, st);
      }
   } else {
      struct sockaddr_in server;
      server.sin_family = AF_INET;
      server.sin_port = htons(port);
      server.sin_addr.s_addr = inet_addr(buffer);
      int st = connect(id, (struct sockaddr *)&server, sizeof(server));
      if (st == -1) {
         machine->WriteRegister(2, -1);
      } else {
         machine->WriteRegister(2, st);
      }
   }
   Console->V();
   returnFromSystemCall();
}


/*
 *  System call interface: int Bind( Socket_t, int )
 */
void NachOS_Bind() {		// System call 32
}


/*
 *  System call interface: int Listen( Socket_t, int )
 */
void NachOS_Listen() {		// System call 33
}


/*
 *  System call interface: int Accept( Socket_t )
 */
void NachOS_Accept() {		// System call 34
}


/*
 *  System call interface: int Shutdown( Socket_t, int )
 */
void NachOS_Shutdown() {	// System call 25
}


//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    switch ( which ) {

       case SyscallException:
          switch ( type ) {
             case SC_Halt:		// System call # 0
                NachOS_Halt();
                break;
             case SC_Exit:		// System call # 1
                NachOS_Exit();
                break;
             case SC_Exec:		// System call # 2
                NachOS_Exec();
                break;
             case SC_Join:		// System call # 3
                NachOS_Join();
                break;

             case SC_Create:		// System call # 4
                NachOS_Create();
                break;
             case SC_Open:		// System call # 5
                NachOS_Open();
                break;
             case SC_Read:		// System call # 6
                NachOS_Read();
                break;
             case SC_Write:		// System call # 7
                NachOS_Write();
                break;
             case SC_Close:		// System call # 8
                NachOS_Close();
                break;

             case SC_Fork:		// System call # 9
                NachOS_Fork();
                break;
             case SC_Yield:		// System call # 10
                NachOS_Yield();
                break;

             case SC_SemCreate:         // System call # 11
                NachOS_SemCreate();
                break;
             case SC_SemDestroy:        // System call # 12
                NachOS_SemDestroy();
                break;
             case SC_SemSignal:         // System call # 13
                NachOS_SemSignal();
                break;
             case SC_SemWait:           // System call # 14
                NachOS_SemWait();
                break;

             case SC_LckCreate:         // System call # 15
                NachOS_LockCreate();
                break;
             case SC_LckDestroy:        // System call # 16
                NachOS_LockDestroy();
                break;
             case SC_LckAcquire:         // System call # 17
                NachOS_LockAcquire();
                break;
             case SC_LckRelease:           // System call # 18
                NachOS_LockRelease();
                break;

             case SC_CondCreate:         // System call # 19
                NachOS_CondCreate();
                break;
             case SC_CondDestroy:        // System call # 20
                NachOS_CondDestroy();
                break;
             case SC_CondSignal:         // System call # 21
                NachOS_CondSignal();
                break;
             case SC_CondWait:           // System call # 22
                NachOS_CondWait();
                break;
             case SC_CondBroadcast:           // System call # 23
                NachOS_CondBroadcast();
                break;

             case SC_Socket:	// System call # 30
		NachOS_Socket();
               break;
             case SC_Connect:	// System call # 31
		NachOS_Connect();
               break;
             case SC_Bind:	// System call # 32
		NachOS_Bind();
               break;
             case SC_Listen:	// System call # 33
		NachOS_Listen();
               break;
             case SC_Accept:	// System call # 32
		NachOS_Accept();
               break;
             case SC_Shutdown:	// System call # 33
		NachOS_Shutdown();
               break;

             default:
                printf("Unexpected syscall exception %d\n", type );
                ASSERT( false );
                break;
          }
          break;

       case PageFaultException: {
          break;
       }

       case ReadOnlyException:
          printf( "Read Only exception (%d)\n", which );
          ASSERT( false );
          break;

       case BusErrorException:
          printf( "Bus error exception (%d)\n", which );
          ASSERT( false );
          break;

       case AddressErrorException:
          printf( "Address error exception (%d)\n", which );
          ASSERT( false );
          break;

       case OverflowException:
          printf( "Overflow exception (%d)\n", which );
          ASSERT( false );
          break;

       case IllegalInstrException:
          printf( "Ilegal instruction exception (%d)\n", which );
          ASSERT( false );
          break;

       default:
          printf( "Unexpected exception %d\n", which );
          ASSERT( false );
          break;
    }

}

void returnFromSystemCall() {

        machine->WriteRegister( PrevPCReg, machine->ReadRegister( PCReg ) );		// PrevPC <- PC
        machine->WriteRegister( PCReg, machine->ReadRegister( NextPCReg ) );			// PC <- NextPC
        machine->WriteRegister( NextPCReg, machine->ReadRegister( NextPCReg ) + 4 );	// NextPC <- NextPC + 4

}       // returnFromSystemCall

// Pass the user routine address as a parameter for this function
// This function is similar to "StartProcess" in "progtest.cc" file under "userprog"
// Requires a correct AddrSpace setup to work well

void NachosForkThread( void * p ) { // for 64 bits version

    AddrSpace *space;

    space = currentThread->space;
    space->InitRegisters();             // set the initial register values
    space->RestoreState();              // load page table register
    currentThread->OpenFilesTable->addThread();
    

// Set the return address for this thread to the same as the main thread
// This will lead this thread to call the exit system call and finish
    machine->WriteRegister( RetAddrReg, 4 );

    machine->WriteRegister( PCReg, (long) p );
    machine->WriteRegister( NextPCReg, (long) p + 4 );

    machine->Run();                     // jump to the user progam
    ASSERT(false);

}
