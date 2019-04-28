#include "qtmeta.h"

namespace ai {

void registerQtMetaTypes()
{
    qRegisterMetaType<Individual>();
    qRegisterMetaType<FitnessRecord>();
}

} // namespace ai
