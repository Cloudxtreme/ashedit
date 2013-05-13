#ifndef GENERAL_H
#define GENERAL_H

#define MIN(a, b) ((a) < (b) ? (a) : (b))

class General {
public:
	static int tileSize;
	static int areaSize;
	static int startLayers;
	static bool can_add_and_delete_layers;
	static int scale;
};

#endif // GENERAL_H
