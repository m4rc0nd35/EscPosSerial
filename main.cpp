#include <QCoreApplication>
#include "escposusb.h"

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);

  try{
      EscPosUSB *printer = new EscPosUSB();
      printer->connectPrinter(0x067b, 0x2303);
      qDebug() << "PRINTER INIT" << endl;
      printer->setFont(EscPosUSB::FontFamily::B);
      printer->getStatus(EscPosUSB::TransmitStatus::PapelRollStatus);
      printer->setJustification(EscPosUSB::Align::RIGHT);
      printer->text("justification");
      printer->endline();
      printer->setStyles(EscPosUSB::Style::Font2);
      printer->setJustification(EscPosUSB::Align::LEFT);
      printer->text("Align left");
      printer->endline();
      printer->setStyles(EscPosUSB::Style::Emphasized);
      printer->setJustification(EscPosUSB::Align::CENTER);
      printer->text("TESTE PrinterSerial");
      printer->endline();
      printer->setStyles(EscPosUSB::Style::Underline);
      printer->setJustification(EscPosUSB::Align::CENTER);
      printer->text("TESTE PrinterSerial");
      printer->endline();
      printer->barCode("123456789", EscPosUSB::Position::topBottom, EscPosUSB::FontFamily::A, EscPosUSB::BarCodeType::CODE39, 60, 20);
      printer->qrCode("http://nfce.set.rn.gov.br/consultarNFCe.aspx:p=24210937166065000111650010000162351161050451}2}1}2}352DA08F1F872DE16EDC657391EE6CFE5933FB10", EscPosUSB::Model::Model1, EscPosUSB::ModelSize::S1, EscPosUSB::ErrorCorrection::H);
      printer->endline();
      printer->cut();
      printer->printer();
      qDebug() << "PRINTER END" << endl;
  }catch(const char* e){
      qDebug() << "EXCEPINFO" << e << endl;
  }

  return a.exec();
}
