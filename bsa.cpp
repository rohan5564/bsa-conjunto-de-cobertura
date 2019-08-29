#include "bsa.h"
#include <iostream>
#include <algorithm>

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////
/* INICIO CLASE CAMINO*/
////////////////////////////////////////////////////////////////////////////////////////////////////
bool bsa::operator==(const bsa::Camino& c1, const bsa::Camino& c2)
{
    if(c1.antenas.size() != c2.antenas.size())
        return false;
    for(const auto& com1 : c1.antenas){
        bool encontrado = false;
        for(const auto& com2 : c2.antenas){
            if(datos::compararComunas(com1, com2))
                encontrado = true;
        }
        if(!encontrado)
            return false;
    }
    return true;
}

bool bsa::operator!=(const bsa::Camino &c1, const bsa::Camino& c2)
{
    return !(c1 == c2);
}

bool bsa::operator>(const bsa::Camino &c1, const bsa::Camino &c2)
{
    return c1.cobertura.size() > c2.cobertura.size();
}

bool bsa::operator>=(const bsa::Camino &c1, const bsa::Camino &c2)
{
    return c1.cobertura.size() >= c2.cobertura.size();
}

bool bsa::operator<(const bsa::Camino &c1, const bsa::Camino &c2)
{
    return c1.cobertura.size() < c2.cobertura.size();
}

bool bsa::operator<=(const bsa::Camino &c1, const bsa::Camino &c2)
{
    return c1.cobertura.size() <= c2.cobertura.size();
}

bsa::Camino::Camino(){
    dummy = true;
}

bsa::Camino::Camino(const bsa::Camino &c) : antenas(c.antenas), cobertura(c.cobertura), mutado(c.mutado), dummy(c.dummy){}

bsa::Camino::Camino(std::weak_ptr<datos::Comuna> _inicio) {
    dummy = false;
    antenas.push_back(_inicio);
    cobertura.push_back(_inicio);
    for (const auto& vecino : _inicio.lock()->getVecinos())
        cobertura.push_back(vecino);
};

bsa::Camino::~Camino() {
    limpiar(antenas);
    limpiar(cobertura);
};

bool bsa::Camino::esValido() {
    if (dummy)
        return false;
    unsigned int i = 0;
    for(auto& cob = cobertura; i < cobertura.size(); ++i){
        auto& comuna = cob.at(i);
        if (find_if(VALPO.comunas.begin(), VALPO.comunas.end(),
            [&comuna](weak_ptr<datos::Comuna> p) { return datos::compararComunas(comuna, p); })
            != VALPO.comunas.end())
            continue;
        return false;
    }
    return i >= VALPO.comunas.size();
}

bool bsa::Camino::iniciarBusqueda() {
    while (avanzar());
    return esValido();
}

void bsa::Camino::limpiar(std::vector<std::weak_ptr<datos::Comuna>>& t_vector) {
    while (t_vector.size()){
        t_vector.back().reset();
        t_vector.pop_back();
    }
    t_vector.clear();
}

bool bsa::Camino::avanzar() {
    const auto& intermedio = antenas.back().lock()->vecinoRandom(datos::EVADIR, antenas); //salto intermedio
    vector<weak_ptr<datos::Comuna>> noRegistrados = ampliarCobertura(*intermedio.lock(), cobertura); //diferencia de conjuntos (vecinos de "intermedio" - cobertura)
    const auto& siguiente = intermedio.lock()->vecinoRandom(datos::AGARRAR, noRegistrados); //seleccion de comuna entre los no registrados

    /* si el punto intermedio y la antena son ubicaciones validas para el modelo, entonces se agrega la antena y
     * las comunas vecinas restantes a la cobertura
     */
    if (!intermedio.expired() && !siguiente.expired()) {
        antenas.push_back(siguiente);
        cobertura.push_back(siguiente);
        for (auto& vecino : siguiente.lock()->getVecinos()) {
            if (find_if(cobertura.begin(), cobertura.end(),
                [vecino](weak_ptr<datos::Comuna> p) { return datos::compararComunas(vecino, p); })
                != cobertura.end())
                continue;
            cobertura.push_back(vecino);
        }
        return true;
    }
    return false;
};

void bsa::Camino::retroceder() {
    std::vector<std::weak_ptr<datos::Comuna>> vecindario; //cobertura de la ultima antena
    /* se agregan al vector los vecinos de la ultima antena, descartando los que puedan ser vecinos
     * de otras antenas
     */
    for(auto i :antenas.back().lock()->getVecinos())
        vecindario.push_back(i);
    for(int i = static_cast<int>(antenas.size()-2) ; i>=0; --i){
        if(vecindario.size()==0)
            break;
        for(const auto& j : antenas[static_cast<unsigned int>(i)].lock()->getVecinos()){
            for(const auto& k:vecindario){
                if(datos::compararComunas(k,j)){
                    vecindario.erase(remove_if(vecindario.begin(), vecindario.end(),
                        [&k](weak_ptr<datos::Comuna> p) {return datos::compararComunas(p,k);}),
                        vecindario.end());
                }
            }
        }
    }

    /* se eliminan las comunas de la cobertura que se encuentren en el vector
     */
    if(vecindario.size()>0){
        for(const auto& i : vecindario){
            cobertura.erase(remove_if(cobertura.begin(), cobertura.end(),
                [&i](weak_ptr<datos::Comuna> p) {return datos::compararComunas(i,p);}),
                cobertura.end());
        }
    }
    antenas.back().reset();
    antenas.pop_back();
}

std::vector<std::weak_ptr<datos::Comuna>> bsa::Camino::ampliarCobertura(const datos::Comuna t_comuna, std::vector<std::weak_ptr<datos::Comuna>> &t_cob)
{
    std::vector<std::weak_ptr<datos::Comuna>> noRegistrados;
    for (auto& vecino : t_comuna.getVecinos()) {
        if (find_if(t_cob.begin(), t_cob.end(),
            [&vecino](weak_ptr<datos::Comuna> p) { return datos::compararComunas(vecino, p); })
            != t_cob.end())
            continue;
        noRegistrados.push_back(vecino);
    }
    return noRegistrados;
}

float bsa::Camino::costoFinal() {
    float total = 0;
    if (!esValido())
        return total;
    for (const auto& i : antenas)
        total += i.lock()->getPrecio();
    return total;
}

void bsa::Camino::mutar(unsigned int porcentajeDeMutacion) {
    unsigned int porcentaje = static_cast<unsigned int>((porcentajeDeMutacion*(antenas.size()-1))/100);
    if(porcentaje == 0)
        return;
    mutado = true;
    for(unsigned int i = 0; i < porcentaje; ++i){
        retroceder();
    }
    while(avanzar());

}

void bsa::Camino::cruzar(bsa::Camino* &c2){
    int id = -1; //id de la comuna en comun (punto de cruzamiento)
    unsigned int pos = 0; //posicion en el vector de antenas
    for(auto& i : antenas){
        pos = 0;
        for(auto& j : c2->antenas){
            if(datos::compararComunas(i,j)){
                id = static_cast<int>(i.lock()->getId());
                break;
            }
            ++pos;
        }
        if(id>0)
            break;
    }

    /* de ser encontrado se agregan las antenas restantes y la cobertura alcanzada
     */
    if(id>0){
        while(static_cast<int>(antenas.back().lock()->getId()) != id){
            retroceder();
        }
        std::vector<std::weak_ptr<datos::Comuna>> coberturaCruzada;
        for(auto i = pos; i<c2->antenas.size(); ++i){
            auto tmp = ampliarCobertura(*c2->antenas.at(pos).lock(), cobertura);
            coberturaCruzada.insert(coberturaCruzada.end(), tmp.begin(), tmp.end());
        }
        antenas.insert(antenas.end(), c2->antenas.begin()+pos, c2->antenas.end());
        cobertura.insert(cobertura.end(), coberturaCruzada.begin(), coberturaCruzada.end());
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/* FIN CLASE CAMINO*/
////////////////////////////////////////////////////////////////////////////////////////////////////

void bsa::busquedaInicial(std::vector<std::unique_ptr<bsa::Camino>>& t_iniciales, bool& t_timeout,
                          int& t_caminosRestantes, Camino*& t_mejor) {
    if(t_timeout)
        return;

    auto camino = new Camino(VALPO.getRandom());
    camino->iniciarBusqueda();

    if(camino){ //para aplicar mutex
        scoped_lock<mutex> seguro(MUTEX);
        std::sort(camino->antenas.begin(), camino->antenas.end(), [&](const auto& c1,const auto& c2)
        {return c1.lock()->getId() < c2.lock()->getId();});
        t_iniciales.push_back(make_unique<Camino>(*camino));
        if(!t_mejor || camino->costoFinal() < t_mejor->costoFinal()){
            delete t_mejor;
            t_mejor = camino;
        }
        --t_caminosRestantes;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/* INICIO CLASE ALGORITMO::BSATAREA*/
////////////////////////////////////////////////////////////////////////////////////////////////////

extern bsa::Camino* MEJORCAMINO; //variable global: camino mas barato
extern std::map<unsigned long long, bsa::Camino*> GENERACIONES;
extern unsigned long long MEJORGENERACION; //variable global: generacion del mejor camino

bsa::algoritmo::BsaTarea::BsaTarea(bool& tm, const std::vector<std::unique_ptr<bsa::Camino>>& solucionesIniciales,
                              unsigned int n, unsigned int d, double mr):
    timeout(&tm), N(n), D(d), mixrate(mr){
    auto p = new std::vector<std::vector<bsa::Camino*>>(n, std::vector<bsa::Camino*>(d));
    P = p;
    auto oldp = new std::vector<std::vector<bsa::Camino*>>(n, std::vector<bsa::Camino*>(d));
    oldP = oldp;
    auto map = new std::vector<std::vector<bsa::Camino*>>(n, std::vector<bsa::Camino*>(d));
    trialP = map;

    /*se inicializan N individuos en P aplicando distribucion uniforme en el vector de soluciones iniciales,
    * por lo que pueden repetirse soluciones iniciales como individuos
    */
    for(unsigned int i = 0; i < n; ++i){
        for(unsigned int j = 0; j < d; ++j){
            auto *solRng = new bsa::Camino(*solucionesIniciales.at(static_cast<unsigned int>(U(0, solucionesIniciales.size()-1))).get());
            P->at(i).at(j) = new bsa::Camino(*solRng);
            oldP->at(i).at(j) = solRng;
        }
    }
}

bsa::algoritmo::BsaTarea::~BsaTarea(){
    for(std::vector<bsa::Camino*>& i : *P)
        i.clear();
    P->clear();
    delete P;
    P = nullptr;
    for(std::vector<bsa::Camino*>& i : *oldP)
        i.clear();
    oldP->clear();
    delete oldP;
    oldP = nullptr;
    for(std::vector<bsa::Camino*>& i : *trialP)
        i.clear();
    trialP->clear();
    delete trialP;
    trialP = nullptr;
}

double bsa::algoritmo::BsaTarea::U(double t_min = 0, double t_max = 1){
    std::uniform_real_distribution<double> distribucion(t_min,t_max);
    return distribucion(azar);
}

double bsa::algoritmo::BsaTarea::ND(double t_valor = 50, double t_desviacion = 15){
    std::normal_distribution<double> distribucion(t_valor,t_desviacion);
    return distribucion(azar);
}

void bsa::algoritmo::BsaTarea::permutacion(std::vector<std::vector<bsa::Camino*>>*& t_vector){
    for(std::vector<bsa::Camino*>& i : *t_vector)
        std::shuffle(i.begin(), i.end(), azar);
}

bool bsa::algoritmo::BsaTarea::insertar(std::vector<std::vector<bsa::Camino*>>*& t_vector, bsa::Camino* t_camino){
    for(unsigned int i = 0; i<N; ++i){
        for(unsigned int j = 0; j<D; ++j){
            if(t_vector->at(i).at(j) == nullptr || t_vector->at(i).at(j)->dummy){
                t_vector->at(i).at(j) = t_camino;
                return true;
            }
        }
    }
    return false;
}

void bsa::algoritmo::BsaTarea::seleccion(){
    /*se inicializan N individuos en P aplicando distribucion uniforme en el vector de soluciones iniciales,
     * por lo que pueden repetirse soluciones iniciales como individuos
     */
    if(U() < U()){
       for(auto& i:*oldP){
           i.clear();
       }
       oldP->clear();
       delete oldP;
       oldP = new std::vector<std::vector<bsa::Camino*>>(*P);
    }
    permutacion(oldP);
}

void bsa::algoritmo::BsaTarea::mutacion(){
    auto nd = ND();
    auto F = static_cast<unsigned int>(nd>99?100:nd<1?0:nd); //porcentaje de mutacion
    /* mutacion = P+F*(oldP-P)
     * la mutacion se realiza de forma aleatoria, en donde la poblacion de prueba sera mutada acorde al porcentaje de
     * mutacion definido previamente
     */

    std::vector<bsa::Camino*> M;
    //M += P
    for(std::vector<bsa::Camino*> &m : *P){
        for(bsa::Camino* &n : m)
            M.push_back(new bsa::Camino(*n));
    }

    //M += oldP-P
    for(std::vector<bsa::Camino*> &i : *oldP){
        for(bsa::Camino* &j : i){
            bool igualdad = false;
            for(std::vector<bsa::Camino*> &m : *P){
                for(bsa::Camino* &n : m){
                    if(*j == *n){
                        igualdad = true;
                        break;
                    }
                }
                if(igualdad)
                    break;
            }
            if(!igualdad)
                M.push_back(new bsa::Camino(*j));
        }
    }

    //M = mutacion con factor F
    for(auto &i : M){
        if(U() < U())
            i->mutar(F);
        insertar(trialP, ref(i));

    }

    M.clear();
}

void bsa::algoritmo::BsaTarea::cruce(){
    if(U()<U()){ //cruzar los elementos de manera aleatoria
        for(std::vector<bsa::Camino*> &i : *trialP){
            for(bsa::Camino* &j : i){
                if(U() <= mixrate){
                    auto tmp = new bsa::Camino(*j);
                    tmp->cruzar(trialP->at(static_cast<unsigned int>(U(0,trialP->size()-1))).at(static_cast<unsigned int>(U(0,i.size()-1))));
                    if(tmp->esValido()){
                        std::swap(tmp, j);
                    }
                    delete tmp;
                }
            }
        }
    }else if(MEJORCAMINO){ //cruzar un elemento aleatorio con la solucion de mejor fitness
        unsigned int i = static_cast<unsigned int>(U(0,trialP->size()-1));
        unsigned int j = static_cast<unsigned int>(U(0,trialP->at(i).size()-1));
        trialP->at(i).at(j)->cruzar(MEJORCAMINO);
    }
}

void bsa::algoritmo::BsaTarea::seleccion2(){
    if(!trialP)
        return;
    for (unsigned int i = 0; i < N; ++i) {
        for (unsigned int j = 0; j < D; ++j) {
            auto& pop = P->at(i).at(j);
            auto& T = trialP->at(i).at(j);
            if(T && T->esValido()){
                auto c = new bsa::Camino(*T);
                GENERACIONES.insert(std::make_pair(TOTALGENERACIONES, c));
                objFun(MEJORCAMINO, c);
                pop = c;
            }
        }
    }
}

bool bsa::algoritmo::BsaTarea::objFun(bsa::Camino* &c1, bsa::Camino* &c2)
{
    if(!c1 || c1->costoFinal() < 1 || (c2->costoFinal() <= c1->costoFinal()) ){
        c1 = c2;
        MEJORGENERACION = TOTALGENERACIONES;
        return true;
    }
    return false;
}

void bsa::algoritmo::BsaTarea::ejecutar(){
    while(!*timeout){
        ++TOTALGENERACIONES;
        seleccion();
        mutacion();
        cruce();
        seleccion2();
        /* limpieza de la poblacion de prueba
         */
        for(auto &i : *trialP){
            for(auto &j : i){
                delete j;
            }
        }
        trialP = new std::vector<std::vector<bsa::Camino*>>(N, std::vector<bsa::Camino*>(D));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/* FIN CLASE ALGORITMO::BSATAREA*/
////////////////////////////////////////////////////////////////////////////////////////////////////
