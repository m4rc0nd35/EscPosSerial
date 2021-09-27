#include "escposusb.h"

bool EscPosUSB::connectPrinter(int vendor, int product)
{
    bool connected = false;
    try{
        m_serial = new QSerialPort();
        foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        {
            if(m_serial->open(QIODevice::ReadWrite))
                qDebug() << QString("[PRINTER] ERROR %1").arg(m_serial->errorString())<< endl;

            if(info.vendorIdentifier() == vendor && info.productIdentifier() == product)
            {
                m_serial->setPortName(info.portName());
                m_serial->setBaudRate(QSerialPort::Baud115200);
                m_serial->setDataBits(QSerialPort::Data8);
                m_serial->setParity(QSerialPort::NoParity);
                m_serial->setStopBits(QSerialPort::OneStop);
//                m_serial->setFlowControl(QSerialPort::NoFlowControl);

                if(m_serial->open(QIODevice::ReadWrite))
                {
                    connect(m_serial, SIGNAL(readyRead()), this, SLOT(readyRead()));
                    qDebug() << QString("[SUCCESS] CONNECTED PRINTER PORT: %1").arg(info.portName()) << endl;
                }else
                    qDebug() << "[CRITICAL] DISCONNECTED" << endl;
            }
        }
    } catch (const char* e){
        connected = false;
        qDebug() << e << endl;
        throw e;
    }
    return connected;
}
void EscPosUSB::qrCode(QString data, Model model, ModelSize moduleSize, ErrorCorrection erroCorrection)
{

    int iTam = data.length() + 3;
    int pL = (iTam % 256) ;
    int pH = iTam >> 8;
    // function 165 Model
    char mT[9] = {GS, '(', 'k', 0x04, 0x00, 0x31, 0x41, char(model), 0x0};
    _buffer.append(mT,9);
    // function 167 print speed
    const char mS[8] = {GS, '(', 'k', 0x03, 0x00, 0x31, 0x43, char(moduleSize)};
    _buffer.append(mS,8);
    // function 169
    // L = 0, M = 1, Q = 2, H = 3
    const char eL[8] = {GS, '(', 'k', 0x03, 0x00, 0x31, 0x45, char(erroCorrection)};
    _buffer.append(eL,8);
    // function 180 QR Store
    const char dH[8] = {GS, '(', 'k', char(pL), char(pH), 0x31, 0x50, 0x30};
    _buffer.append(dH,8);
    _buffer.append(data.mid(0, 7092));
    // function 181 Print QR code
    const char pP[8] = {GS, '(', 'k', 0x03, 0x00, 0x31, 0x51, 0x30};
    _buffer.append(pP,8);
    // function 182 Print QR code
    const char pT[8] = {GS, '(', 'k', 0x03, 0x00, 0x31, 0x52, 0x30};
    _buffer.append(pT,8);
    qDebug() << pL << pH << endl;
}
void EscPosUSB::barCode(QByteArray data, Position position, FontFamily font, BarCodeType barcode, int height, int width)
{
    const char pP[3] = {GS, 'H', char(position)};
    _buffer.append(pP,3);
    const char pF[3] = {GS, 'f', char(font)};
    _buffer.append(pF,3);
    const char pH[3] = {GS, 'h', char(height)};
    _buffer.append(pH,3);
    const char pW[3] = {GS, 'w', char(width)};
    _buffer.append(pW,3);
    const char pK[3] = {GS, 'k', char(barcode)};
    _buffer.append(pK,3);
    _buffer.append(data.count());
    _buffer.append(data);
}
void EscPosUSB::endline()
{
    _buffer.append(LF);
}
void EscPosUSB::getStatus(TransmitStatus status)
{
    const char cmd[3] = {DLE, EOT, char(status)};
    _buffer.append(cmd, 3);
}
void EscPosUSB::setLineSpacing(int spc)
{
    const char cmd[3] = {ESC, 0x33, char(spc)};
    _buffer.append(cmd, 3);
}
void EscPosUSB::image(QByteArray img)
{
    /* image("IM") image stored in printer flash */
    const char pI[7] = {GS, '(','L', ACK, 0x0, '0','E'};
    _buffer.append(pI,7);
    _buffer.append(img);
    const char pE[2] = {SOH, SOH};
    _buffer.append(pE,2);
}
void EscPosUSB::cut()
{
    _buffer.append(GS);
    _buffer.append("V");
    _buffer.append(66);
    _buffer.append(1);
}
void EscPosUSB::textList(QList<QString> l, bool c)
{
    for (int i=0; i < l.count(); i++)
    {
        _buffer.append(l[i]);
        if(c)
            cut();
        else
            if(l.count() > 1 && l.count() != (i+1))
            {
                text("----------------------------------------------------------------");
                endline();
            }
    }
}
void EscPosUSB::text(QByteArray s)
{
    _buffer.append(s);
}
void EscPosUSB::setFontSize(Size s)
{
    char sT[3] = {GS, '!', char(s)};
    _buffer.append(sT,3);
}
void EscPosUSB::setFont(FontFamily f)
{
    char sT[3] = {ESC, 0x4D, char(f)};
    _buffer.append(sT,3);
}
void EscPosUSB::setStyles(Style mode)
{
    /* mode 0~5 */
    char sT[3] = {ESC, '!', char(mode)};
    _buffer.append(sT,3);
}
void EscPosUSB::setJustification(Align mode)
{
    /* mode 0~2 */
    char mA[3] = {ESC, 'a', char(mode)};
    _buffer.append(mA,3);
}
void EscPosUSB::reset()
{
    /* return status */
    char sT[3] = {DLE, EOT, SOH};
    _buffer.append(sT,3);
    /* reset */
    char rT[2] = {ESC, '@'};
    _buffer.append(rT,2);
}
void EscPosUSB::colsText(QList<int> cols, QStringList list, int limit)
{ // cols = [start, ..., stop], list = values, max char per colun(default 64)
    QString ret;
    for(int i = 0; cols.count() > i; i++)
        if((cols.count() - 1) == i)
            ret.insert((cols[i] - list[i].count()), list[i]);
        else
            ret.insert(cols[i], list[i].mid(0,limit));

    _buffer.append(ret);
}
void EscPosUSB::column(int x, int y, int c1, int c2)
{
    int iTam = (y + x + c1 + c2);
    int pL = (iTam % 256) ;
    int pH = iTam >> 8;
    _buffer.append(GS);
    _buffer.append("P");
    _buffer.append(pL); // pL
    _buffer.append(pH); // pH
    _buffer.append(9); // fn
    _buffer.append(y); // y
    _buffer.append(c1); // c1
    _buffer.append(c2); // c2
    _buffer.append(x); // d
}
void EscPosUSB::printer()
{
    try {
        m_serial->write(_buffer);
        m_serial->flush();
        qDebug() << _buffer << endl;
        _buffer.clear();
    } catch (const char* e) {
        throw e;
    }
}
void EscPosUSB::readyRead()
{
    QByteArray rData = m_serial->readAll();
    qDebug() << rData << endl;
}
