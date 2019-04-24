#pragma once
#include "instrument/bank.h"
#include <QFrame>
#include <memory>

namespace Ui { class OperatorEditor; }

class OperatorEditor : public QFrame
{
public:
    explicit OperatorEditor(QWidget *parent = nullptr);
    ~OperatorEditor();

    void setOperatorNumber(int op);

    void setValuesFromInstrument(const FmBank::Instrument &ins);
    void getInstrumentFromValues(FmBank::Instrument &ins);

private:
    std::unique_ptr<Ui::OperatorEditor> ui_;
    int op_ = 0;
};
