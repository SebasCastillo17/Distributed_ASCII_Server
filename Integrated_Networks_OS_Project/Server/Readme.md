# Servidor de ASCII art

## Este codigo implementa un servidor que responde a solicitudes HTTP y devuelve figuras ASCII o mensajes según el protocolo definido.

## Manual de usuario:

### Para compilar servidor

    make

* Argumentos:

1. Por defecto: modo = 0 (Con hilos)

        ./bin/Server

2. En modo -> 0 para hilos, 1 para procesos

        ./bin/Server <modo>

3. Con ip, puerto y modo -> 127.0.0.1  8080  0

        ./bin/Server <ip> <puerto> <modo>

### Para compilar cliente

* Sacar una terminal de la carpeta Cliente

1.  Generar carpeta build

        cmake -S . -B build

2.  Compilar carpeta build

        cmake --build build

3.  Para correr el programa cliente

        ./build/src/client



### Para usar dentro del navegador:

    Conectarse al localhost en el puerto donde esta corriendo el servidor y probar los distintos request.

* Request1 = objects
* Request2 = get/Goku
* Request3 = get/Gogeta
* Request4 = get/CocaCola