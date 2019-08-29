#include "busquedaensegundoplano.h"
#include <QDebug>
#include <QMainWindow>
#include <QMutex>
#include <iostream>
#include <future>
#include <atomic>

using namespace std;

BusquedaEnSegundoPlano::BusquedaEnSegundoPlano(QObject *parent) :
    QThread(parent){
}

void BusquedaEnSegundoPlano::run(){
    resetearBusqueda();
    /*
    inicializacion de los sujetos de busqueda y la busqueda del mejor resultado encontrado
    */
    vector<std::thread> individuos;
    auto& t = timeout = false;
    auto n = totalIniciales;
    std::thread time([&t]{
        while(!CRONOMETRO.fin())
            this_thread::sleep_for(chrono::milliseconds(10));
        t = true;
    });
    while(n>0){
        for (unsigned int i = 0; i < HILOS; ++i) {
            std::thread thr(bsa::busquedaInicial, std::ref(INICIALES), std::ref(t), std::ref(n), std::ref(MEJORCAMINO));
            individuos.push_back(std::move(thr));
        }
        for (auto& i : individuos) {
            i.join();
        }
        individuos.clear();
    }    
    bsa::algoritmo::BsaTarea* algoritmo = new bsa::algoritmo::BsaTarea(timeout, INICIALES, totalIniciales, totalIniciales, MIXRATE);
    std::thread ta([&algoritmo](){algoritmo->ejecutar();});
    ta.join();
    delete algoritmo;
    time.join();
    std::thread checkeo(&BusquedaEnSegundoPlano::bestFitness, this, ref(n));
    checkeo.join();
}


void BusquedaEnSegundoPlano::bestFitness(int& n) {
    emit mejorCaminoEncontrado(!INICIALES.empty());
}

void BusquedaEnSegundoPlano::resetearBusqueda(){
    TOTALGENERACIONES = 0;
    MEJORCAMINO = nullptr;
    for(auto& i : INICIALES)
        i.reset();
    INICIALES.clear();
}
