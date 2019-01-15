#include <thread>
#include <exception>

#include "socket.hpp"

class Threads {
    public:
        void send_thread(Socket &A, sockaddr_in remote_adress, std::exception_ptr &eptr);
        void recive_thread(Socket &A, sockaddr_in remote_adress, std::exception_ptr &eptr);
        void recive_and_send_thread(Socket &A, sockaddr_in remote_adress, std::exception_ptr &eptr);

        // Matar hilos
        void request_cancellation(std::thread& thread);

        // Manejo de señales
        void int_signal_handler(int signum);
};