#include <QDialog>
#include "bsa.h"
#include "datos.h"

#pragma once

extern std::vector<std::unique_ptr<bsa::Camino>> INICIALES; //historial de busquedas
extern std::map<unsigned long long, bsa::Camino*> GENERACIONES;

namespace Ui {
class Registro;
}

class Gui_Registro : public QDialog
{
    Q_OBJECT

public:
    explicit Gui_Registro(QWidget *parent = nullptr);
    ~Gui_Registro();

private slots:
    void on_btn_cerrar_clicked();

    void on_buscar_comuna_textEdited(const QString &arg1);

    void on_listado_currentRowChanged(int currentRow);

private:
    Ui::Registro *ui;

    void registro(const QString& str = nullptr);

    bool actualizarLista();
};
