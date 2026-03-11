#include <iostream>
#include <exception>
#include "Server.h"

int main(int argc, const char* argv[]) {
    Server* server;
    try {
        if (argc == 1) {
            // Sin argumentos: todo por defecto
            server = new Server(); // Usa IP, puerto y modo por defecto
        } else if (argc == 2) {
            // Solo modo (0 o 1)
            int modo = std::stoi(argv[1]);
            if (modo != 0 && modo != 1) {
                std::cerr << "El argumento debe ser 0 (procesos) o 1 (hilos)" << std::endl;
                return 1;
            }
            bool usarHilos = modo;
            server = new Server(usarHilos); // Constructor que recibe solo el modo
        } else if (argc == 4) {
            // IP, puerto, modo
            int puerto = std::stoi(argv[2]);
            if (puerto < 0 || puerto > 65535) {
                std::cerr << "El puerto debe estar entre 0 y 65535" << std::endl;
                return 1;
            }
            int modo = std::stoi(argv[3]);
            if (modo != 0 && modo != 1) {
                std::cerr << "El tercer argumento debe ser 0 (procesos) o 1 (hilos)" << std::endl;
                return 1;
            }
            bool usarHilos = modo;
            server = new Server(argv[1], puerto, usarHilos); // Constructor con todo
        } else {
            std::cerr << "Cantidad de argumentos inválida" << std::endl;
            std::cerr << "Uso:" << std::endl;
            std::cerr << "  ./Servidor                        -> Por defecto" << std::endl;
            std::cerr << "  ./Servidor [modo]                -> 0 = procesos, 1 = hilos" << std::endl;
            std::cerr << "  ./Servidor [ip] [puerto] [modo] -> Ej: ./Servidor 127.0.0.1 8000 1" << std::endl;
            return 1;
        }

        server->run();
        delete server;
    } catch (const std::exception& e) {
        std::cerr << "Error en los argumentos: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}