#include <vector>
#include <memory>
#include <chrono>
#include <atomic>
#include "datos.h"
#include <random>
#include <algorithm>
#include <functional>
#include <iostream>
#include <mutex>
#include <map>

#pragma once

extern datos::region<datos::Comuna> VALPO; //variable global: region de valparaiso
extern unsigned long long TOTALGENERACIONES; //variable global: generaciones transcurridas en el algoritmo bsa
extern std::mutex MUTEX;

namespace bsa {
    class Camino {
        /* operadores utilizados para ordenar el los elementos en cada generacion de individuos
         * en el algoritmo
         */
        friend bool operator==(const bsa::Camino &c1, const bsa::Camino &c2);
        friend bool operator!=(const bsa::Camino &c1, const bsa::Camino &c2);
        friend bool operator>(const bsa::Camino &c1, const bsa::Camino &c2);
        friend bool operator>=(const bsa::Camino &c1, const bsa::Camino &c2);
        friend bool operator<(const bsa::Camino &c1, const bsa::Camino &c2);
        friend bool operator<=(const bsa::Camino &c1, const bsa::Camino &c2);
    public:
        std::vector<std::weak_ptr<datos::Comuna>> antenas; //tiene alcance de una comuna y sus cercanias (registra solo la comuna de origen)
        std::vector<std::weak_ptr<datos::Comuna>> cobertura; //señal alcanzada por las antenas
        bool mutado = false;
        bool dummy = false;

        Camino();

        Camino(const Camino &c);

        Camino(std::weak_ptr<datos::Comuna> inicio);

        ~Camino();

        /* @return cobertura total de las antenas en la region
         */
        bool esValido();


        /* avance (constante) en la posicion de antenas y el registro de la cobertura total
         * @return validez del camino
         */
        bool iniciarBusqueda();


        /* @return costo total de produccion, que corresponde a la suma de precios de todas las antenas registradas
         */
        float costoFinal();


        /* avance (una iteracion) del camino a partir de la ultima antena guardada, utilizando un
         * salto intermedio a la siguiente antena a agregar para mantener la consistencia del
         * problema del conjunto de cobertura (SCP)
         * @return avance exitoso o fallido
         */
        bool avanzar();


        /* retroceso en el camino, osea en los vectores antena y cobertura
         */
        void retroceder();

        /* se buscaran los vecinos de una comuna en una cobertura especifica
         * @param t_comuna: comuna base
         * @param t_cob: cobertura en la que se buscaran los vecinos de t_comuna
         * @return comunas vecinas que no se encuentren en la cobertura
         */
        std::vector<std::weak_ptr<datos::Comuna>> ampliarCobertura(datos::Comuna t_comuna, std::vector<std::weak_ptr<datos::Comuna>> &t_cob);


        /* mutacion del camino, lo que se traduce volver a realizar la busqueda desde un punto en el camino.
         * @param t_porcentajeDeMutacion: porcentaje del camino que sufrira posibles modificaciones
         */
        void mutar(unsigned int t_porcentajeMutacion);


        /* cruzamiento de la comuna actual con otra, mientras no se repitan datos previos de las antenas
         * y la cobertura. El factor de cruzamiento variara segun el primer punto de cruzamiento alcanzado y
         * se completara el camino hasta agregar todos los elementos o alcancar la cobertura total de la region
         * @param t_c2: comuna con la que se cruzara la actual
         */
        void cruzar(bsa::Camino* &t_c2);

    private:

        /* se vacia un vector, eliminando los residuos de la memoria
         * @param t_vector: memoria del vector a vaciar
         */
        void limpiar(std::vector<std::weak_ptr<datos::Comuna>> &t_vector);

    };


    /*
    ciclo repetitivo que reasignara la primera antena de manera aleatoria para proceder con la busqueda
    @param t_iniciales: vector donde se almacenaran los caminos validos
    @param t_timeout: tiempo limite de busqueda
    @param t_n: numero de soluciones iniciales requerido, se decrementa cuando se agrega una solucion al vector de iniciales
    @param t_mejor: camino mas barato
    */
    void busquedaInicial(std::vector<std::unique_ptr<bsa::Camino>> &t_iniciales, bool &t_timeout,
                         int &t_n, Camino* &t_mejor);


    namespace algoritmo{

        static std::random_device rng; //aleatoriedad
        static std::mt19937 azar(rng()); //valor aleatorio utilizado para distribucion normal y uniforme
        class BsaTarea{
        private:
            bool *timeout; //control de loop en la funcion ejecutar
            std::vector<std::vector<bsa::Camino*>> *P = nullptr; //generacion actual
            std::vector<std::vector<bsa::Camino*>> *oldP = nullptr; //generacion = vector de vectores
            std::vector<std::vector<bsa::Camino*>> *trialP = nullptr; //poblacion de prueba
            unsigned int N; //tamaño de la poblacion
            unsigned int D; //dimension del problema == variables
            double mixrate; //control de cruzamiento

            double U(double min, double max); //distribucion uniforme

            double ND(double valor, double desviacion); //distribucion normal

            void permutacion(std::vector<std::vector<bsa::Camino*>>* &vector); //ordenar de forma aleatoria el vector

            bool insertar(std::vector<std::vector<bsa::Camino*>>* &vector, bsa::Camino* camino); //inserta

            void seleccion(); //primera seleccion de individuos

            void mutacion(); //mutacion de individuos

            void cruce(); //recombinacion de individuos

            void seleccion2(); //segunda seleccion: elitista, (U + Lambda)

            bool objFun(bsa::Camino* &c1, bsa::Camino* &c2); //funcion objetivo: encontrar el camino mas barato que cubra toda la region
        public:

            explicit BsaTarea(bool& tm, const std::vector<std::unique_ptr<bsa::Camino>> &solucionesIniciales,
                              unsigned int n, unsigned int d, double mr);
            ~BsaTarea();

            void ejecutar();
        };
    }
}
