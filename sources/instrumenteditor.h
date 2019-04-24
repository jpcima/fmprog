#pragma once
#include "instrument/bank.h"
#include <QFrame>
#include <memory>

namespace Ui { class InstrumentEditor; }

class InstrumentEditor : public QFrame
{
public:
    explicit InstrumentEditor(QWidget *parent = nullptr);
    ~InstrumentEditor();

    void setValuesFromInstrument(const FmBank::Instrument &ins);
    void getInstrumentFromValues(FmBank::Instrument &ins);

private:
    std::unique_ptr<Ui::InstrumentEditor> ui_;
};
