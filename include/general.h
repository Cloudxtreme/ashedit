#ifndef GENERAL_H
#define GENERAL_H

#include <allegro5/allegro.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

class General {
public:
	static int tileSize;
	static int areaSize;
	static int startLayers;
	static bool can_add_and_delete_layers;
	static int scale;
};

// Formerly in TGUI
extern ALLEGRO_EVENT_SOURCE evtsrc;
const int TGUI_EVENT_OBJECT = 0;
void pushEvent(int type, void *data);

#endif // GENERAL_H
