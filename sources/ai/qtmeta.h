#pragma once
#include "ai.h"
#include "algorithm.h"
#include <QObject>

Q_DECLARE_METATYPE(ai::Individual);
Q_DECLARE_METATYPE(ai::FitnessRecord);

namespace ai {

void registerQtMetaTypes();

} // namespace ai
