#pragma once
#include <iostream>
#include <vector>

/* --
    En esta seccion añadiremos las funciones auxiliares, o helpers.
    Estas no son propias de las clases del programa como Socket o Threads
-- */




// Funcion std::find personalizada, devuelve true si se encontro el puerto del usuario,
// y false si el usuario es nuevo
bool my_find(std::vector<Addressee> addressee, int size, int value) // value sera nuestro puerto
{

    // EL problema de comprobar solo el puerto es que si nuestro programa fuese
    // ejecutado de forma no local, podrías coincidir puertos pero diferentes ips
    // SOLUCION NO OLVIDAR
    bool result;
    for (int i = 0; i < size; i++)
    {
        if (addressee[i].remote_port != value)
        {
            result = false;
        }
        else
        {
            result = true;
            break; // Si lo encuentra break, para que no siga iterando
        }
    }
    return result;
}
