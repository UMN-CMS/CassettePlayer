#include "slots.h"

std::size_t CasSlot::getHash() const { return std::hash<AnyType>{}(value); }
