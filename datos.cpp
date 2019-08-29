#include <iostream>
#include <algorithm>
#include <random>
#include <cassert>
#include "datos.h"

using namespace std;

/*
factor aleatorio
*/
static random_device rng;
static mt19937 azar(rng());

////////////////////////////////////////////////////////////////////////////////////////////////////
/* CLASE COMUNA*/
////////////////////////////////////////////////////////////////////////////////////////////////////
datos::Comuna::Comuna(string&& t_nombre, float&& t_precio, unsigned int&& t_id) :
    m_id(t_id), m_nombre(t_nombre), m_precio(t_precio) {}

datos::Comuna::~Comuna() {
    m_id = 0;
    m_nombre = "";
    m_precio = 0;
    for (auto& i : m_vecinos)
        i.reset();
    m_vecinos.clear();
}

const vector<weak_ptr<datos::Comuna>>& datos::Comuna::getVecinos() const { return m_vecinos; }

const float& datos::Comuna::getPrecio() const { return m_precio; }

const string& datos::Comuna::getNombre() const { return m_nombre; }

const unsigned int& datos::Comuna::getId() const { return m_id; }

void datos::Comuna::setVecinos(initializer_list<weak_ptr<datos::Comuna>> const& t_vecinos) {
    for (const auto& i : t_vecinos)
        setVecino(i);
}

bool datos::Comuna::setVecino(weak_ptr<Comuna> const& t_vecino) {
    if (find_if(m_vecinos.begin(), m_vecinos.end(),
        [t_vecino](weak_ptr<datos::Comuna> p) { return datos::compararComunas(t_vecino, p); })
        != m_vecinos.end())
        return false;
    m_vecinos.push_back(t_vecino);
    t_vecino.lock()->setVecino(shared_from_this());
    return true;
}

weak_ptr<datos::Comuna> datos::Comuna::buscarVecino(const string& t_nombre) {
    for (auto& i : m_vecinos) {
        if (!i.lock()->getNombre().compare(t_nombre))
            return i;
    }
    return weak_ptr<datos::Comuna>();
}

bool datos::Comuna::borrarVecino(const string& t_nombre) {
    const auto& vecino = buscarVecino(t_nombre);
    if (vecino.expired())
        return false;
    m_vecinos.erase(remove_if(m_vecinos.begin(), m_vecinos.end(),
        [vecino](weak_ptr<datos::Comuna> p) {return !(p.owner_before(vecino) || vecino.owner_before(p)); }),
        m_vecinos.end());
    return true;
}

weak_ptr<datos::Comuna> datos::Comuna::vecinoRandom(short int t_opcion, const vector<weak_ptr<datos::Comuna>>& t_vector) {
    assert(t_opcion == EVADIR || t_opcion == AGARRAR);
    if (m_vecinos.empty())
        return weak_ptr<datos::Comuna>();
    vector<weak_ptr<datos::Comuna>> random;
    short int limite = 10; //maximo de intentos de busqueda aleatoria, para un resultado consistente
    short int i = -1;
    if (t_opcion == EVADIR) {
        if (t_vector.size()) {
            random.push_back(weak_ptr<datos::Comuna>());
            do {
                random.pop_back();
                if (++i > limite) return weak_ptr<datos::Comuna>();
                sample(m_vecinos.begin(), m_vecinos.end(), back_inserter(random),
                    1, azar);
            } while (find_if(t_vector.begin(), t_vector.end(),
                [random](weak_ptr<datos::Comuna> p) {
                    return compararComunas(p, random[0]);
                })
                != t_vector.end());
        }
        else {
            sample(m_vecinos.begin(), m_vecinos.end(), back_inserter(random),
                1, azar);
        }
        return random[0];
    }
    else if (t_opcion == AGARRAR) {
        if (!t_vector.size()) {
            return weak_ptr<datos::Comuna>();
        }
        random.push_back(weak_ptr<datos::Comuna>());
        do {
            random.pop_back();
            if (++i > limite) return weak_ptr<datos::Comuna>();
            sample(m_vecinos.begin(), m_vecinos.end(), back_inserter(random),
                1, azar);
        } while ( !(find_if(t_vector.begin(), t_vector.end(),
            [random](weak_ptr<datos::Comuna> p) {
                return compararComunas(p, random[0]);
            })
            != t_vector.end()) );
        return random[0];
    }
    return weak_ptr<datos::Comuna>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/* FIN CLASE COMUNA*/
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
/* STRUCT REGION*/
////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
bool datos::region<datos::Comuna>::agregarComuna(const shared_ptr<Comuna>& t_comuna) {
    for (auto& i : comunas) {
        if (i == t_comuna)
            return false;
    }
    comunas.push_back(move(t_comuna));
    return true;
}

template <>
shared_ptr<datos::Comuna> datos::region<datos::Comuna>::buscarComuna(const string& t_nombre) {
    for (auto& i : comunas) {
        if (i->getNombre() == t_nombre)
            return i;
    }
    return nullptr;
}

template <>
bool datos::region<datos::Comuna>::borrarComuna(const string& t_nombre) {
    const auto& comuna = buscarComuna(t_nombre);
    if (comuna == nullptr)
        return false;

    comuna.~shared_ptr();
    return true;
}

template <>
bool datos::region<datos::Comuna>::borrarVecindad(initializer_list<shared_ptr<datos::Comuna>> t_comunas) {
    bool estado = true;
    for (auto& i : t_comunas) {
        for (auto& j : t_comunas) {
            if (i == j || i->buscarVecino(j->getNombre()).expired())
                continue;
            estado = estado && buscarComuna(i->getNombre())->borrarVecino(j->getNombre());
        }
        if (!estado) break;
    }

    return estado;
}

template <>
weak_ptr<datos::Comuna> datos::region<datos::Comuna>::getRandom() const {
    vector<weak_ptr<datos::Comuna>> random;
    sample(comunas.begin(), comunas.end(), back_inserter(random),
        1, azar);
    return random[0];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/* FIN STRUCT REGION*/
////////////////////////////////////////////////////////////////////////////////////////////////////

bool datos::compararComunas(const weak_ptr<datos::Comuna>& t_comuna1, const weak_ptr<datos::Comuna>& t_comuna2) {
    return !(t_comuna1.owner_before(t_comuna2) || t_comuna2.owner_before(t_comuna1));
}

datos::region<datos::Comuna> datos::inicializarDatos() {
    datos::region<datos::Comuna> valpo;
    auto calle_larga = make_shared<Comuna>("Calle Larga", 1.0f, 2);
    valpo.agregarComuna(move(calle_larga));
    auto los_andes = make_shared<Comuna>("Los Andes", 2.0f, 3);
    valpo.agregarComuna(move(los_andes));
    auto rinconada = make_shared<Comuna>("Rinconada", 1.2f, 4);
    valpo.agregarComuna(move(rinconada));
    auto san_esteban = make_shared<Comuna>("San Esteban", 1.5f, 5);
    valpo.agregarComuna(move(san_esteban));
    auto cabildo = make_shared<Comuna>("Cabildo", 3.0f, 6);
    valpo.agregarComuna(move(cabildo));
    auto la_ligua = make_shared<Comuna>("La Ligua", 2.0f, 7);
    valpo.agregarComuna(move(la_ligua));
    auto papudo = make_shared<Comuna>("Papudo", 1.0f, 8);
    valpo.agregarComuna(move(papudo));
    auto petorca = make_shared<Comuna>("Petorca", 1.0f, 9);
    valpo.agregarComuna(move(petorca));
    auto zapallar = make_shared<Comuna>("Zapallar", 3.0f, 10);
    valpo.agregarComuna(move(zapallar));
    auto hijuelas = make_shared<Comuna>("Hijuelas", 4.0f, 11);
    valpo.agregarComuna(move(hijuelas));
    auto la_calera = make_shared<Comuna>("La Calera", 3.0f, 12);
    valpo.agregarComuna(move(la_calera));
    auto la_cruz = make_shared<Comuna>("La Cruz", 3.0f, 13);
    valpo.agregarComuna(move(la_cruz));
    auto limache = make_shared<Comuna>("Limache", 2.0f, 14);
    valpo.agregarComuna(move(limache));
    auto nogales = make_shared<Comuna>("Nogales", 2.5f, 15);
    valpo.agregarComuna(move(nogales));
    auto olmue = make_shared<Comuna>("Olmue", 1.5f, 16);
    valpo.agregarComuna(move(olmue));
    auto quillota = make_shared<Comuna>("Quillota", 2.0f, 17);
    valpo.agregarComuna(move(quillota));
    auto algarrobo = make_shared<Comuna>("Algarrobo", 2.0f, 18);
    valpo.agregarComuna(move(algarrobo));
    auto cartagena = make_shared<Comuna>("Cartagena", 3.0f, 19);
    valpo.agregarComuna(move(cartagena));
    auto el_quisco = make_shared<Comuna>("El Quisco", 2.0f, 20);
    valpo.agregarComuna(move(el_quisco));
    auto el_tabo = make_shared<Comuna>("El Tabo", 2.0f, 21);
    valpo.agregarComuna(move(el_tabo));
    auto san_antonio = make_shared<Comuna>("San Antonio", 3.0f, 22);
    valpo.agregarComuna(move(san_antonio));
    auto santo_domingo = make_shared<Comuna>("Santo Domingo", 2.0f, 23);
    valpo.agregarComuna(move(santo_domingo));
    auto catemu = make_shared<Comuna>("Catemu", 3.0f, 24);
    valpo.agregarComuna(move(catemu));
    auto llay_llay = make_shared<Comuna>("Llay Llay", 3.0f, 25);
    valpo.agregarComuna(move(llay_llay));
    auto panquehue = make_shared<Comuna>("Panquehue", 1.0f, 26);
    valpo.agregarComuna(move(panquehue));
    auto putaendo = make_shared<Comuna>("Putaendo", 2.5f, 27);
    valpo.agregarComuna(move(putaendo));
    auto san_felipe = make_shared<Comuna>("San Felipe", 2.0f, 28);
    valpo.agregarComuna(move(san_felipe));
    auto santa_maria = make_shared<Comuna>("Santa Maria", 3.5f, 29);
    valpo.agregarComuna(move(santa_maria));
    auto casablanca = make_shared<Comuna>("Casablanca", 3.0f, 30);
    valpo.agregarComuna(move(casablanca));
    auto concon = make_shared<Comuna>("Concon", 1.5f, 31);
    valpo.agregarComuna(move(concon));
    auto puchuncavi = make_shared<Comuna>("Puchuncavi", 2.0f, 33);
    valpo.agregarComuna(move(puchuncavi));
    auto quilpue = make_shared<Comuna>("Quilpue", 2.0f, 34);
    valpo.agregarComuna(move(quilpue));
    auto quintero = make_shared<Comuna>("Quintero", 3.5f, 35);
    valpo.agregarComuna(move(quintero));
    auto valparaiso = make_shared<Comuna>("Valparaiso", 2.0f, 36);
    valpo.agregarComuna(move(valparaiso));
    auto villa_alemana = make_shared<Comuna>("Villa Alemana", 2.5f, 37);
    valpo.agregarComuna(move(villa_alemana));
    auto vina_del_mar = make_shared<Comuna>("Viña Del Mar", 1.5f, 38);
    valpo.agregarComuna(move(vina_del_mar));

    calle_larga->setVecinos({ los_andes, rinconada, san_felipe });
    los_andes->setVecinos({ calle_larga, san_esteban, san_felipe, santa_maria });
    rinconada->setVecinos({ calle_larga, llay_llay, panquehue, san_felipe });
    san_esteban->setVecinos({ los_andes, putaendo, santa_maria });
    cabildo->setVecinos({ la_ligua, petorca, nogales, catemu, putaendo });
    la_ligua->setVecinos({ cabildo, papudo, petorca, zapallar, nogales });
    papudo->setVecinos({ la_ligua, zapallar });
    petorca->setVecinos({ cabildo, la_ligua });
    zapallar->setVecinos({ la_ligua, papudo, nogales, puchuncavi });
    hijuelas->setVecinos({ la_calera, la_cruz, olmue, quillota, catemu, llay_llay });
    la_calera->setVecinos({ hijuelas, la_cruz, nogales });
    la_cruz->setVecinos({ hijuelas, la_calera, nogales, quillota, puchuncavi });
    limache->setVecinos({ olmue, quillota, concon, quilpue, villa_alemana });
    nogales->setVecinos({ cabildo, la_ligua, zapallar, hijuelas, la_calera, la_cruz, catemu, puchuncavi });
    olmue->setVecinos({ hijuelas, limache, quillota, quilpue });
    quillota->setVecinos({ hijuelas, la_cruz, limache, olmue, concon, puchuncavi, quintero });
    algarrobo->setVecinos({ el_quisco, casablanca });
    cartagena->setVecinos({ el_tabo, san_antonio, casablanca });
    el_quisco->setVecinos({ algarrobo, el_tabo, casablanca });
    el_tabo->setVecinos({ cartagena, el_quisco, casablanca });
    san_antonio->setVecinos({ cartagena, santo_domingo });
    santo_domingo->setVecinos({ san_antonio });
    catemu->setVecinos({ cabildo, hijuelas, nogales, llay_llay, panquehue, putaendo, san_felipe });
    llay_llay->setVecinos({ rinconada, hijuelas, catemu, panquehue });
    panquehue->setVecinos({ rinconada, catemu, llay_llay, san_felipe });
    putaendo->setVecinos({ san_esteban, cabildo, catemu, san_felipe, santa_maria });
    san_felipe->setVecinos({ calle_larga, los_andes, rinconada, catemu, panquehue, putaendo, santa_maria });
    santa_maria->setVecinos({ los_andes, san_esteban, putaendo, san_felipe });
    casablanca->setVecinos({ algarrobo, cartagena, el_quisco, el_tabo, quilpue, valparaiso });
    concon->setVecinos({ limache, quillota, quilpue, quintero, vina_del_mar });
    puchuncavi->setVecinos({ zapallar, la_cruz, nogales, quillota, quintero });
    quilpue->setVecinos({ limache, olmue,  casablanca, concon, valparaiso, villa_alemana, vina_del_mar });
    quintero->setVecinos({ quillota, concon, puchuncavi });
    valparaiso->setVecinos({ casablanca, quilpue, vina_del_mar });
    villa_alemana->setVecinos({ limache, quilpue });
    vina_del_mar->setVecinos({ concon, quilpue, valparaiso });

    return valpo;
}
