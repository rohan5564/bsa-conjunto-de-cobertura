#include <QMainWindow>
#include <QVariant>
#include <vector>
#include <thread>
#include <future>
#include "bsa.h"
#include "datos.h"
#include "busquedaensegundoplano.h"
#include "gui_registro.h"

#pragma once

extern std::map<unsigned long long, bsa::Camino*> GENERACIONES;
extern unsigned long long MEJORGENERACION; //variable global: generacion del mejor camino
extern double MIXRATE; //control de cruzamiento

namespace Ui {
class MainWindow;
}

class Gui_Main : public QMainWindow
{
    Q_OBJECT

public:
    explicit Gui_Main(QWidget *parent = nullptr);
    ~Gui_Main();
    void cargarAlgoritmo();
    void mostrarInformacion(bool t_condicion);

protected:
    void closeEvent(QCloseEvent *e);

public slots:
    void alEncontrarElCamino(bool);

private slots:
    void on_btn_iniciar_clicked();

    void on_btn_registro_clicked();

private:
    Ui::MainWindow *ui;
    Gui_Registro *registro = nullptr;
    QMovie *gif;
    BusquedaEnSegundoPlano *busquedaEnSegundoPlano = nullptr;
    void bestFitness(std::vector<bsa::Camino*>& t_caminos);
    void guardar(const QString &t_grupo, const QString &t_llave, const QVariant &t_valor);
    QVariant cargar(const QString &t_grupo, const QString &t_llave, const QVariant &t_valor);
};

