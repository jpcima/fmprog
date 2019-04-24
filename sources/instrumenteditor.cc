#include "instrumenteditor.h"
#include "ui_instrumenteditor.h"

InstrumentEditor::InstrumentEditor(QWidget *parent)
    : QFrame(parent),
      ui_(new Ui::InstrumentEditor)
{
    ui_->setupUi(this);

    ui_->operator1Editor->setOperatorNumber(0);
    ui_->operator2Editor->setOperatorNumber(1);
    ui_->operator3Editor->setOperatorNumber(2);
    ui_->operator4Editor->setOperatorNumber(3);
}

InstrumentEditor::~InstrumentEditor()
{
}

void InstrumentEditor::setValuesFromInstrument(const FmBank::Instrument &ins)
{
    ui_->alSlider->setValue(ins.algorithm);
    ui_->fbSlider->setValue(ins.feedback);
    ui_->fmsSlider->setValue(ins.fm);
    ui_->amsSlider->setValue(ins.am);

    ui_->operator1Editor->setValuesFromInstrument(ins);
    ui_->operator2Editor->setValuesFromInstrument(ins);
    ui_->operator3Editor->setValuesFromInstrument(ins);
    ui_->operator4Editor->setValuesFromInstrument(ins);
}

void InstrumentEditor::getInstrumentFromValues(FmBank::Instrument &ins)
{
    ins.algorithm = ui_->alSlider->value();
    ins.feedback = ui_->fbSlider->value();
    ins.fm = ui_->fmsSlider->value();
    ins.am = ui_->amsSlider->value();

    ui_->operator1Editor->getInstrumentFromValues(ins);
    ui_->operator2Editor->getInstrumentFromValues(ins);
    ui_->operator3Editor->getInstrumentFromValues(ins);
    ui_->operator4Editor->getInstrumentFromValues(ins);
}
