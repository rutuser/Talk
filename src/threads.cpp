#include <iostream>

#include <thread>
#include <atomic>
#include <cstdlib> // para std::getenv() y std::setenv()

#include "socket.hpp"

extern std::atomic<bool> quit;
extern std::atomic<bool> user_show;

void request_cancelation(std::thread &thread)
{

    // pthread_cancel devuelve un numero diferente a 0
    // si la cancelacion es erronea
    int res = pthread_cancel(thread.native_handle());

    //std::cout << res << std::endl;

    if (res != 0)
    {
        throw std::system_error(errno, std::system_category(), "pthread_cancelation() failed");
    }
}

void recive_thread(Socket &A, sockaddr_in remote_adress, std::exception_ptr &eptr)
{
    try
    {
        while (!quit)
        {
            Message message;
            A.receive_from(message, remote_adress); //Esperamos a recibir el mensaje
                                                    // Vamos a mostrar el mensaje recibido en la terminal

            // Primero convertimos la dirección IP como entero de 32 bits
            // en una cadena de texto.
            char *remote_ip = inet_ntoa(remote_adress.sin_addr);

            // Recuperamos el puerto del remitente en el orden adecuado para nuestra CPU
            int remote_port = ntohs(remote_adress.sin_port);

            // Imprimimos el mensaje
            auto end = std::chrono::system_clock::now();                      // End contendra la hora actual
            std::time_t end_time = std::chrono::system_clock::to_time_t(end); // La convertimos para poder leerla
            std::cout << " [ " << message.name << " ] "
                      << " -- "
                      << " Mensaje enviado a las " << std::ctime(&end_time);
            std::cout << " -> " << message.text << std::endl;
        }
    }
    catch (...)
    {
        eptr = std::current_exception();

        quit = true;
    }

    // En caso de excepción el hilo terminará solo por aquí.
}

void recive_and_send_thread(Socket &A, sockaddr_in remote_adress, std::exception_ptr &eptr)
{
    try
    {
        while (!quit)
        {
            Message message;
            A.receive_and_send(message, remote_adress); //Esperamos a recibir el mensaje y lo enviamos
        }
    }
    catch (...)
    {
        eptr = std::current_exception();

        quit = true;
    }

    // En caso de excepción el hilo terminará solo por aquí.
}

void send_thread(Socket &A, sockaddr_in remote_adress, std::exception_ptr &eptr)
{
    try
    {
        Message message;

        // Comprobamos si el usuario uso la opcion -u
        // si la ha elegido elegira su numbre de usuario
        if (user_show)
        {
            std::cout << "Indique su nombre de usuario: ";
            std::string name;

            std::getline(std::cin, name);
            message.name = name;
        }

        // En caso contrario se le asignara su nombre USER (variable de entorno)
        else
        {
            // Si getenv() devuelve nullptr, es que la variable
            // especificada no existe.
            const char *USER = std::getenv("USER");
            if (USER != nullptr)
                message.name = USER;
            else
            {
                throw std::system_error(errno, std::system_category(), "Variable de usuario no existe");
            }
        }

        while (!quit)
        {

            std::cout << "Intruduzca el mensaje" << std::endl;

            std::string frase;

            std::getline(std::cin, frase);

            if (frase == "quit")
            {
                quit = true;
            }

            // Copiamos el mensaje que recogimos y lo copiamos en nuestra estructura Message
            // y aprovechamos el return del copy para guardar la longitud del mensaje
            int size = frase.copy(message.text, sizeof(message.text) - 1, 0);

            //Marca para finalizar palabra
            message.text[size] = '\0';

            A.send_to(message, remote_adress); //Enviamos el mensaje
        }
    }
    catch (...)
    {
        eptr = std::current_exception();

        quit = true;
    }

    // En caso de excepción el hilo terminará solo por aquí.
}