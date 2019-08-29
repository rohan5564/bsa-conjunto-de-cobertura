#include <vector>
#include <string>
#include <memory>
#include <initializer_list>

#pragma once

namespace datos {

    static const short int EVADIR = 1; //opcion del metodo "vecinoRandom" en la clase "Comuna"
    static const short int AGARRAR = 2; //opcion del metodo "vecinoRandom" en la clase "Comuna"

    class Comuna : public std::enable_shared_from_this<Comuna> {
    public:
        Comuna(std::string&& t_nombre, float&& t_precio, unsigned int&& t_id);
        ~Comuna();

        const float& getPrecio() const ;

        const std::string& getNombre() const ;

        const unsigned int& getId() const;

        const std::vector<std::weak_ptr<Comuna>>& getVecinos() const;


        /* se itera sobre una lista de vecinos para agregarlos
         * @param t_vecinos listado de vecinos a agregar
         */
        void setVecinos(std::initializer_list<std::weak_ptr<Comuna>> const& t_vecinos);


        /* se busca un vecino en el listado de vecinos de la comuna
         * @param t_nombre: nombre de la comuna a buscar
         * @return direccion en memoria de un vecino encontrado o ubicacion vacia
         */
        std::weak_ptr<Comuna> buscarVecino(const std::string& t_nombre);

        /* se borra el enlace con una comuna
         * @param t_nombre: comuna a borrar
         * @return borrado exitoso o fallido
         */
        bool borrarVecino(const std::string& t_nombre);


        /* opera para conseguir una comuna vecina aleatoria segun se requiera desde un listado especifico o se
         * desee evadir otras comunas
         * @param t_opcion: cuenta con 2 opciones aceptadas:
         *                      EVADIR = se escoje un vecino aleatorio sin que se encuentre en el vector t_vector
         *                      AGARRAR = se escoje un vecino aleatorio que este en el vector t_vector
         * @param t_vector: vector desde el cual se operara segun t_opcion para conseguir un vecino aleatorio
         * @return direccion en memoria de una comuna aleatoria valida o ubicacion vacia
         */
        std::weak_ptr<Comuna> vecinoRandom(short int t_opcion, const std::vector<std::weak_ptr<Comuna>>& t_evadir);

    private:
        unsigned int m_id; //ID de la comuna
        std::string m_nombre; //nombre de la comuna
        float m_precio; //precio por poner antena repetidora
        std::vector<std::weak_ptr<Comuna>> m_vecinos; //vecinos de la comuna

        /* se agrega un vecino al listado de vecino de la comuna y viceversa
         * @param t_vecino: vecino a agregar
         * @return agregacion exitosa o fallida
         */
        bool setVecino(std::weak_ptr<Comuna> const& t_vecino);
    };

    /* se comparan las direcciones en memoria de las comunas ingresadas
     * @param t_comuna1, t_comuna2: comunas a comparar
     * @return misma o diferente direccion en memoria de las comunas
     */
    bool compararComunas(const std::weak_ptr<Comuna>& t_comuna1, const std::weak_ptr<Comuna>& t_comuna2);


    template<typename comuna>
    struct region {
        std::vector<std::shared_ptr<Comuna>> comunas; //comunas de la region

        /* se agrega una comuna a la region si es que no la registra previamente
         * @param t_comuna: comuna a agregar
         * @return agregacion exitosa o fallida
         */
        bool agregarComuna(std::shared_ptr<Comuna> const& t_comuna);

        /* se busca una comuna dentro de la region
         * @param t_nombre: nombre de la comuna buscada
         * @return direccion en memoria de la comuna
         */
        std::shared_ptr<Comuna> buscarComuna(const std::string& t_nombre);

        /* se elimina la direccion en memoria de una comuna
         * @param t_nombre: nombre de la comuna a borrar
         * @return borrado exitoso o fallido
         */
        bool borrarComuna(const std::string& t_nombre);

        /* @return direccion en memoria de una comuna aleatoria de la region
         */
        std::weak_ptr<Comuna> getRandom() const;


        /* se desvinculan los lazos entre las comunas ingresadas
         * @param t_comunas: comunas que se desvincularan
         * @return vecindad borrada de forma exitosa o fallida
         */
        bool borrarVecindad(std::initializer_list<std::shared_ptr<comuna>> t_comunas);
    };

    /* se inicializan los datos de la region y sus comunas
     * @return region de valparaiso
     */
    region<Comuna> inicializarDatos();

    extern region<Comuna> VALPO; //variable global: region de valparaiso
}
