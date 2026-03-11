# Servidor ASCII e Implementación de Sockets

## Arquitectura

El trabajo consta de varios componentes principales:

1. **Infraestructura de Sockets**: Una implementación robusta de sockets que soporta tanto IPv4 como IPv6, con capacidades TCP (stream) y UDP (datagram).
2. **Servidor ASCII**: Un servidor que maneja solicitudes de archivos de arte ASCII y gestiona la comunicación utilizando un protocolo personalizado.
3. **Servidor Fork**: Actúa como intermediario entre clientes HTTP y el Servidor ASCII, traduciendo las peticiones HTTP al protocolo ASCII y viceversa.
4. **Sistema de Archivos**: Una implementación simple de sistema de archivos para almacenar y recuperar archivos de arte ASCII.

## Protocolo ASCII

El protocolo utilizado para la comunicación entre clientes y el servidor ASCII sigue el siguiente formato:

```
BEGIN/[COMANDO]/[PARÁMETROS]/END
```

Comandos soportados:

- **OBJECTS**: Lista todos los objetos ASCII disponibles.
- **GET**: Obtiene un archivo ASCII específico.
- **ON**: Registra un servidor o un tenedor.
- **OFF**: Desconecta un servidor.

Ver mas informacion en documento

## Protocolo HTTP

El Servidor Fork traduce entre el protocolo ASCII y HTTP para permitir el acceso a través de navegadores web. Las traducciones principales son:

- `GET /objects` → `BEGIN/OBJECTS/END`
- `GET /get/[nombre]` → `BEGIN/GET/[nombre]/END`
- `POST /on/server` → `BEGIN/ON/SERVIDOR/param1/param2/END`
- `POST /on/fork` → `BEGIN/ON/TENEDOR/END`
- `POST /off/server` → `BEGIN/OFF/SERVIDOR/param1/param2/END`

## Sistema de Archivos

El sistema de archivos implementa una estructura simple para almacenar y recuperar archivos ASCII. Cada archivo se almacena en bloques de 256 bytes en un disco virtual.

## Requisitos del Sistema

- Sistema operativo compatible con POSIX (Linux, Unix)
- Biblioteca estándar de C/C++
- Soporte para hilos POSIX (pthread)

## Compilación

Para compilar el proyecto, utilice los siguientes comandos en la raíz del proyecto:

```bash
make
```

## Uso

### Iniciar el Servidor ASCII

```bash
./bin/ASCIIServer 
```

```bash
./bin/ASCIIServer IPV6
```

### Iniciar el Servidor Fork

```bash
./ForkServer 
```

```bash
./ForkServer IPV6 
```

### Iniciar cliente

```bash
./Client Peticion (se puede dejar vacio) 
```

