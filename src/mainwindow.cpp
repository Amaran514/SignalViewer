
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QtMath>
#include <vector>
#include <complex>
#include "fft.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->signalView->setRenderHint(QPainter::Antialiasing);
    settings = new QSettings("setting.ini",QSettings::IniFormat);
    resize(settings->value("Window/width").toInt(), settings->value("Window/hight").toInt());
    ui->lineEditId->setText(settings->value("Id/id").toString());
    ui->lineEditNoiseMean->setText(settings->value("Noise/mean").toString());
    ui->lineEditNoiseStd->setText(settings->value("Noise/standardDeviation").toString());
    values.fill(0, 500); //  Fix crash when noise is added with no signal
    freq_sample = 500;
    datatype = 2;
    display_mode = 0;
    series = new QLineSeries();
    chart = new QChart();
    chart->addSeries(series);
    chart->createDefaultAxes();
    axisx = (QValueAxis*)chart->axes(Qt::Horizontal).first();
    axisy = (QValueAxis*)chart->axes(Qt::Vertical).first();
    chart->legend()->hide();
    axisx->setRange(0, 160);
    axisy->setRange(0, 5);
    ui->signalView->setChart(chart);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::updateView()
{
    if (display_mode == 0) {
        QVector<QPointF> points;
        points.reserve(values.size());
        for (int i = 0; i < values.size(); ++i) {
            points.append(QPointF(i / freq_sample, values[i]));
        }
        axisx->setRange(0, (values.size()-1) / freq_sample);
        axisy->setRange(*std::min_element(values.begin(),values.end())*1.2, *std::max_element(values.begin(),values.end())*1.2);
        series->replace(points);
        axisx->setTitleText("时间/s");
        axisy->setTitleText("幅度");
        chart->setTitle("信号时域图");
    } else if (display_mode == 1) {
        std::vector<std::complex<double>> freq_spectrum;
        freq_spectrum.resize(values.size());
        freq_spectrum.assign(values.begin(), values.end());
        fft(freq_spectrum, false);
        QVector<double> freq_mod;
        freq_mod.resize(freq_spectrum.size());
        for (int i = 0; i < freq_spectrum.size(); ++i) {
            freq_mod[i] = sqrt(freq_spectrum[i].real()*freq_spectrum[i].real()+freq_spectrum[i].imag()*freq_spectrum[i].imag());
        }
        QVector<QPointF> points;
        points.reserve(freq_mod.size());
        for (int i = 0; i < freq_mod.size(); ++i) {
            points.append(QPointF(i*freq_sample/(freq_mod.size()-1), freq_mod[i]));
        }
        axisx->setRange(0, freq_sample);
        axisy->setRange(0, *std::max_element(freq_mod.begin(),freq_mod.end())*1.2);
        series->replace(points);
        axisx->setTitleText("频率/f");
        axisy->setTitleText("幅度");
        chart->setTitle("信号幅度谱");
    }
    return;
}


double MainWindow::gaussianRandom(double mean, double stdDev) {
    double u = QRandomGenerator::global()->generateDouble();
    double v = QRandomGenerator::global()->generateDouble();
    double z = sqrt(-2 * log(u)) * cos(2 * M_PI * v);
    return mean + z * stdDev;
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
        double freq_sine = 0.0, amplitude = 0.0;
        for (int i = 0; i < cfg_values.size(); ++i) {
            if (cfg_values[i]==id) {
                freq_sample = cfg_values[i+1];
                freq_sine = cfg_values[i+2];
                amplitude = cfg_values[i+3];
                break;
            }
        }
        values.resize(freq_sample/freq_sine*50);
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


void MainWindow::on_pushButtonAddNoise_clicked()
{
    double mean = ui->lineEditNoiseMean->text().toDouble();
    double std = ui->lineEditNoiseStd->text().toDouble();
    datatype = 2;
    for (int i = 0; i < values.size(); ++i) {
        values[i] += gaussianRandom(mean, std);
    }
    updateView();
    return;
}


void MainWindow::on_lineEditId_editingFinished()
{
    settings->setValue("Id/id", ui->lineEditId->text());
}


void MainWindow::on_lineEditNoiseMean_editingFinished()
{
    settings->setValue("Noise/mean", ui->lineEditNoiseMean->text());
}


void MainWindow::on_lineEditNoiseStd_editingFinished()
{
    settings->setValue("Noise/standardDeviation", ui->lineEditNoiseStd->text());
}


void MainWindow::on_pushButtonDisplaySwitch_clicked()
{
    if (display_mode == 0) {
        display_mode = 1;
    } else {
        display_mode = 0;
    }
    updateView();
}

