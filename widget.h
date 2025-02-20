#ifndef WIDGET_H
#define WIDGET_H

#include <cstring>

#include <QWidget>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QComboBox>
#include <QRegularExpressionValidator>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QButtonGroup>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QFontMetrics>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QMap>
#include <QFile>
#include <QDateTime>
#include <QIODevice>
#include <QObject>
#include <QScreen>
#include <QPixmap>
#include <QFileDialog>
#include <QGuiApplication>

class Widget : public QWidget
{
    Q_OBJECT
public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

public:
    uint8_t base_data[14] = {0x7e, 0xe7, 0x7e, 0x01, 0x01, 0x01, 0x00, 0x02, 0x10, 0x00, 0x01, 0x07, 0x0d};

private slots:
    void onSelectAllCheckBoxChanged(int state);
    void onRowCountChanged(int value);
    void onOpenCloseButtonClicked();
    void onStartPauseButtonClicked();
    void onTimerTimeout();
    void readSerialPortData();
    void openBaffle();
    void closeBaffle();
    void onCaptureScreenButtonClicked();

private:
    void updateTableRows(int rows);
    void setupTableWidgetRow(int row, const QList<QSerialPortInfo>& serialPortInfos);
    void setColumnsNonEditable(int row);   
    void createFirstRowLayout(QVBoxLayout *mainLayout);    // 创建第一行布局（全选框和数字输入框）
    void createSerialPortTable(QVBoxLayout *mainLayout);// 创建串口列表
    void createSecondRowLayout(QVBoxLayout *mainLayout);    // 创建第二行布局（实时数据发送label和lineEdit）
    void createThirdRowLayout(QVBoxLayout *mainLayout);// 创建第三行布局（打开挡板和关闭挡板按钮）
    void createFourthRowLayout(QVBoxLayout *mainLayout);// 创建第四行布局（打开串口和开始运行按钮）
    void setcoLumnWidth();//设置图表列宽
    bool getSerialPortConfig(int row , QString &portName , qint32 &baudRate , QSerialPort::DataBits &dataBits , QSerialPort::StopBits &stopBits, QSerialPort::Parity &parity);//获取串口配置
    bool configAndOpenSerialPort(const QString &portName, qint32 baudRate, QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits, QSerialPort::Parity parity);//配置闭关打开串口
    void closeAllSerialPort();//关闭所有串口
    QByteArray generateCommand(int command , int dataHigh ,   int dataLow );//生成命令
    void setWindowSize();//变更窗口
    void sendLightBrightnessCommands();
    QString formatHexString(const QString &hexString);
    void updataTableViewColum();
    void parseReceivedData(const QByteArray &data , QSerialPort * serialPort , const char *base_data);

private:
    QLabel *selectDeviceCountLabel;
    QTableWidget *serialPortTable;
    QCheckBox *selectAllCheckBox;       // 全选
    QPushButton *openCloseButton;       // 开关串口
    QPushButton *startPauseButton;      // 开始暂停
    QComboBox *comboBox;
    QSpinBox *rowCountSpinBox; // 新增数字输入框
    QLabel *realTimeDataLabel;
    QButtonGroup* frictionGroup;
    QRadioButton *yesRadio;
    QRadioButton *noRadio;
    QHBoxLayout *frictionLayout;
    QWidget *frictionWidget;

    QLineEdit* lineEdit;
    QSpinBox *lightBrightnessSpinBox;

    QCheckBox *checkBox;

    QFile *logfile;

    QSerialPort serialPort;
    QList<QSerialPort * >serialPorts;
    QMap<QSerialPort* , int>portToRowMap;
    QMap<QSerialPort* , QByteArray> serialPortReceivedDataMap;
    QMap<QSerialPort*, int> communicationCountOpenMap;
    QMap<QSerialPort*, int> communicationCountCloseMap;
    QMap<QSerialPort*, int> communicationFailCountMap;

    QTimer timer;

    QRegularExpressionValidator *baudRateValidator;
    QRegularExpressionValidator *dataBitsValidator;
    QRegularExpressionValidator *stopBitsValidator;
    QRegularExpressionValidator *parityValidator;

    bool isSerialPortOpen;
    bool isRunning  = false;
    int commandIndex;

    int upperLimit;
    int lowerLimit;
};

#endif // WIDGET_H
