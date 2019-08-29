#include <QListWidgetItem>
#include <QMetaType>
#include <algorithm>
#include "gui_registro.h"
#include "ui_registro.h"

using namespace std;

//Q_DECLARE_METATYPE(bsa::Camino *)
typedef std::pair<unsigned long long, bsa::Camino *> mapa;
Q_DECLARE_METATYPE(mapa)
Gui_Registro::Gui_Registro(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::Registro)
{
    ui->setupUi(this);
    registro();
}

Gui_Registro::~Gui_Registro()
{
    while(!actualizarLista());
    delete ui;
}

void Gui_Registro::on_btn_cerrar_clicked()
{
    QDialog::close();
    //this->~Gui_Registro();
}

void Gui_Registro::registro(const QString& str)
{
    int num = 0;
    if(str == nullptr){ //registro inicial
        for(const auto& i : GENERACIONES){
            QListWidgetItem *item = new QListWidgetItem;
            QVariant datos;
            mapa m = i;
            datos.setValue(m);
            item->setData(Qt::UserRole , datos);
            item->setText(QString::number(static_cast<double>(m.second->costoFinal())));
            ui->listado->insertItem(num++, item);
        }
    }else{ //registro de busqueda
        for(const auto& i : GENERACIONES){
            auto s = str.toStdString();
            transform(s.begin(), s.end(), s.begin(), [](unsigned char c){return tolower(c);});
            //buscar similitud del string ingresado con el nombre de cada comuna con antena
            if(find_if(i.second->antenas.begin(), i.second->antenas.end(),
                       [s](weak_ptr<datos::Comuna> e){
                            auto low = e.lock()->getNombre();
                            transform(low.begin(), low.end(), low.begin(), [](unsigned char c){return tolower(c);});
                            return low.find(s) != string::npos;}
                    ) != i.second->antenas.end())
            {
                QListWidgetItem *item = new QListWidgetItem;
                QVariant datos;
                mapa m = i;
                datos.setValue(m);
                item->setData(Qt::UserRole , datos);
                item->setText(QString::number(static_cast<double>(m.second->costoFinal())));
                ui->listado->insertItem(num++, item);
            }
        }
    }
    ui->listado->sortItems(Qt::AscendingOrder);
    ui->lbl_total->setText("Total: " +QString::number(num));
}

bool Gui_Registro::actualizarLista()
{
    for(int i = 0 ; i < ui->listado->count() ; ++i){
        ui->listado->takeItem(i)->data(Qt::UserRole).value<mapa>();
    }
    return ui->listado->count() == 0;
}


void Gui_Registro::on_buscar_comuna_textEdited(const QString &arg1)
{
    ui->listado->clearFocus();
    ui->listado->clearSelection();
    while(!actualizarLista());
    ui->buscar_comuna->text().isEmpty() ? registro() : registro(arg1);
}

void Gui_Registro::on_listado_currentRowChanged(int currentRow)
{
    ui->lbl_generacion->clear();
    ui->lbl_id->clear();
    ui->lbl_comuna->clear();
    ui->lbl_costo->clear();
    if(currentRow<0) return;
    auto datos = ui->listado->item(currentRow)->data(Qt::UserRole);
    bsa::Camino *item = datos.value<mapa>().second;
    for(const auto& i : item->antenas){
        ui->lbl_id->setText(ui->lbl_id->text() + QString::number(i.lock()->getId()) + "\n\n");
        ui->lbl_comuna->setText(ui->lbl_comuna->text() + QString::fromStdString(i.lock()->getNombre()) + "\n\n");
        ui->lbl_costo->setText(ui->lbl_costo->text() + QString::number(static_cast<double>(const_cast<float&>(i.lock()->getPrecio()))) + "\n\n");
    }
    ui->lbl_generacion->setText("generacion = " + QString::number(datos.value<mapa>().first));
}
