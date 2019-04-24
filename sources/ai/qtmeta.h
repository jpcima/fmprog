#pragma once
#include "ai.h"
#include <QObject>

Q_DECLARE_METATYPE(ai::Individual);

namespace ai {

void registerQtMetaTypes();

} // namespace ai
