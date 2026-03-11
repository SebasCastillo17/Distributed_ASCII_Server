#pragma once

#define PORT 8080 ///< Puerto por defecto
#define IP "172.0.0.1" ///< IP por defecto
#define BUFSIZE 256 ///< Tamaño del buffer

#include "Socket.h"
#include "../FileSystem/FileSystem.h"
#include "../FileSystem/Structures.h"

/// @brief Clase que representa un servidor capaz de manejar conexiones TCP o UDP, utilizando hilos o procesos.
class Server {
  // Atributos
 private:
  const char * ip; ///< Dirección IP del servidor
  int port; ///< Puerto del servidor
  bool usarHilos; ///< Indica si se usan hilos (true) o procesos (false)
  VSocket * serverSocket; ///< Socket de comunicación principal
  char buffer[BUFSIZE] = {0}; ///< Buffer para los mensajes recibidos

  /**
   * @brief Imprime la respuesta recibida por el servidor.
   * @param respuesta Cadena de caracteres con la respuesta.
   */
  void imprimirRespuesta(char* respuesta);

  /**
   * @brief Monta e inicializa el sistema de archivos.
   */
  void MountSystem();

  // Métodos
 public:

  /**
   * @brief Constructor por defecto de la clase Server.
   * Establece valores por defecto para IP, puerto y uso de hilos.
   */
  Server();

  /**
   * @brief Constructor que permite especificar si se usan hilos.
   * @param usarHilos true si se desea usar hilos, false para usar procesos.
   */
  Server(bool usarHilos);

  /**
   * @brief Constructor que permite especificar IP, puerto y si se usan hilos.
   * @param ip Dirección IP del servidor.
   * @param port Puerto del servidor.
   * @param usarHilos true si se desea usar hilos, false para usar procesos.
   */
  Server(const char * ip, int port, bool usarHilos);

  /**
   * @brief Destructor de la clase Server.
   * Libera los recursos asociados al socket.
   */
  ~Server();
  
  /**
   * @brief Ejecuta el ciclo principal del servidor para manejar conexiones entrantes.
   */
  void run();

  /**
   * @brief Maneja una solicitud recibida por el socket.
   * @param socket Puntero al socket del cliente.
   * @param request Solicitud recibida.
   * @return Respuesta generada por el servidor.
   */
  std::string handleRequest(VSocket *socket, const char *request);

  /**
   * @brief Valida una solicitud recibida.
   * @param request Cadena de la solicitud.
   * @return Resultado de la validación.
   */
  std::string validateRequest(const char *request);
 
  /**
   * @brief Traduce una solicitud HTTP a formato ASCII.
   * @param httpRequest Solicitud en formato HTTP.
   * @return Solicitud traducida a formato ASCII.
   */
  std::string translateHttpToAscii(const char *httpRequest);

  /**
   * @brief Traduce una respuesta en ASCII a formato HTTP.
   * @param asciiResponse Respuesta en formato ASCII.
   * @return Respuesta traducida a formato HTTP.
   */
  std::string translateAsciiToHttp(const char *asciiResponse);
  
  /**
   * @brief Procesa una solicitud en formato ASCII.
   * @param asciiRequest Solicitud en formato ASCII.
   * @return Resultado del procesamiento.
   */
  std::string processThroughAscii(const std::string &asciiRequest);
};
