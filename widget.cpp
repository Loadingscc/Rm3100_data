#include <QDebug>
#include <QTimer>
#include <QtCharts>
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

QT_CHARTS_USE_NAMESPACE

#include "widget.h"
#include "ui_widget.h"

#define is_debug false

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    rm3100_serialPort = new QSerialPort();  // 创建新的串口端口
    initChart();

    URAT_update = new QTimer(this);
    connect(URAT_update,&QTimer::timeout,this,&Widget::update_urat);
    URAT_update->start(1000);

    chart_update = new QTimer(this);
    connect(chart_update,&QTimer::timeout,this,&Widget::debug_add_num);
    chart_update->start(1000);
}

Widget::~Widget()
{
    qDebug() << "项目运行结束";
    delete ui;
}

void Widget::initChart()
{
    z_series = new QLineSeries();  // 创建Z轴
    z_series->setName("Z轴数据");
    ui->chartView->setRenderHint(QPainter::Antialiasing);  // 设置抗锯齿
}

void Widget::debug_add_num(){
//    z_series->clear();
    # if is_debug
        std::vector<int> zp_vector;

        for (int i = 0; i < 10; i++){
            int num = rand()%100;
            zp_vector.push_back(num);
        }

        int time = 0;
        for (auto it : zp_vector){
            qDebug() << z_num;
            *z_series << QPointF(time, z_num);
            time++;
        }

    #else

        *z_series << QPointF(time, z_num);
        time++;

    #endif
    if (num_update_flag == 0){
        QChart* chart = ui->chartView->chart();  // 获取QchartView内置的chart
        chart->addSeries(z_series);
        chart->createDefaultAxes();   // 基于已添加到图表中的series为图表创建轴。以前添加到图表中的任何轴都将被删除。
        QList<QAbstractAxis*> axisX = chart->axes(Qt::Horizontal); // 获取创建的X轴
        QList<QAbstractAxis*> axisY = chart->axes(Qt::Vertical);   // 获取创建的X轴
        if(!axisX.isEmpty()) axisX.first()->setRange(0, 1000);       // 设置X轴范围（设置前需要判断是否有X轴，避免数组越界）
        if(!axisY.isEmpty()) axisY.first()->setRange(0, 10000);       // 设置Y轴范围
        chart->setTitle("RM3100 数据折线图");  // 设置标题
        num_update_flag = 1;
    }
}

void Widget::update_urat(){
    // 检测可使用串口，并显示在combox中
    ui->port_com->clear();
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos)
    {
        QSerialPort serial;
        serial.setPort(info);
        //如果某个串口打开，读取正常，统统关闭
        if(serial.open(QIODevice::ReadWrite))
        {
            ui->port_com->addItem(info.portName());
            serial.close();
        }
    }
}


void Widget::get_data(){
    QByteArray buf; //二进制
    buf=rm3100_serialPort->readAll();
    if(!buf.isEmpty())
    {
        QString str=buf;
        ui->data_edit->append(str);
        if (str.length() > 15){
            data_list = str.split(",");
            if (data_list.length() > 3){
                z_num_string = data_list[2].replace("Magz:", "");
                ui->znum_label->setText(z_num_string);
                z_num = z_num_string.toInt();
                z_vector.push_back(z_num);
            }
        }
    }
    buf.clear();
}


void Widget::on_pushButton_clicked()
{
    if (ui->pushButton->text() == "打开串口"){
        qDebug() << "打开串口";
        URAT_update->stop();
        rm3100_serialPort->setPortName(ui->port_com->currentText());//当前选择的串口名字
        ui->port_com->setEnabled(false);  // 端口号comboBox冻结
        ui->baud_com->setEnabled(false);  // 波特率comboBox冻结
        if(!rm3100_serialPort->open(QIODevice::ReadWrite))//用ReadWrite 的模式尝试打开串口
        {
            QMessageBox::warning(this, tr("警告"),tr("连接失败"), QMessageBox::Yes);
            URAT_update->start();
            return;
        }
        qDebug()<<"串口打开成功!";
        ui->pushButton->setText("关闭串口");
        if (ui->pushButton->text() == "打开串口"){
            URAT_update->start();
        }

        // 波特率筛选
        switch(ui->baud_com->currentText().toInt()){
            case 1200: rm3100_serialPort->setBaudRate(QSerialPort::Baud1200,QSerialPort::AllDirections); break;
            case 2400: rm3100_serialPort->setBaudRate(QSerialPort::Baud2400,QSerialPort::AllDirections); break;
            case 4800: rm3100_serialPort->setBaudRate(QSerialPort::Baud4800,QSerialPort::AllDirections); break;
            case 9600: rm3100_serialPort->setBaudRate(QSerialPort::Baud9600,QSerialPort::AllDirections); break;
            case 19200: rm3100_serialPort->setBaudRate(QSerialPort::Baud19200,QSerialPort::AllDirections); break;
            case 38400: rm3100_serialPort->setBaudRate(QSerialPort::Baud38400,QSerialPort::AllDirections); break;
            case 57600: rm3100_serialPort->setBaudRate(QSerialPort::Baud57600,QSerialPort::AllDirections); break;
            case 115200: rm3100_serialPort->setBaudRate(QSerialPort::Baud115200,QSerialPort::AllDirections); break;
        }

        rm3100_serialPort->setDataBits(QSerialPort::Data8);  // 数据位8位
        rm3100_serialPort->setStopBits(QSerialPort::OneStop);  // 停止位为1
        rm3100_serialPort->setFlowControl(QSerialPort::NoFlowControl);  // 无流控制
        rm3100_serialPort->setParity(QSerialPort::NoParity);  // 无校验位

        connect(rm3100_serialPort,SIGNAL(readyRead()),this,SLOT(receiveInfo()));  // 信号与槽链接，链接一个获取数据的信号
        QObject::connect(rm3100_serialPort,&QSerialPort::readyRead,this,&Widget::get_data);
    }
    else{
        rm3100_serialPort->close();
        ui->port_com->setEnabled(true);  // 端口号comboBox解冻
        ui->baud_com->setEnabled(true);  // 波特率comboBox解冻
        ui->pushButton->setText("打开串口");
    }
}


void Widget::on_port_com_currentIndexChanged(const QString &arg1)
{
    qDebug() << arg1;
    if (URAT_update->isActive()){
        URAT_update->stop();
    }
    return;
}
