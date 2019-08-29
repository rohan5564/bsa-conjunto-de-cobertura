#include <QThread>
#include <QMainWindow>
#include "bsa.h"
#include "datos.h"

#pragma once

extern datos::region<datos::Comuna> VALPO; //variable global: region de valparaiso
extern unsigned long long TOTALGENERACIONES; //variable global: generaciones transcurridas en el algoritmo bsa
extern bsa::Camino* MEJORCAMINO; //variable global: camino mas barato
extern unsigned int HILOS;//cada sujeto DEBE encontrar un camino valido
extern std::mutex MUTEX; //propio de programacion multithread
extern std::vector<std::unique_ptr<bsa::Camino>> INICIALES; //historial de busquedas
extern double MIXRATE; //control de cruzamiento
extern struct cronometro{//calculo del tiempo
    std::chrono::time_point<std::chrono::steady_clock> INICIO;
    long long tiempoActual(){
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - INICIO).count();
    }
    int secs;
    bool fin(){
        return tiempoActual() >= secs;
    }
}CRONOMETRO;

namespace Ui {
class MainWindow;
}

class BusquedaEnSegundoPlano : public QThread
{
    Q_OBJECT
public:
    bool encontrado = false;
    bool timeout = false;
    int totalIniciales = 0;

    explicit BusquedaEnSegundoPlano(QObject *parent = nullptr);

    void bestFitness(int& n);

    void resetearBusqueda();

    void run();
signals:    
    void mejorCaminoEncontrado(bool);
};
