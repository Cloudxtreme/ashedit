#include <cstdio>
#include <cstdlib>
#include <map>

int readInt(FILE *f) {
	unsigned char b1 = fgetc(f);
	unsigned char b2 = fgetc(f);
	unsigned char b3 = fgetc(f);
	unsigned char b4 = fgetc(f);
	return b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
}

void writeInt(FILE *f, int i) {
	fputc((i >> 0) & 0xff, f);
	fputc((i >> 8) & 0xff, f);
	fputc((i >> 16) & 0xff, f);
	fputc((i >> 24) & 0xff, f);
}

void change(char *in, int from, int to, char *out)
{
	FILE *inf = fopen(in, "rb");
	FILE *outf = fopen(out, "wb");

	int w = readInt(inf);
	int h = readInt(inf);
	int layers = readInt(inf);

	writeInt(outf, w);
	writeInt(outf, h);
	writeInt(outf, layers);

	// for each layer
	for (int l = 0; l < layers; l++) {
		// read each tile: tile number and sheet
		for (unsigned int y = 0; y < h; y++) {
			for (unsigned int x = 0; x < w; x++) {
				int number = readInt(inf);
				char sheet = fgetc(inf);
				char solid = fgetc(inf);
				if (sheet == from)
					sheet = to;
				writeInt(outf, number);
				fputc(sheet, outf);
				fputc(solid, outf);
			}
		}
	}

	fclose(inf);
	fclose(outf);
}

void show(char *in)
{
	FILE *inf = fopen(in, "rb");

	int w = readInt(inf);
	int h = readInt(inf);
	int layers = readInt(inf);

	std::map<int, bool> map;

	// for each layer
	for (int l = 0; l < layers; l++) {
		// read each tile: tile number and sheet
		for (unsigned int y = 0; y < h; y++) {
			for (unsigned int x = 0; x < w; x++) {
				int number = readInt(inf);
				char sheet = fgetc(inf);
				char solid = fgetc(inf);
				if (sheet == -1)
					continue;
				map[sheet] = true;
			}
		}
	}

	std::map<int, bool>::iterator it;
	for (it = map.begin(); it != map.end(); it++) {
		int sheet = it->first;
		printf("Used: %d\n", sheet);
	}

	fclose(inf);
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		printf("Usage: change_layer <infile> <from_layer> <to_layer> <outfile>\n");
		printf("Usage: change_layer -show-used-sheets <infile>\n");
		return 0;
	}
		
	bool show_only = !strcmp(argv[1], "-show-used-sheets");

	if (show_only) {
		show(argv[2]);
	}
	else {
		change(argv[1], atoi(argv[2]), atoi(argv[3]), argv[4]);
	}
}

