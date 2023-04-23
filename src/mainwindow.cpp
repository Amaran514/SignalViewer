
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QtMath>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->signalView->setRenderHint(QPainter::Antialiasing);
    settings = new QSettings("setting.ini",QSettings::IniFormat);
    resize(settings->value("Window/width").toInt(), settings->value("Window/hight").toInt());
    ui->lineEditId->setText(settings->value("Id/id").toString());
    series = new QLineSeries();
    chart = new QChart();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->legend()->hide();
    chart->axes(Qt::Horizontal).first()->setRange(0, 160);
    chart->axes(Qt::Vertical).first()->setRange(0,5);
    ui->signalView->setChart(chart);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::updateView()
{
    series->clear();
    chart->axes(Qt::Horizontal).first()->setRange(0, (values.size()-1) / freq_sample);
    chart->axes(Qt::Vertical).first()->setRange(*std::min_element(values.begin(),values.end())*1.2,*std::max_element(values.begin(),values.end())*1.2);
    for (int n = 0; n < values.size(); ++n) {
        series->append(n / freq_sample, values[n]);
    }
    return;
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    settings->setValue("Window/hight", geometry().height());
    settings->setValue("Window/width", geometry().width());
}


void MainWindow::on_pushButtonReadFile_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "打开文件", "./");
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QString data;
    QStringList list;
    data = file.readAll();
    file.close();
    list = data.split(QRegularExpression("[\\s,]+"));
    values.clear();
    if (list[0].toInt()==20230406) {
        freq_sample = list[1].toInt();
        datatype = list[3].toInt();
        values.reserve(list.size());
        for (int i = 4; i < list.size(); ++i) {
            values.append(list[i].toInt());
        }
    } else {
        datatype = 2;
        QVector<double> cfg_values;
        cfg_values.reserve(list.size());
        for (int i = 0; i < list.size(); ++i) {
            cfg_values.append(list[i].toInt());
        }
        int id = ui->lineEditId->text().toInt();
        double freq_sine, amplitude;
        for (int i = 0; i < cfg_values.size(); ++i) {
            if (cfg_values[i]==id) {
                freq_sample = cfg_values[i+1];
                freq_sine = cfg_values[i+2];
                amplitude = cfg_values[i+3];
                qDebug() << freq_sample << freq_sine << amplitude;
                break;
            }
        }
        values.resize(100);
        for (int i = 0; i < values.size(); ++i) {
            values[i] = amplitude * cos(2*M_PI*freq_sine/freq_sample*i);
        }
    }
    updateView();
    return;
}


void MainWindow::on_pushButtonSaveFile_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, "保存文件", "./", "Data Files(*.dat)");
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return;
    }
    file.write("20230406\t");
    file.write(QString::number(freq_sample).toUtf8());
    file.write("\t20\t");
    file.write(QString::number(datatype).toUtf8());
    file.write("\n");
    for (int i = 0; i < values.size(); ++i) {
        file.write(QString::number(values[i]).toUtf8());
        if ((i+1)%20==0 && i!=0) {
            file.write("\n");
        } else {
            file.write("\t");
        }
    }
    file.close();
}


void MainWindow::on_lineEditId_editingFinished()
{
    settings->setValue("Id/id", ui->lineEditId->text());
}

