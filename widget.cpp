#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent), isSerialPortOpen(false)
{

    setWindowIcon(QIcon(":/Dangban.ico"));
    setWindowTitle(u8"挡板测试工具");
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    createFirstRowLayout(mainLayout);
    createSerialPortTable(mainLayout);
    createSecondRowLayout(mainLayout);
    createThirdRowLayout(mainLayout);
    createFourthRowLayout(mainLayout);

    connect(openCloseButton, &QPushButton::clicked, this, &Widget::onOpenCloseButtonClicked);
    connect(startPauseButton,&QPushButton::clicked,this,&Widget::onStartPauseButtonClicked);

    timer.setInterval(1000);
    connect(&timer , &QTimer::timeout , this , &Widget::onTimerTimeout);

    // 获取可用串口列表
    QList<QSerialPortInfo> serialPortInfos = QSerialPortInfo::availablePorts();

    // 定义输入验证器
    baudRateValidator = new QRegularExpressionValidator(QRegularExpression("^[1-9]\\d{0,6}$"), this);
    dataBitsValidator = new QRegularExpressionValidator(QRegularExpression("^[5-8]$"), this);
    stopBitsValidator = new QRegularExpressionValidator(QRegularExpression("^[1-2]$"), this);
    parityValidator = new QRegularExpressionValidator(QRegularExpression("^(None|Odd|Even)$"), this);

    // 初始化串口列表
    for (int i = 0; i < 10; ++i) {
        setupTableWidgetRow(i, serialPortInfos);
    }

    setcoLumnWidth();

    setWindowSize();
}

Widget::~Widget()
{
    closeAllSerialPort();

    delete baudRateValidator;
    delete dataBitsValidator;
    delete stopBitsValidator;
    delete parityValidator;
}

void Widget::createFirstRowLayout(QVBoxLayout *mainLayout) {
    QHBoxLayout *firstRowLayout = new QHBoxLayout();

    // 添加选择设备数量的label
    selectDeviceCountLabel = new QLabel(u8"选择设备数量:", this);
    QFontMetrics fm(selectDeviceCountLabel->font());
    int textWidth = fm.horizontalAdvance(selectDeviceCountLabel->text());
    selectDeviceCountLabel->setFixedWidth(textWidth);  // 设置label的宽度为文字宽度
    firstRowLayout->addWidget(selectDeviceCountLabel);

    // 创建数字输入框
    rowCountSpinBox = new QSpinBox(this);
    rowCountSpinBox->setRange(1, 16); // 设置输入范围
    rowCountSpinBox->setValue(10); // 设置初始值
    firstRowLayout->addWidget(rowCountSpinBox);

    // 创建全选复选框
    selectAllCheckBox = new QCheckBox(u8"全选", this);
    firstRowLayout->addWidget(selectAllCheckBox);
    connect(selectAllCheckBox, &QCheckBox::stateChanged, this, &Widget::onSelectAllCheckBoxChanged);

    connect(rowCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Widget::onRowCountChanged);


        QPushButton *captureScreenButton  = new QPushButton(u8"截取当前窗口",this);
        firstRowLayout->addWidget(captureScreenButton );
        connect(captureScreenButton ,&QPushButton::clicked,this,&Widget::onCaptureScreenButtonClicked);

    //创建重置窗口大小按钮
    QPushButton *resetWindowSizeButton = new QPushButton(u8"重置窗口大小", this);
    firstRowLayout->addWidget(resetWindowSizeButton);
    connect(resetWindowSizeButton, &QPushButton::clicked, this, &Widget::setWindowSize);

    mainLayout->addLayout(firstRowLayout);
}

void Widget::createSerialPortTable(QVBoxLayout *mainLayout) {
    serialPortTable = new QTableWidget(10, 13, this);
    serialPortTable->setHorizontalHeaderLabels({u8"选择"
                                                , u8"串口名称"
                                                , u8"波特率"
                                                , u8"数据位"
                                                , u8"停止位"
                                                , u8"奇偶校验"
                                                ,u8"灯亮度"
                                                ,u8"上限次数"
                                                ,u8"下限次数"
                                                ,u8"运行次数"
                                                , u8"通信失败"
                                                ,u8"电机与上下表面是否有摩擦"
                                                ,u8"电机偏转角度是否正常"});
    mainLayout->addWidget(serialPortTable);

    QFontMetrics fm(serialPortTable->horizontalHeader()->font());
    serialPortTable->setColumnWidth(6, fm.horizontalAdvance(u8"灯亮度") + 10);
    serialPortTable->setColumnWidth(7, fm.horizontalAdvance(u8"上限次数") + 10);
    serialPortTable->setColumnWidth(8, fm.horizontalAdvance(u8"下限次数") + 10);
    serialPortTable->setColumnWidth(9, fm.horizontalAdvance(u8"运行次数") + 10);
    serialPortTable->setColumnWidth(10, fm.horizontalAdvance(u8"通信失败") + 10);
    serialPortTable->setColumnWidth(11, fm.horizontalAdvance(u8"电机与上下表面是否有摩擦") + 10);
    serialPortTable->setColumnWidth(12, fm.horizontalAdvance(u8"电机偏转角度是否正常") + 10);

    // 设置第8 - 11列不可编辑
    for (int row = 0; row < serialPortTable->rowCount(); ++row) {
        setColumnsNonEditable(row);
    }
}

void Widget::createSecondRowLayout(QVBoxLayout *mainLayout) {
    QHBoxLayout *secondRowLayout = new QHBoxLayout();
    realTimeDataLabel = new QLabel(u8"实时数据发送", this);
    lineEdit = new QLineEdit(this);
    secondRowLayout->addWidget(realTimeDataLabel);
    secondRowLayout->addWidget(lineEdit);
    mainLayout->addLayout(secondRowLayout);
}

void Widget::createThirdRowLayout(QVBoxLayout *mainLayout)
{
    QHBoxLayout *thirdRowLayout = new QHBoxLayout();
    QPushButton *openBaffleButton = new QPushButton(u8"打开挡板", this);
    QPushButton *closeBaffleButton = new QPushButton(u8"关闭挡板", this);
    thirdRowLayout->addWidget(openBaffleButton);
    thirdRowLayout->addWidget(closeBaffleButton);
    connect(openBaffleButton , &QPushButton::clicked , this , &Widget::openBaffle);
    connect(closeBaffleButton , &QPushButton::clicked , this , &Widget::closeBaffle);
    mainLayout->addLayout(thirdRowLayout);
}

void Widget::createFourthRowLayout(QVBoxLayout *mainLayout)
{
    QHBoxLayout *fourthRowLayout = new QHBoxLayout();
    openCloseButton = new QPushButton(u8"打开串口", this);
    startPauseButton = new QPushButton(u8"开始运行", this);
    startPauseButton->setEnabled(false);
    openCloseButton->setFixedHeight(40);
    startPauseButton->setFixedHeight(40);
    fourthRowLayout->addWidget(openCloseButton);
    fourthRowLayout->addWidget(startPauseButton);
    mainLayout->addLayout(fourthRowLayout);
}

void Widget::setcoLumnWidth()
{
    serialPortTable->setColumnWidth(0, 40);
    serialPortTable->setColumnWidth(1, 65);
    serialPortTable->setColumnWidth(2, 65);
    serialPortTable->setColumnWidth(3, 40);
    serialPortTable->setColumnWidth(4, 40);
    serialPortTable->setColumnWidth(5, 65);
}

bool Widget::getSerialPortConfig(int row, QString &portName, qint32 &baudRate, QSerialPort::DataBits &dataBits, QSerialPort::StopBits &stopBits, QSerialPort::Parity &parity)
{
    QComboBox *comboBox = qobject_cast<QComboBox*>(serialPortTable->cellWidget(row, 1));
    if (!comboBox)
    {
        return false;
    }
    portName = comboBox->currentText();

    QComboBox* baudRateCombobox = qobject_cast<QComboBox*>(serialPortTable->cellWidget(row, 2));
    if (!baudRateCombobox)
    {
        return false;
    }
    QString baudRateStr = baudRateCombobox->currentText();
    bool ok;
    baudRate = baudRateStr.toInt(&ok);
    if (!ok)
    {
        return false;
    }

    QComboBox *dataBitsComboBox = qobject_cast<QComboBox*>(serialPortTable->cellWidget(row, 3));
    if (!dataBitsComboBox)
    {
        return false;
    }
    QString dataBitsStr = dataBitsComboBox->currentText();
    if (dataBitsStr == u8"5")
    {
        dataBits = QSerialPort::Data5;
    } else if (dataBitsStr == u8"6")
    {
        dataBits = QSerialPort::Data6;
    } else if (dataBitsStr == u8"7")
    {
        dataBits = QSerialPort::Data7;
    } else if (dataBitsStr == u8"8")
    {
        dataBits = QSerialPort::Data8;
    } else
    {
        return false;
    }

    QComboBox *stopBitsComboBox = qobject_cast<QComboBox*>(serialPortTable->cellWidget(row, 4));
    if (!stopBitsComboBox)
    {
        return false;
    }
    QString stopBitsStr = stopBitsComboBox->currentText();
    if (stopBitsStr == "1")
    {
        stopBits = QSerialPort::OneStop;
    } else if (stopBitsStr == "2")
    {
        stopBits = QSerialPort::TwoStop;
    } else
    {
        return false;
    }

    QComboBox *parityComboBox = qobject_cast<QComboBox*>(serialPortTable->cellWidget(row, 5));
    if (!parityComboBox)
    {
        return false;
    }
    QString parityStr = parityComboBox->currentText();
    if (parityStr == "None")
    {
        parity = QSerialPort::NoParity;
    } else if (parityStr == "Odd")
    {
        parity = QSerialPort::OddParity;
    } else if (parityStr == "Even")
    {
        parity = QSerialPort::EvenParity;
    } else
    {
        return false;
    }

    return true;
}

bool Widget::configAndOpenSerialPort(const QString &portName, qint32 baudRate, QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits, QSerialPort::Parity parity)
{
    serialPort.setPortName(portName);
    serialPort.setBaudRate(baudRate);
    serialPort.setDataBits(dataBits);
    serialPort.setStopBits(stopBits);
    serialPort.setParity(parity);

    if (serialPort.open(QIODevice::ReadWrite))
    {
        isSerialPortOpen = true;
        openCloseButton->setText(u8"关闭所有串口");
        startPauseButton->setEnabled(true);

        // 打开串口后查询所有状态
        QByteArray queryAllStatusCmd = generateCommand(0x04, 0x00, 0x00);
        serialPort.write(queryAllStatusCmd);

        return true;
    } else
    {
        QMessageBox::warning(this, u8"串口打开失败", u8"无法打开所选串口，请检查设置和设备连接。");
        return false;
    }
}

void Widget::closeAllSerialPort()
{
    for(QSerialPort* serialPort : serialPorts){
        if(serialPort->isOpen())
        {
            serialPort->close();
            qDebug() <<  "serialPort" <<serialPort->portName() << u8"is closed";
        }
        delete serialPort;
    }
    serialPorts.clear();
    isSerialPortOpen = false;
    openCloseButton->setText(u8"打开串口");
    startPauseButton->setEnabled(false);
}

//生成命令
QByteArray Widget::generateCommand(int command, int dataHigh, int dataLow)
{
    QByteArray frame;
    frame.append(static_cast<char>(0x7E)); // 帧头
    frame.append(static_cast<char>(0xE7));
    frame.append(static_cast<char>(0x7E));

    frame.append(static_cast<char>(0x01)); // 地址
    frame.append(static_cast<char>(0x01));

    frame.append(static_cast<char>(command)); // 命令

    frame.append(static_cast<char>(0x00)); // 数据长度
    frame.append(static_cast<char>(0x02));

    frame.append(static_cast<char>(dataHigh)); // 数据
    frame.append(static_cast<char>(dataLow));

    // 计算异或校验
    int xorChecksum = 0;
    for (int i = 0; i < frame.size(); ++i) {
        xorChecksum ^=frame[i];
    }
    frame.append(xorChecksum);

    // 计算和校验
    int sumChecksum = 0;
    for (int i = 1; i < frame.size(); ++i) {
        sumChecksum += frame[i];
    }
    frame.append(sumChecksum);

    frame.append(0x0D); // 帧尾

    return frame;
}

//重置窗口大小
void Widget::setWindowSize()
{
    // 获取主布局
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (!mainLayout)
    {
        return;
    }

    // 计算表格宽度
    int tableWidth = serialPortTable->horizontalHeader()->length() + serialPortTable->verticalHeader()->width() + 2 * serialPortTable->frameWidth();
    int layoutMarginLeft = mainLayout->contentsMargins().left();
    int layoutMarginRight = mainLayout->contentsMargins().right();
    int totalWidth = tableWidth + layoutMarginLeft + layoutMarginRight;

    // 计算表格高度
    int tableHeight = 0;
    // 获取行数
    int rowCount = serialPortTable->rowCount();
    // 累加每一行的高度
    for (int i = 0; i < rowCount; ++i) {
        tableHeight += serialPortTable->rowHeight(i);
    }
    // 加上表头高度
    tableHeight += serialPortTable->horizontalHeader()->height();
    // 加上表格的上下边框宽度
    tableHeight += 2 * serialPortTable->frameWidth();

    // 计算其他布局的高度
    int firstRowHeight = mainLayout->itemAt(0)->layout()->sizeHint().height(); // 第一行布局高度
    int secondRowHeight = mainLayout->itemAt(2)->layout()->sizeHint().height(); // 第二行布局高度
    int thirdRowHeight = mainLayout->itemAt(3)->layout()->sizeHint().height(); // 第三行布局高度
    int fourthRowHeight = mainLayout->itemAt(4)->layout()->sizeHint().height(); // 第四行布局高度

    int totalHeight = firstRowHeight + secondRowHeight + thirdRowHeight + tableHeight + fourthRowHeight + mainLayout->contentsMargins().top() + mainLayout->contentsMargins().bottom() + 25;

    // 设置窗口大小
    this->resize(totalWidth, totalHeight);
}

//发送灯亮度命令
void Widget::sendLightBrightnessCommands()
{
    for(QList<QSerialPort*>::iterator it = serialPorts.begin(); it != serialPorts.end(); ++it){
        QSerialPort* port = *it;
        int row = portToRowMap.value(port , -1);
        if(row != -1)
        {
            QSpinBox * lightBrightnessSpinBox = qobject_cast< QSpinBox* >(serialPortTable->cellWidget(row,6));
            if(lightBrightnessSpinBox)
            {
                int brightness = lightBrightnessSpinBox->value();
                QByteArray setLightness = generateCommand(0x03 , brightness , 0x00);
                if(port->isOpen())
                {
                    if(port->write(setLightness) == -1)
                    {
                        qDebug() <<"Failed to write to serial port:" << port->portName();
                    }else
                    {
                        qDebug() << "Serial port:" << port->portName() << "Brightness value:" << brightness;
                        QString message = setLightness.toHex();
                        QString spacedHexString = formatHexString(message);
                        lineEdit->setText(spacedHexString);
                        qDebug() <<  "Command sent to" << port->portName() << spacedHexString;
                    }
                }
            }
        }
    }
}

//格式转换
QString Widget::formatHexString(const QString &hexString)
{
    QString spacedHexString;
    for (int i = 0; i < hexString.length(); i += 2) {
        if (i + 2 <= hexString.length())
        {
            spacedHexString += hexString.mid(i, 2).toUpper() + " ";
        } else
        {
            spacedHexString += hexString.mid(i).toUpper();
        }
    }
    return spacedHexString;
}

void Widget::updataTableViewColum()
{
    for (int row = 0; row < serialPortTable->rowCount(); ++row) {
        QTableWidgetItem *item = serialPortTable->item(row, 7);
        if (item)
        {
            int currentValue = item->text().toInt();
            item->setText(QString::number(currentValue + 1));  // 简单地将上限次数加1
        }
    }
}


void Widget::parseReceivedData(const QByteArray &data, QSerialPort *serialPort, const char *base_data)
{
    if(data.size() >= 3&& 0== std::memcmp(data.constData() , base_data , 3))
    {
        char command = data[5];
        int row = portToRowMap.value(serialPort, -1);
        if(row!= -1)
        {
            if(command == 0x01)
            {
                serialPortReceivedDataMap[serialPort].clear();
                // 更新表格中的上限次数
                QTableWidgetItem *upperLimitItem = serialPortTable->item(row, 7);
                if (upperLimitItem) {
                    int currentValue = upperLimitItem->text().toInt();
                    upperLimitItem->setText(QString::number(currentValue + 1));
                }
                // 更新打开通信次数
                communicationCountOpenMap[serialPort] = communicationCountOpenMap.value(serialPort, 0) + 1;
            }else if(command == 0x02)
            {
                serialPortReceivedDataMap[serialPort].clear();

                // 更新表格中的下限次数
                QTableWidgetItem *lowerLimitItem = serialPortTable->item(row, 8);
                if (lowerLimitItem) {
                    int currentValue = lowerLimitItem->text().toInt();
                    lowerLimitItem->setText(QString::number(currentValue + 1));
                }
                // 更新关闭通信次数
                communicationCountCloseMap[serialPort] = communicationCountCloseMap.value(serialPort, 0) + 1;
            }
            // 更新通信次数
            int maxCommunicationCount = qMin(communicationCountOpenMap.value(serialPort, 0), communicationCountCloseMap.value(serialPort, 0));
            QTableWidgetItem *totalCommunicationItem = serialPortTable->item(row, 9);
            if (!totalCommunicationItem)
            {
                totalCommunicationItem = new QTableWidgetItem();
                serialPortTable->setItem(row, 9, totalCommunicationItem);
            }
            totalCommunicationItem->setText(QString::number(maxCommunicationCount));

        }else
        {
            int row = portToRowMap.value(serialPort, -1);
            if (row != -1)
            {
                QTableWidgetItem* communicationFailItem = serialPortTable->item(row, 10);
                if (communicationFailItem)
                {
                    communicationFailCountMap[serialPort] = communicationFailCountMap.value(serialPort, 0) + 1;
                    communicationFailItem->setText(QString::number(communicationFailCountMap.value(serialPort, 0)));
                }
            }
        }
    }
}

    //全选设备
    void Widget::onSelectAllCheckBoxChanged(int state)
    {
        for (int i = 0; i < serialPortTable->rowCount(); ++i) {
            QCheckBox *checkBox = qobject_cast<QCheckBox*>(serialPortTable->cellWidget(i, 0));
            if (checkBox)
            {
                checkBox->setChecked(state == Qt::Checked);
            }
        }
    }

    //设备数量改变，表格该表
    void Widget::onRowCountChanged(int value)
    {
        updateTableRows(value);
    }

    //开关串口
    void Widget::onOpenCloseButtonClicked()
    {
        if(isSerialPortOpen)
        {
            closeAllSerialPort();
            portToRowMap.clear();
        }else
        {
            serialPorts.clear();
            for (int row = 0; row < serialPortTable->rowCount(); ++row) {
                QCheckBox *checkBox = qobject_cast<QCheckBox* >(serialPortTable->cellWidget(row,0));
                if(checkBox && checkBox->isChecked())
                {
                    QString portName;
                    qint32 baudRate;
                    QSerialPort::DataBits dataBits;
                    QSerialPort::StopBits stopBits;
                    QSerialPort::Parity parity;

                    if(getSerialPortConfig(row,portName,baudRate,dataBits,stopBits,parity))
                    {
                        QSerialPort* serialPort = new QSerialPort(this);
                        serialPort->setPortName(portName);
                        serialPort->setBaudRate(baudRate);
                        serialPort->setDataBits(dataBits);
                        serialPort->setParity(parity);

                        if(serialPort->open(QIODevice::ReadWrite))
                        {
                            isSerialPortOpen = true;
                            openCloseButton->setText(u8"关闭所有串口");
                            startPauseButton->setEnabled(true);
                            serialPorts.append(serialPort);
                            connect(serialPort, &QSerialPort::readyRead, this, &Widget::readSerialPortData);

                            portToRowMap.insert(serialPort , row);
                            QString str = "serialPort" + portName;
                            qDebug() << str << u8"isOpend";
                        }
                        else
                        {
                            QMessageBox::warning(this, u8"串口打开失败" ,  u8"无法代开串口" +portName);
                        }
                    }
                }
            }
            // 如果所有选中串口都打开失败，重置isSerialPortOpen
            if (serialPorts.isEmpty())
            {
                isSerialPortOpen = false;
                openCloseButton->setText(u8"打开串口");
                startPauseButton->setEnabled(false);
            }
        }
        serialPortTable->update();
    }

    //开始暂停
    void Widget::onStartPauseButtonClicked()
    {
        if( isRunning)
        {
            timer.stop();
            startPauseButton->setText(u8"开始运行");
            isRunning = false;
        }
        else
        {
            sendLightBrightnessCommands();
            timer.start();
            startPauseButton->setText(u8"停止运行");
            isRunning = true;
        }
    }

    void Widget::onTimerTimeout()
    {
        QByteArray command;
        if(commandIndex == 0)
        {
            command = generateCommand(0x01, 0x01, 0x00);
            commandIndex = 1;
        }
        else
        {
            command = generateCommand(0x02, 0x01, 0x00);
            commandIndex = 0;
        }

        for(QSerialPort* serialPort : serialPorts){
            if(serialPort->isOpen())
            {
                serialPort->write(command);
                QString message =command.toHex();
                QString spacedHexString;
                spacedHexString = formatHexString(message);
                qDebug() << "Command sent to" << serialPort->portName() << ":" << formatHexString( command.toHex());
                lineEdit->setText(spacedHexString);
            }
        }
    }

    //读取串口信息
    void Widget::readSerialPortData()
    {
        for(QSerialPort* serialPort:serialPorts){
            if(serialPort->isOpen())
            {
                QByteArray receivedData = serialPort->readAll();
                if(!receivedData.isEmpty())
                {
                    serialPortReceivedDataMap[serialPort] += receivedData;
                    QByteArray cachedData = serialPortReceivedDataMap[serialPort];

                    while (cachedData.length() >= 13) {
                        QByteArray dataToProcess = cachedData.left(13);

                        qDebug() << "receivedData : " << dataToProcess.length();
                        qDebug() << "Received data from" << serialPort->portName() << ":" << dataToProcess;
                        parseReceivedData(dataToProcess , serialPort, reinterpret_cast <const char*>(base_data));

                        cachedData = cachedData.mid(13);
                    }
                }/*else
                {
                    qDebug() <<"Read data from" << serialPort->portName() << ":" << receivedData.toHex();
                }*/
            }
        }
    }

    //开挡板
    void Widget::openBaffle()
    {
        QByteArray command;
        command = generateCommand(0x01, 0x01, 0x00);
        for(QSerialPort* serialPort:serialPorts){
            if(serialPort->isOpen()){
                serialPort->write(command);
                qDebug() <<"The bezel opend successfully";
            }
        }
    }

    //关挡板
    void Widget::closeBaffle()
    {
        for(QSerialPort* serialPort:serialPorts){
            QByteArray command;
            command = generateCommand(0x02, 0x01, 0x00);;
            if(serialPort->isOpen()){
                serialPort->write(command);
                qDebug() <<"The bezel closes successfully";
            }
        }
    }

    void Widget::onCaptureScreenButtonClicked()
    {
        //全屏截取
        /*QScreen *screen = QGuiApplication::primaryScreen();
        if(!screen){
            QMessageBox::warning(this , u8"截屏失败" , u8"无法获取屏幕信息");
            return;
        }
// 截取整个屏幕
    QPixmap screenshot = screen->grabWindow(0);

    // 生成保存文件的默认文件名
    QString fileName = QDateTime::currentDateTime().toString("yyyyMMddHHmmss") + ".png";

    // 弹出文件保存对话框
    QString filePath = QFileDialog::getSaveFileName(this, u8"保存截屏", fileName, "PNG Images (*.png);;JPEG Images (*.jpg *.jpeg)");
    if (filePath.isEmpty()) {
        return;
    }

    // 保存截屏文件
    if (screenshot.save(filePath)) {
        QMessageBox::information(this, u8"截屏成功", u8"截屏已保存到：" + filePath);
    } else {
        QMessageBox::warning(this, u8"截屏失败", u8"无法保存截屏文件");
    }*/

        //窗口截取
        QPixmap pixmap = this->grab();
        QString fileName =  QDateTime::currentDateTime().toString(u8"yy年MM月dd日HH:mm") + u8"挡板老化实验（台）"  +".png";
        QString filePath = QFileDialog::getSaveFileName(this , u8"保存截图" , fileName , "PNG Images (*.png);;JPEG Images (*.jpg *.jpeg)");
        if(pixmap.save(filePath)){
            QMessageBox::information(this , u8"截屏成功" ,  u8"截图保存至" + filePath);
        }else{
            QMessageBox::warning(this , u8"截屏失败" , u8"无法保留截屏文件");
        }
    }

    // 更新表格行数的函数
    void Widget::updateTableRows(int rows)
    {
        // 保存原有数据
        QList<QList<QWidget*>> cellWidgets;
        for (int i = 0; i < serialPortTable->rowCount(); ++i) {
            QList<QWidget*> rowWidgets;
            for (int j = 0; j < serialPortTable->columnCount(); ++j) {
                rowWidgets.append(serialPortTable->cellWidget(i, j));
            }
            cellWidgets.append(rowWidgets);
        }

        // 重新设置表格行数
        serialPortTable->setRowCount(rows);

        // 恢复原有数据
        for (int i = 0; i < qMin(rows, cellWidgets.size()); ++i) {
            for (int j = 0; j < serialPortTable->columnCount(); ++j) {
                if (i < cellWidgets.size() && j < cellWidgets[i].size())
                {
                    serialPortTable->setCellWidget(i, j, cellWidgets[i][j]);
                }
            }
        }

        // 如果新的行数大于原有行数，需要补充新的控件
        if (rows > cellWidgets.size())
        {
            QList<QSerialPortInfo> serialPortInfos = QSerialPortInfo::availablePorts();
            for (int i = cellWidgets.size(); i < rows; ++i) {
                setupTableWidgetRow(i, serialPortInfos);
            }
        }

        // 设置第8 - 11列不可编辑
        for (int row = 0; row < serialPortTable->rowCount(); ++row) {
            setColumnsNonEditable(row);
        }

        setWindowSize();
    }

    //初始化表格
    void Widget::setupTableWidgetRow(int row, const QList<QSerialPortInfo>& serialPortInfos)
    {
        QCheckBox *checkBox = new QCheckBox(this);
        serialPortTable->setCellWidget(row, 0, checkBox);

        QComboBox *comboBox = new QComboBox(this);
        for (const auto& info : serialPortInfos) {
            comboBox->addItem(info.portName());
        }

        // 获取表格的列宽度并设置下拉框的宽度
        int columnWidth = serialPortTable->columnWidth(1);
        comboBox->setFixedWidth(columnWidth);

        // 将串口选择的 QComboBox 设置到表格的第 1 列
        serialPortTable->setCellWidget(row, 1, comboBox);

        // 波特率
        QComboBox *baudRateComboBox = new QComboBox(this);
        baudRateComboBox->addItem(u8"9600");
        baudRateComboBox->addItem(u8"115200");
        baudRateComboBox->setEditable(true);
        baudRateComboBox->setValidator(baudRateValidator);
        serialPortTable->setCellWidget(row, 2, baudRateComboBox);

        // 数据位
        QComboBox *dataBitsComboBox = new QComboBox(this);
        dataBitsComboBox->addItem(u8"8");
        dataBitsComboBox->setEditable(true);
        dataBitsComboBox->setValidator(dataBitsValidator);
        serialPortTable->setCellWidget(row, 3, dataBitsComboBox);

        // 停止位
        QComboBox *stopBitsComboBox = new QComboBox(this);
        stopBitsComboBox->addItem(u8"1");
        stopBitsComboBox->setEditable(true);
        stopBitsComboBox->setValidator(stopBitsValidator);
        serialPortTable->setCellWidget(row, 4, stopBitsComboBox);

        // 奇偶校验
        QComboBox *parityComboBox = new QComboBox(this);
        parityComboBox->addItem(u8"None");
        parityComboBox->addItem(u8"Odd");
        parityComboBox->addItem(u8"Even");
        parityComboBox->setEditable(true);
        parityComboBox->setValidator(parityValidator);
        serialPortTable->setCellWidget(row, 5, parityComboBox);

        //灯亮度
        QSpinBox *lightBrightnessSpinBox = new QSpinBox(this);
        lightBrightnessSpinBox->setRange(0, 255); // 设置取值范围，例如 0 到 255，否则只能达到99
        lightBrightnessSpinBox->setValue(199);
        serialPortTable->setCellWidget(row, 6, lightBrightnessSpinBox);

        // 添加单选按钮组 “有、无” 到第 11 列
        QButtonGroup *frictionGroup = new QButtonGroup(this);
        QRadioButton *yesRadio = new QRadioButton(u8"有", this);
        QRadioButton *noRadio = new QRadioButton(u8"无", this);
        frictionGroup->addButton(yesRadio);
        frictionGroup->addButton(noRadio);
        noRadio->setChecked(true); // 默认选中 “无”
        QHBoxLayout *frictionLayout = new QHBoxLayout();
        frictionLayout->addWidget(yesRadio);
        frictionLayout->addWidget(noRadio);
        QWidget *frictionWidget = new QWidget();
        frictionWidget->setLayout(frictionLayout);
        serialPortTable->setCellWidget(row, 11, frictionWidget);

        // 添加单选按钮组 “正常、异常” 到第 12 列
        QButtonGroup *angleGroup = new QButtonGroup(this);
        QRadioButton *normalRadio = new QRadioButton(u8"正常", this);
        QRadioButton *abnormalRadio = new QRadioButton(u8"异常", this);
        angleGroup->addButton(normalRadio);
        angleGroup->addButton(abnormalRadio);
        normalRadio->setChecked(true); // 默认选中 “正常”
        QHBoxLayout *angleLayout = new QHBoxLayout();
        angleLayout->addWidget(normalRadio);
        angleLayout->addWidget(abnormalRadio);
        QWidget *angleWidget = new QWidget();
        angleWidget->setLayout(angleLayout);
        serialPortTable->setCellWidget(row, 12, angleWidget);

        //8 - 11列的代码
        for (int col = 7; col <= 10; ++col) {
            QTableWidgetItem *item = new QTableWidgetItem(QString::number(0));
            serialPortTable->setItem(row, col, item);
            item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            //        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        }
    }

    //设置表格不可手动编辑
    void Widget::setColumnsNonEditable(int row)
    {
        for (int col = 7; col <= 10; ++col) {
            QTableWidgetItem *item = serialPortTable->item(row, col);
            if (!item)
            {
                item = new QTableWidgetItem();
                serialPortTable->setItem(row, col, item);
            }
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        }
    }
