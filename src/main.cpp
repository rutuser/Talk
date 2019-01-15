#include <iostream>
#include <unistd.h>
#include <stdlib.h>

#include "socket.hpp"
#include "threads.cpp"

#include <thread>
#include <atomic>
#include <csignal>

std::atomic<bool> quit(false);
std::atomic<bool> user_show(false);

// Funcion para manejar señales, activa true para matar hilos
void int_signal_handler(int signum)
{
    quit = true;
}

// Help function
void help_desk()
{
    std::cout << " \n-----TALK HELP INTERFACE----- \n"
                 " \n -h Help interface will show up \n"
                 " -s Server mode will be activated \n"
                 " -p Option with argument (port) \n"
                 " -c Client option will be activated. Option with argument (ip)"
              << std::endl;
}

int main(int argc, char *argv[])
{

    // Cuando llegue SIGINT, invocar int_signal_handler
    std::signal(SIGINT, &int_signal_handler);
    std::signal(SIGTERM , &int_signal_handler);
    std::signal(SIGHUP, &int_signal_handler);

    try
    {

        // Variables que usaremos para para marcar las opciones que
        // han sido indicadas por el usuario
        bool help_option = false;
        bool client_option = false;
        bool server_option = false;
        std::string port_option;
        int local_port;
        int remote_port;
        std::string ip_adress;

        // "hsup:c:01" indica que nuestro programa acepta las opciones
        // "-h", "c", "-s", "-p", "u", "-0" y "-1".

        // El "p:" y "c:" en la cadena indica que la opción "-p" admite un
        // argumento de la forma "-p argumento"

        // En cada iteración la variable "c" contiene la letra de la
        // opción encontrada. Si vale -1, es que ya no hay más
        // opciones en argv.

        int c;
        while ((c = getopt(argc, argv, "hsup:c:")) != -1)
        {
            switch (c)
            {
            case 'h':
                help_option = true;
                break;
            case 'c':
                client_option = true;
                ip_adress = std::string(optarg);
                break;
            case 's':
                server_option = true;
                break;
            case 'p':
                // Esta opción recibe un argumento.
                // getopt() lo guarda en la variable global "optarg"
                remote_port = atoi(optarg);
                local_port = 0;
                break;
            case 'u':
                user_show = true;
            case '?':
                // c == '?' cuando la opción no está en la lista
                // No hacemos nada porque getopt() se encarga de
                // mostrar el mensaje de error.
                break;
            default:
                // Si "c" vale cualquier otra cosa, algo fue mal con
                // getopt(). Esto nunca debería pasar.

                // Usamos fprintf() porque los errores siempre deben
                // imprimirse por la salida de error: stderr o cerr.
                std::fprintf(stderr, "?? getopt devolvió código "
                                     "de error 0%o ??\n",
                             c);
            }
        }

        // La variable global "optind" siempre contiene el índice del
        // siguiente elemento que debe ser procesado.
        // Si optind == argc, es que ya no hay más elementos en argv
        // para procesar.
        if (optind < argc)
        {
            std::cout << "-- argumentos no opción --\n";
            for (; optind < argc; ++optind)
            {
                std::cout << "argv[" << optind << "]: " << argv[optind] << '\n';
            }
        }

        // MANEJO DE OPCIONES
        // -h Help
        if (help_option)
        {
            help_desk();
        }

        // -c -s Client & Server
        if (client_option || server_option)
        {
            sockaddr_in remote_adress;
            sockaddr_in local_adress;

            // MODO SERVIDOR
            if (server_option)
            {
                std::cout << "                         |¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|" << std::endl;
                std::cout << "                         |       SERVER MODE        |" << std::endl;
                std::cout << "                         |__________________________|" << std::endl;

                //Contruimos la estructura pasando una ip y el puerto pedido al make_ip_adress
                remote_adress = make_ip_address("127.0.0.1", 0);

                local_adress = make_ip_address("127.0.0.1", atoi(optarg));

                //Inicializamos un socket A
                Socket A(local_adress);

                std::exception_ptr eptr2{};
                std::thread thread2(&recive_and_send_thread, std::ref(A), remote_adress, std::ref(eptr2));

                // Esperar a que un hilo termine...
                while (!quit)
                {
                    usleep(1000);
                }

                // Mátalos a todos!!!
                request_cancelation(thread2);

                thread2.join();

                if (eptr2)
                {
                    std::rethrow_exception(eptr2);
                }
            }

            // MODO CLIENTE
            else if (client_option)
            {
                std::cout << "                         |¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|" << std::endl;
                std::cout << "                         |       CLIENT MODE        |" << std::endl;
                std::cout << "                         |__________________________|" << std::endl;

                //Contruimos la estructura pasando una ip y el puerto pedido al make_ip_adress
                remote_adress = make_ip_address(ip_adress, atoi(optarg));

                local_adress = make_ip_address(ip_adress, 0);

                //Inicializamos un socket A
                Socket A(local_adress);

                std::exception_ptr eptr2{};
                std::thread thread2(&recive_thread, std::ref(A), remote_adress, std::ref(eptr2));

                std::exception_ptr eptr1{};
                std::thread thread1(&send_thread, std::ref(A), remote_adress, std::ref(eptr1));

                // Esperar a que un hilo termine...
                while (!quit)
                {
                    usleep(1000);
                }

                // Mátalos a todos!!!
                request_cancelation(thread1);
                request_cancelation(thread2);

                thread1.join();
                thread2.join();

                if (eptr1)
                {
                    std::rethrow_exception(eptr1);
                }

                if (eptr2)
                {
                    std::rethrow_exception(eptr2);
                }
            }
        }

        // Si el hilo terminó con una excepción, relanzarla aquí.
    }
    catch (std::bad_alloc &e)
    {
        std::cerr << "mytalk"
                  << ": memoria insuficiente\n";
        return 1;
    }

    catch (const std::system_error &e)
    {
        std::cerr << e.what() << std::endl;
    }
}