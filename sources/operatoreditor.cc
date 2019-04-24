#include "operatoreditor.h"
#include "ui_operatoreditor.h"

OperatorEditor::OperatorEditor(QWidget *parent)
    : QFrame(parent),
      ui_(new Ui::OperatorEditor)
{
    ui_->setupUi(this);
}

OperatorEditor::~OperatorEditor()
{
}

void OperatorEditor::setOperatorNumber(int op)
{
    op_ = op;
    ui_->titleLabel->setText(tr("Operator %1").arg(op + 1));
}

void OperatorEditor::setValuesFromInstrument(const FmBank::Instrument &ins)
{
    unsigned realop = op_;
    realop = (realop == 1) ? 2 : (realop == 2) ? 1 : realop;

    ui_->amCheckBox->setChecked(ins.OP[realop].am_enable);
    ui_->dtSlider->setValue(ins.OP[realop].detune);
    ui_->fmSlider->setValue(ins.OP[realop].fmult);
    ui_->lvSlider->setValue(ins.OP[realop].level);
    ui_->rsSlider->setValue(ins.OP[realop].ratescale);
    ui_->atSlider->setValue(ins.OP[realop].attack);
    ui_->d1Slider->setValue(ins.OP[realop].decay1);
    ui_->d2Slider->setValue(ins.OP[realop].decay2);
    ui_->slSlider->setValue(ins.OP[realop].sustain);
    ui_->rrSlider->setValue(ins.OP[realop].release);
    ui_->sgSlider->setValue(ins.OP[realop].ssg_eg);
}

void OperatorEditor::getInstrumentFromValues(FmBank::Instrument &ins)
{
    unsigned realop = op_;
    realop = (realop == 1) ? 2 : (realop == 2) ? 1 : realop;

    ins.OP[realop].am_enable = ui_->amCheckBox->isChecked();
    ins.OP[realop].detune = ui_->dtSlider->value();
    ins.OP[realop].fmult = ui_->fmSlider->value();
    ins.OP[realop].level = ui_->lvSlider->value();
    ins.OP[realop].ratescale = ui_->rsSlider->value();
    ins.OP[realop].attack = ui_->atSlider->value();
    ins.OP[realop].decay1 = ui_->d1Slider->value();
    ins.OP[realop].decay2 = ui_->d2Slider->value();
    ins.OP[realop].sustain = ui_->slSlider->value();
    ins.OP[realop].release = ui_->rrSlider->value();
    ins.OP[realop].ssg_eg = ui_->sgSlider->value();
}
