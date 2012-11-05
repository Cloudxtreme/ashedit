#include "general.h"

int General::areaSize = 20;

#ifdef MONSTERRPG2
int General::tileSize = 16;
int General::startLayers = 4;
int General::scale = 3;
#else
int General::tileSize = 64;
int General::startLayers = 2;
int General::scale = 1;
#endif

