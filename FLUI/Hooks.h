#pragma once

#include "TrimmedFLCoreCommon.h"

extern std::unordered_map<uint, std::vector<uint>> orderedBaseShipMap;
void SetPrevShips();
void SetNextShips();
void InitShipDealerHooks();
bool HasPrevShips();
bool HasNextShips();