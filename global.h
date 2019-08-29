#include "datos.h"
#include "bsa.h"
#include "gui_main.h"
#include "busquedaensegundoplano.h"
#include <mutex>
#include <thread>
#include <map>

#pragma once

datos::region<datos::Comuna> VALPO = datos::inicializarDatos(); //asignacion de comunas en la region con sus correspondientes datos
bsa::Camino *MEJORCAMINO = nullptr; //camino mas barato
unsigned long long TOTALGENERACIONES; //generaciones transcurridas en el algoritmo bsa
unsigned long long MEJORGENERACION = 0; //generacion del mejor camino
unsigned int HILOS = std::thread::hardware_concurrency(); //cada sujeto DEBE encontrar un camino valido
std::mutex MUTEX; //propio de programacion multithread
std::vector<std::unique_ptr<bsa::Camino>> INICIALES; //historial de busquedas
std::map<unsigned long long, bsa::Camino*> GENERACIONES; //caminos validos por generacion
double MIXRATE = 0; //control de cruzamiento
struct cronometro CRONOMETRO; //mide el tiempo de busqueda del algoritmo

