// ErrorTableColab.h
#pragma once
#include <string>
#include <unordered_map>

struct ErrorInfo {
    std::string tipo;      // Descripción corta
    std::string mensaje;   // Mensaje detallado
};

static const std::unordered_map<int, ErrorInfo> ERROR_TABLE = {
    {100, {"Servidor no alcanzable", "No se puede establecer conexión con el servidor."}},
    {101, {"Fork no alcanzable", "No se puede establecer conexión con el tenedor."}},
    {102, {"Pérdida abrupta de conexión al servidor", "Sucedió una pérdida abrupta de conexión al servidor."}},
    {103, {"Pérdida abrupta de conexión al tenedor", "Sucedió una pérdida abrupta de conexión al tenedor."}},
    {104, {"Servidor de dibujos vacío", "El servidor de dibujos se encuentra vacío."}},
    {105, {"Timeout de servidor", "Ha sucedido un timeout de solicitud al servidor."}},
    {106, {"Timeout de tenedor", "Ha sucedido un timeout de solicitud al tenedor."}},
    {200, {"Solicitud inválida de cliente", "La solicitud del cliente tiene un formato inválido."}},
    {201, {"Dibujo no encontrado", "El dibujo solicitado no fue encontrado."}},
    {202, {"Pérdida abrupta de conexión al tenedor", "Ha sucedido una pérdida abrupta de conexión al cliente."}},
    {300, {"Error en mensaje por protocolo", "Ha sucedido un error en el formato del mensaje: {mensaje}"}}
};
