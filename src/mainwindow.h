
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChartView>
#include <QVector>
#include <QSettings>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow

{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonReadFile_clicked();

    void on_pushButtonSaveFile_clicked();

    void on_pushButtonAddNoise_clicked();

    void on_lineEditId_editingFinished();

    void on_lineEditNoiseMean_editingFinished();

    void on_lineEditNoiseStd_editingFinished();

    void on_pushButtonDisplaySwitch_clicked();

private:
    void updateView();
    double gaussianRandom(double mean, double stdDev);
    virtual void resizeEvent(QResizeEvent *event) override;

    QSettings *settings;
    Ui::MainWindow *ui;
    QLineSeries *series;
    QChart *chart;
    QVector<double> values;
    double freq_sample;
    int datatype;  // 0: int16, 1: uint32, 2: float
    int display_mode = 0;  // 0: time, 1: freq
};

#endif // MAINWINDOW_H
