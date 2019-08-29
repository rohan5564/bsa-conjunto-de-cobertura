#include "gui_main.h"
#include "ui_mainwindow.h"
#include <qcalendarwidget.h>
#include <QSettings>
#include <QMovie>
#include <chrono>
#include <thread>
#include <QThread>
#include <QTime>

using namespace std;

Gui_Main::Gui_Main(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    auto point = cargar("ventana", "posicion", pos()).value<QPoint>();
    move(point);
    ui->lbl_realizando_busqueda->hide();
    ui->widget_GUI->setVisible(false);
    ui->progreso->setVisible(false);
    ui->tiempo_maximo_de_busqueda->setTime(QTime(00,01,00));
    gif = new QMovie(":/resources/img/loading.gif");
    ui->lbl_realizando_busqueda->setMovie(gif);
    busquedaEnSegundoPlano = new BusquedaEnSegundoPlano(this);
    connect(busquedaEnSegundoPlano, SIGNAL(mejorCaminoEncontrado(bool)),
                this, SLOT(alEncontrarElCamino(bool)));
}

Gui_Main::~Gui_Main()
{
    delete MEJORCAMINO;
    MEJORCAMINO = nullptr;
    if(INICIALES.size() == 0){
        for(auto& i : INICIALES){
            delete i.release();
            i.reset();
        }
        INICIALES.clear();
    }
    delete registro;
    delete gif;
    delete busquedaEnSegundoPlano;
    delete ui;
}

void Gui_Main::closeEvent(QCloseEvent *e){
    guardar("ventana", "posicion", pos());
    if(busquedaEnSegundoPlano->isRunning()){
        busquedaEnSegundoPlano->terminate();
        busquedaEnSegundoPlano->wait();
    }
    QMainWindow::closeEvent(e);
}

void Gui_Main::on_btn_iniciar_clicked()
{
    mostrarInformacion(false);
    gif->start();
    MIXRATE = ui->mx_qspin->value();
    CRONOMETRO.INICIO = chrono::steady_clock::now();
    CRONOMETRO.secs = ui->tiempo_maximo_de_busqueda->time().second() + ui->tiempo_maximo_de_busqueda->time().minute()*60;
    std::thread t([&]{
        auto t = [](){return (100-((CRONOMETRO.secs - CRONOMETRO.tiempoActual())*100)/CRONOMETRO.secs);};
        while(!CRONOMETRO.fin()){
            ui->progreso->setText(QString::number(t()>=100?100:t()) + "%");
            this_thread::sleep_for(chrono::milliseconds(1));
        }
        ui->progreso->setText("");
    });
    t.detach();
    busquedaEnSegundoPlano->totalIniciales = static_cast<int>(ui->variables->value());
    busquedaEnSegundoPlano->start();

}

void Gui_Main::alEncontrarElCamino(bool valor){
    gif->stop();
    if(MEJORCAMINO == nullptr)
        return;
    ui->lbl_id->clear();
    ui->lbl_comuna->clear();
    ui->lbl_costo->clear();
    ui->lbl_costo_total->clear();
    ui->lbl_tiempo_de_busqueda->clear();
    ui->lbl_mejor_generacion->clear();
    ui->lbl_total_generaciones->clear();


    for (const auto& i : MEJORCAMINO->antenas){
        ui->lbl_id->setText(ui->lbl_id->text() + QString::number(i.lock()->getId()) + "\n\n");
        ui->lbl_comuna->setText(ui->lbl_comuna->text() + QString::fromStdString(i.lock()->getNombre()) + "\n\n");
        ui->lbl_costo->setText(ui->lbl_costo->text() + QString::number(static_cast<double>(const_cast <float&>(i.lock()->getPrecio()))) + "\n\n");
    }

    ui->lbl_costo_total->setText(QString::number(static_cast<double>(MEJORCAMINO->costoFinal())) + " unidades");
    {
        ui->lbl_tiempo_de_busqueda->setText(QString::number(CRONOMETRO.tiempoActual()/60)
                                        +":"+QString::number(CRONOMETRO.tiempoActual()%60)
                                        + ((CRONOMETRO.tiempoActual()/60 > 0) ?
                                            " minutos" : " segundos"));
    }
    ui->lbl_mejor_generacion->setText(QString::number(MEJORGENERACION));
    ui->lbl_total_generaciones->setText(QString::number(TOTALGENERACIONES));
    mostrarInformacion(valor);
}

void Gui_Main::mostrarInformacion(bool t_condicion)
{
    t_condicion ? ui->lbl_realizando_busqueda->hide() : ui->lbl_realizando_busqueda->show();
    ui->progreso->setVisible(!t_condicion);
    ui->btn_iniciar->setVisible(t_condicion);
    ui->widget_GUI->setVisible(t_condicion);
    ui->tabWidget->setTabEnabled(1,t_condicion);
    ui->tabWidget->setTabEnabled(2, t_condicion);
}

/*
fuera de la clase
*/
void Gui_Main::guardar(const QString &t_grupo, const QString &t_llave, const QVariant &t_valor)
{
    QSettings config;
    config.beginGroup(t_grupo);
    config.setValue(t_llave, t_valor);
    config.endGroup();
}

QVariant Gui_Main::cargar(const QString &t_grupo, const QString &t_llave, const QVariant &t_valor)
{
    QVariant valor;
    QSettings config;
    config.beginGroup(t_grupo);
    valor = config.value(t_llave, t_valor);
    config.endGroup();
    return valor;
}

void Gui_Main::on_btn_registro_clicked()
{
    registro = new Gui_Registro(this);
    registro->setWindowTitle("Historial de busquedas");
    registro->exec();
}

