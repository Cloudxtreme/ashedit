#include "general.h"

int General::areaSize = 20;

#ifdef MONSTERRPG2
int General::tileSize = 16;
int General::startLayers = 4;
bool General::can_add_and_delete_layers = true;
int General::scale = 2;
#else
int General::tileSize = 64;
int General::startLayers = 2;
bool General::can_add_and_delete_layers = true;
int General::scale = 1;
#endif

