#include <iostream>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include <cerrno>
#include <cstring>
#include <vector>
#include <chrono>
#include <ctime>

#include "socket.hpp"
#include "helpers.cpp"

std::vector<Addressee> addressee;

Socket::Socket(const sockaddr_in &address)
{
    // fd_ valdra un numero (puntero a cabeza del socket)
    fd_ = socket(AF_INET, SOCK_DGRAM, 0);

    // Comprobamos si el numero devuelto por el socket es mayor que 0
    if (fd_ < 0)
    {
        throw std::system_error(ENOTCONN, std::generic_category(),
                                "Socket fail"); // Codigo ENOTCONN para fallo del socket
    }

    int result = bind(fd_, reinterpret_cast<const sockaddr *>(&address),
                      sizeof(address)); //Asignamos direccion al socket

    if (result < 0)
    {
        throw std::system_error(ECONNREFUSED, std::generic_category(),
                                "Failed bind connection"); // Codigo ECONNREFUSED para fallos de conexion
    }
}

Socket::~Socket()
{
    // fd_ = NULL; o esta..
    close(fd_);
}

sockaddr_in make_ip_address(const std::string &ip_address, int port)
{
    // Dirección del socket local
    sockaddr_in local_address{}; // Así se inicializa a 0, como se recomienda
    local_address.sin_family = AF_INET;

    // Convertimos ip en version binaria, pasando la a string en C y asignandole su valor
    inet_aton(ip_address.c_str(), &local_address.sin_addr);

    // Comprobamos que la ip exista, si no existe le asignamos cualquiera
    if (ip_address.empty())
    {
        local_address.sin_addr.s_addr = htonl(INADDR_ANY);
        std::cout << " Se ha asignado la ip: " << local_address.sin_addr.s_addr << std::endl;
    }

    // Si el puerto es 0 o no es especificado, el sistema le asignará uno.
    if (!port)
    {
        //throw std::system_error(errno, std::system_category(), "Empty port");
        local_address.sin_port = htons(0);
    }
    else
    {
        local_address.sin_port = htons(port); // Asignamos puerto
    }

    return local_address; // Devolvemos estructura
}

// Funcion para enviar mensajes
void Socket::send_to(const Message &message, const sockaddr_in &address)
{
    int result = sendto(fd_, &message, sizeof(message), 0,
                        reinterpret_cast<const sockaddr *>(&address),
                        sizeof(address));
    if (result < 0)
    {
        throw std::system_error(errno, std::system_category(), "sendto() failed");
    }
}

// Funcion para recibir mensajes
void Socket::receive_from(Message &message, sockaddr_in &address)
{
    socklen_t src_len = sizeof(address);

    int result = recvfrom(fd_, &message, sizeof(message), 0,
                          reinterpret_cast<sockaddr *>(&address), &src_len);
    if (result < 0)
    {
        throw std::system_error(errno, std::system_category(), "recvfrom() failed");
    }
}

// Funcion de enviar y recibir para el Server.
// Recive el mensaje con los datos del usuario, los guarda en un vector, y
// envia el mensaje a todos los usuarios menos al actual
void Socket::receive_and_send(Message &message, sockaddr_in &address)
{

    // Recivimos el mensaje de los destinatarios
    socklen_t src_len = sizeof(address);

    int result = recvfrom(fd_, &message, sizeof(message), 0,
                          reinterpret_cast<sockaddr *>(&address), &src_len);
    if (result < 0)
    {
        throw std::system_error(errno, std::system_category(), "recvfrom()<receive_and_send> failed");
    }

    // Inicializamos el vector addressee
    char *remote_ip = inet_ntoa(address.sin_addr);
    int remote_port = ntohs(address.sin_port);

    if (addressee.size() == 0) // Solo al principio, cuando addressee valga 0
    {
        Addressee actual_addressee;
        actual_addressee.remote_ip = inet_ntoa(address.sin_addr);
        actual_addressee.remote_port = ntohs(address.sin_port);
        actual_addressee.remote_adress = address;

        addressee.push_back(actual_addressee);

        std::cout << "El usuario: " << message.name << " se ha unido a la sala." << std::endl;
    }

    // Invocamos la funcion que nos mira si el actual usuario existe
    bool res = my_find(addressee, addressee.size(), remote_port);
    // Necesitamos el tamaño de nuestro vector, ya que si lo incluimos directamente
    // como parametro de salida del for, se quedara en un bucle infinito
    int size_adr = addressee.size();

    // Vamos rellenando nuestro vector si el usuario no existe
    for (int i = 0; i < size_adr; i++)
    {
        if (!res)
        {
            Addressee actual_addressee;

            actual_addressee.remote_ip = inet_ntoa(address.sin_addr); // contien la ip
            actual_addressee.remote_port = ntohs(address.sin_port);   // contiene el puerto
            actual_addressee.remote_adress = address;                 // contiene la direccion completa

            addressee.push_back(actual_addressee);

            std::cout << "El usuario: " << message.name << " se ha unido a la sala." << std::endl;
            break;
        }
    }

    // Procedemos a enviar el mensaje del usuario remoto a todos los usuario
    // que esten en nuestro vector menos al actual
    for (int i = 0; i < addressee.size(); i++)
    {
        if (addressee[i].remote_port != remote_port)
        {
            int result = sendto(fd_, &message, sizeof(message), 0,
                                reinterpret_cast<const sockaddr *>(&addressee[i].remote_adress),
                                sizeof(address));
            if (result < 0)
            {
                throw std::system_error(errno, std::system_category(), "sendto()<receive_and_send> failed");
            }
        }
    }
}
