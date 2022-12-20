#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include <vector>
#include <QtCharts>
#include <iostream>
#include <QSerialPort>
#include <QSerialPortInfo>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void get_data();
    void update_urat();
    void on_pushButton_clicked();
    void initChart();
    void debug_add_num();
    void on_port_com_currentIndexChanged(const QString &arg1);

private:
    Ui::Widget *ui;
    QSerialPort *rm3100_serialPort;
    QTimer *URAT_update;
    QTimer *chart_update;
    int num_update_flag = 0;

    QStringList data_list;
    QString z_num_string;
    int z_num = 0;
    int time = 0;

    QLineSeries* x_series;
    QLineSeries* y_series;
    QLineSeries* z_series;

    std::vector<float> x_vector;
    std::vector<float> y_vector;
    std::vector<float> z_vector;
};
#endif // WIDGET_H
