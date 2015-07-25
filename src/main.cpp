#define ALLEGRO_STATICLINK

#include "general.h"
#include "widgets.h"

#include <sstream>
#include <cstdio>

#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_memfile.h>

#include "icon.h"

enum {
   FILE_ID = 1,
   FILE_OPEN_ID,
   FILE_SAVE_ID,
   FILE_SAVE_AS_ID,
   FILE_RELOAD_TILES_ID,
   FILE_LOAD_TILES_ID,
   FILE_EXIT_ID,
   EDIT_ID,
   EDIT_UNDO_ID,
   EDIT_REDO_ID,
   SCALE_ID,
   SCALE_1_ID,
   SCALE_2_ID,
   SCALE_3_ID,
   SCALE_4_ID,
   SCALE_5_ID,
   SCALE_6_ID,
   SCALE_7_ID,
   SCALE_8_ID,
   SCALE_9_ID,
   GROUP_TYPE_ID,
   GROUP_OBJECT_ID,
   GROUP_SHADOW_ID,
   HELP_ID,
   HELP_QUICK_REFERENCE_ID
};

ALLEGRO_MENU_INFO main_menu_info[] = {
   ALLEGRO_START_OF_MENU("&File", FILE_ID),
      { "&Open", FILE_OPEN_ID, 0, NULL },
      { "&Save", FILE_SAVE_ID, 0, NULL },
      { "Save As...", FILE_SAVE_AS_ID, 0, NULL },
      { "Reload Tiles", FILE_RELOAD_TILES_ID, 0, NULL },
      { "Load Tiles...", FILE_LOAD_TILES_ID, 0, NULL },
      ALLEGRO_MENU_SEPARATOR,
      { "E&xit", FILE_EXIT_ID, 0, NULL },
      ALLEGRO_END_OF_MENU,
   
   ALLEGRO_START_OF_MENU("&Edit", EDIT_ID),
      { "Undo", EDIT_UNDO_ID, 0, NULL },
      { "Redo", EDIT_REDO_ID, 0, NULL },
      ALLEGRO_END_OF_MENU,

   ALLEGRO_START_OF_MENU("S&cale", SCALE_ID),
      { "&1x", SCALE_1_ID, 0, NULL },
      { "&2x", SCALE_2_ID, 0, NULL },
      { "&3x", SCALE_3_ID, 0, NULL },
      { "&4x", SCALE_4_ID, 0, NULL },
      { "&5x", SCALE_5_ID, 0, NULL },
      { "&6x", SCALE_6_ID, 0, NULL },
      { "&7x", SCALE_7_ID, 0, NULL },
      { "&8x", SCALE_8_ID, 0, NULL },
      { "&9x", SCALE_9_ID, 0, NULL },
      ALLEGRO_END_OF_MENU,

#ifdef MO3
   ALLEGRO_START_OF_MENU("Group Type", GROUP_TYPE_ID),
   		{ "Object", GROUP_OBJECT_ID, ALLEGRO_MENU_ITEM_CHECKBOX, NULL },
   		{ "Shadow", GROUP_SHADOW_ID, ALLEGRO_MENU_ITEM_CHECKBOX, NULL },
   		ALLEGRO_END_OF_MENU,
#endif

   ALLEGRO_START_OF_MENU("&Help", HELP_ID),
      { "&Quick Reference", HELP_QUICK_REFERENCE_ID, 0, NULL },
      ALLEGRO_END_OF_MENU,

   ALLEGRO_END_OF_MENU
};

ALLEGRO_DISPLAY *display;
ALLEGRO_EVENT_QUEUE *queue;
static bool resize;
std::vector<ALLEGRO_BITMAP *> tileSheets;
static A_Combobox *layerCombo, *sheetCombo;
static A_Tileselector *tileSelector;
static A_Leveleditor *levelEditor;
static bool draw_yellow_and_purple = true;
static A_Scrollpane *levelScrollpane;
static int levelX = -1, levelY = -1;
static A_Scrollpane *tileScrollpane;
static std::vector<bool> draw_solids;

static int mouse_x = 0, mouse_y = 0;

static std::string tile_load_path = "";

/* Quick reference window stuff
 */
A_Frame *quickRefFrame;
A_Splitter *quickRefSplitter;
A_Splitter *quickRefBottomSplitter;
A_Label *quickRefContent1, *quickRefContent2;
A_Titlebar *quickRefTitlebar;

void setTitle()
{
	std::string filename = levelEditor->getOperatingFilename();
	if (filename == "") {
		al_set_window_title(display, "AshEdit");
	}
	else {
		std::string name = std::string("AshEdit") + " - " + filename;
		al_set_window_title(display, name.c_str());
	}
}

static void draw(void)
{
	if (resize) {
		tgui::resize(NULL);
		resize = false;
	}
	al_clear_to_color(al_color_name("white"));
	tgui::draw();
	al_flip_display();
}

static void tileDrawCallback(int ox, int oy, int dx, int dy, int w, int h, int layer)
{
	(void)layer;

	ALLEGRO_BITMAP *sheet = 0;
	
	if (!tileSheets.size() || sheetCombo->getSelected() > (int)tileSheets.size())
		return;
	
	sheet = tileSheets[sheetCombo->getSelected()];
	al_draw_scaled_bitmap(
		sheet,
		ox,
		oy,
		w,
		h,
		dx,
		dy,
		w,
		h,
		0
	);

	int sel_x, sel_y, sel_w, sel_h;
	tileSelector->getSelected(&sel_x, &sel_y, &sel_w, &sel_h);

	int rx = dx + ((sel_x*General::tileSize*General::scale) - ox);
	int ry = dy + ((sel_y*General::tileSize*General::scale) - oy);

	al_draw_rectangle(
		rx+0.5,
		ry+0.5,
		rx+General::tileSize*sel_w*General::scale-0.5,
		ry+General::tileSize*sel_h*General::scale-0.5,
		al_color_name("black"),
		1
	);
	al_draw_rectangle(
		rx+1.5,
		ry+1.5,
		rx+General::tileSize*sel_w*General::scale-1.5,
		ry+General::tileSize*sel_h*General::scale-1.5,
		al_color_name("yellow"),
		1
	);
	al_draw_rectangle(
		rx+2.5,
		ry+2.5,
		rx+General::tileSize*sel_w*General::scale-2.5,
		ry+General::tileSize*sel_h*General::scale-2.5,
		al_color_name("black"),
		1
	);
}

static void levelDrawCallback(int ox, int oy, int dx, int dy, int w, int h, int layer)
{
	int xx = dx - (ox % (General::tileSize*General::scale));
	int yy = dy - (oy % (General::tileSize*General::scale));
	int tx = ox / (General::tileSize*General::scale);
	int ty = oy / (General::tileSize*General::scale);
	int wt = w / (General::tileSize*General::scale)+2;
	int ht = h / (General::tileSize*General::scale)+2;
	int layers = levelEditor->getLayers();
	int savedx = dx;
	int savedy = dy;

	std::vector<bool> solids(layers);

	if (layer == -1) {
		if (draw_yellow_and_purple) {
			al_draw_filled_rectangle(
				dx,
				dy,
				dx+w*General::scale,
				dy+h*General::scale,
				al_color_name("magenta")
			);
		}
		int maxx = MIN(w, levelEditor->getWidth()*(General::tileSize*General::scale)-ox);
		int maxy = MIN(h, levelEditor->getHeight()*(General::tileSize*General::scale)-oy);
		if (draw_yellow_and_purple) {
			al_draw_filled_rectangle(
				dx,
				dy,
				dx+maxx,
				dy+maxy,
				al_color_name("lime")
			);
		}
	}

	dy = yy;
	for (int y = 0; y < ht; y++) {
		if (ty+y >= levelEditor->getHeight() || ty+y < 0)
			continue;
		dx = xx;
		for (int x = 0; x < wt; x++) {
			if (tx+x >= levelEditor->getWidth() || tx+x < 0)
				continue;
			int lstart, lend;
			if (layer == -1) {
				lstart = 0;
				lend = layers;
			}
			else {
				lstart = layer;
				lend = layer+1;
			}
			for (int l = lstart; l < lend; l++) {
				int tile, sheet;
				bool tint;
				bool solids_tmp = solids[l];
				levelEditor->getTile(tx+x, ty+y, l, &tile, &sheet, &solids_tmp, &tint);
				solids[l] = solids_tmp;
				if (tile < 0 || sheet < 0) {
					continue;
				}
				if (sheet >= (int)tileSheets.size()) {
					printf("sheet too high: %d\n", sheet);
					continue;
				}
				if (levelEditor->isVisible(l) == false)
					continue;
				ALLEGRO_BITMAP *bmp = tileSheets[sheet];
				int widthInTiles = al_get_bitmap_width(bmp) / (General::tileSize*General::scale);
				int tileX = tile % widthInTiles;
				int tileY = tile / widthInTiles;
				al_draw_tinted_scaled_bitmap(
					bmp,
					(tint ?
						al_color_name("magenta") :
						al_color_name("white")),
					tileX*General::tileSize*General::scale,
					tileY*General::tileSize*General::scale,
					General::tileSize*General::scale,
					General::tileSize*General::scale,
					dx,
					dy,
					General::tileSize*General::scale,
					General::tileSize*General::scale,
					0
				);
			}
			if (draw_solids[levelEditor->getCurrentLayer()] && solids[layerCombo->getSelected()]) {
				al_draw_line(
					dx,
					dy,
					dx+General::tileSize*General::scale,
					dy+General::tileSize*General::scale,
					al_color_name("yellow"), 2);
				al_draw_line(
					dx,
					dy+General::tileSize*General::scale,
					dx+General::tileSize*General::scale,
					dy,
					al_color_name("yellow"),
					2
				);
			}
			dx += (General::tileSize*General::scale);
		}
		dy += (General::tileSize*General::scale);
	}
	
	if (mouse_x > -1 && mouse_y > -1) {
		int xx = mouse_x - (mouse_x % (General::tileSize*General::scale));
		int yy = mouse_y - (mouse_y % (General::tileSize*General::scale));
		xx -= ox;
		yy -= oy;
		int abs_x, abs_y;
		determineAbsolutePosition(levelEditor, &abs_x, &abs_y);
		xx += abs_x;
		yy += abs_y;
		al_draw_rectangle(
			xx,
			yy,
			xx + (General::tileSize*General::scale),
			yy + (General::tileSize*General::scale),
			al_map_rgba(0, 0, 0, 128), 2.0);
	}

	std::vector<A_Leveleditor::Group> &groups = levelEditor->getGroups();

	for (size_t i = 0; i < groups.size(); i++) {
		A_Leveleditor::Group &g = groups[i];
		if (draw_solids[g.layer]) {
			ALLEGRO_COLOR colour;
			if (g.type == 0) {
				colour = al_map_rgb(255, 0, 0);
			}
			else if (g.type == 1) {
				colour = al_map_rgb(0, 255, 0);
			}
			else if (g.type == 2) {
				colour = al_map_rgb(0, 0, 255);
			}
			else if (g.type == 3) {
				colour = al_map_rgb(0, 255, 255);
			}
			al_draw_rectangle(savedx + (g.x * General::tileSize * General::scale) - ox, savedy + (g.y * General::tileSize * General::scale) - oy, savedx + ((g.x + g.w) * General::tileSize * General::scale) - ox, savedy + ((g.y + g.h) * General::tileSize * General::scale) - oy, colour, 1.0f);
		}
	}

	if (levelEditor->getTool() == "Marquee") {
		ALLEGRO_COLOR c;
		float f = fmod(al_get_time(), 2);
		if (f > 1) {
			f = 2 - f;
		}
		c = al_map_rgb_f(f, f, f);

		int x1, y1, x2, y2;

		if (levelEditor->is_marquee_floating()) {
			std::vector< std::vector< std::vector<A_Leveleditor::_Tile> > > paste = levelEditor->get_marquee_buffer();
			levelEditor->get_marquee_float_xy(&x1, &y1);
			x2 = x1 + paste[0].size();
			y2 = y1 + paste.size();
			x1 = savedx + (x1 * General::tileSize * General::scale) - ox;
			y1 = savedy + (y1 * General::tileSize * General::scale) - oy;
			x2 = savedx + (x2 * General::tileSize * General::scale) - ox;
			y2 = savedy + (y2 * General::tileSize * General::scale) - oy;
			al_draw_rectangle(
				x1, y1, x2, y2, c, 1
			);
		}
		else {
			if (levelEditor->is_marquee_marked()) {
				levelEditor->get_marquee(&x1, &y1, &x2, &y2);
				x1 = savedx + (x1 * General::tileSize * General::scale) - ox;
				y1 = savedy + (y1 * General::tileSize * General::scale) - oy;
				x2 = savedx + (x2 * General::tileSize * General::scale) - ox;
				y2 = savedy + (y2 * General::tileSize * General::scale) - oy;
				al_draw_rectangle(
					x1, y1, x2, y2, c, 1
				);
			}
		}
	}
}

std::string selectDir(const std::string &start)
{
	std::string tmp = start;
	tmp += std::string("/");
	ALLEGRO_FILECHOOSER *diag = al_create_native_file_dialog(tmp.c_str(), "Select Tile Set", 0, ALLEGRO_FILECHOOSER_FOLDER | ALLEGRO_FILECHOOSER_FILE_MUST_EXIST);
	al_show_native_file_dialog(display, diag);

	tgui::clearKeyState();

	if (al_get_native_file_dialog_count(diag) != 1)
		return "";
	
	std::string path = al_get_native_file_dialog_path(diag, 0);
	al_destroy_native_file_dialog(diag);
	return path;
}

bool tilesSort(ALLEGRO_PATH *a, ALLEGRO_PATH *b)
{
	const char *a_fn = al_get_path_filename(a);
	const char *b_fn = al_get_path_filename(b);
	
	int a_int = atoi(a_fn+5);
	int b_int = atoi(b_fn+5);
	
	return a_int < b_int;
}

void loadTileSheets(const char *path)
{
	ALLEGRO_FS_ENTRY *path_entry = al_create_fs_entry(path);
	if (!al_open_directory(path_entry)) {
		fprintf(stderr, "failed to list directory '%s': %s\n", path, strerror(errno));
		return;
	}
	
	std::vector<ALLEGRO_PATH *> items;
	ALLEGRO_FS_ENTRY *ent = 0;
	while((ent = al_read_directory(path_entry))) {
		const char *ent_path = al_get_fs_entry_name(ent);
		
		ALLEGRO_PATH *path = al_create_path(ent_path);
		if (!path) {
			fprintf(stderr, "failed to create path object for loading tile sheet '%s': %s\n", ent_path, strerror(errno));
			continue;
		}
		
		const char *ent_name = al_get_path_filename(path);
		if (ent_name[0] == '.') continue; // skip hidden files, the parent ref, and self ref entries
		
		const char *ent_ext = al_get_path_extension(path);
#ifdef USE_TGA
		if (strcmp(ent_ext, ".tga") == 0 && strncmp(ent_name, "tiles", 5) == 0) {
#else
		if (strcmp(ent_ext, ".png") == 0 && strncmp(ent_name, "tiles", 5) == 0) {
#endif
			items.push_back(path);
		}
	}
	
	std::sort(items.begin(), items.end(), tilesSort);
	
	tileSheets.clear();
	
	sheetCombo->clearValues();
	for(unsigned int i = 0; i < items.size(); i++) {
		ALLEGRO_BITMAP *tmp = al_load_bitmap(al_path_cstr(items[i], ALLEGRO_NATIVE_PATH_SEP));
		if (!tmp)
			break;
		al_convert_mask_to_alpha(tmp, al_map_rgb(255, 0, 255));
		ALLEGRO_BITMAP *bmp = al_create_bitmap(
			al_get_bitmap_width(tmp)*General::scale,
			al_get_bitmap_height(tmp)*General::scale
		);
		ALLEGRO_STATE st;
		al_store_state(&st, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_BLENDER);
		al_set_target_bitmap(bmp);
		al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
		al_draw_scaled_bitmap(
			tmp,
			0, 0,
			al_get_bitmap_width(tmp),
			al_get_bitmap_height(tmp),
			0, 0,
			al_get_bitmap_width(bmp),
			al_get_bitmap_height(bmp),
			0
		);
		al_restore_state(&st);
		al_destroy_bitmap(tmp);
		tileSheets.push_back(bmp);
		al_destroy_path(items[i]);
		
		char buf[20];
		sprintf(buf, "Sheet %d", i);
		sheetCombo->addValue(std::string(buf));
	}

	if (levelEditor) {
		levelEditor->tilesLoaded();
	}
}

static void reloadTiles()
{
	loadTileSheets(tile_load_path.c_str());

	if (tileSheets.size()) {
		tileScrollpane->setScrollSize(
			al_get_bitmap_width(tileSheets[0]),
			al_get_bitmap_height(tileSheets[0])
		);
	}
}

int main(int argc, char **argv)
{
	// Initialize Allegro
	al_init();
	al_install_mouse();
	al_install_keyboard();
	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();
	al_init_native_dialog_addon();

    al_init_user_event_source(&evtsrc);
 
	al_set_new_display_flags(ALLEGRO_RESIZABLE);
	display = al_create_display(1200, 640);

	ALLEGRO_MENU *menu = al_build_menu(main_menu_info);
	al_set_display_menu(display, menu);

#ifdef ALLEGRO_WINDOWS
	ALLEGRO_FONT *font = al_load_ttf_font("C:\\Windows\\Fonts\\arial.ttf", 12, 0);
#else
	ALLEGRO_FONT *font = al_load_ttf_font("font.ttf", 12, 0);
#endif
	queue = al_create_event_queue();

	al_register_event_source(queue, &evtsrc);

	ALLEGRO_TIMER *draw_timer = al_create_timer(1.0/20.0);
	al_start_timer(draw_timer);

	ALLEGRO_FILE *f = al_open_memfile(icon_png, sizeof(icon_png), "rb");
	ALLEGRO_BITMAP *icon_bmp = al_load_bitmap_f(f, ".png");
	al_set_display_icon(display, icon_bmp);
	al_destroy_bitmap(icon_bmp);
	al_fclose(f);

	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_timer_event_source(draw_timer));
	al_register_event_source(queue, al_get_default_menu_event_source());

	tgui::init(display);
	tgui::setFont(font);
	
	A_Splitter *mainSplitBottom= new A_Splitter(A_Splitter::SPLIT_HORIZONTAL);
	A_Splitter *canvasSplit = new A_Splitter(A_Splitter::SPLIT_VERTICAL);
	A_Splitter *canvasSplitLeft = new A_Splitter(A_Splitter::SPLIT_HORIZONTAL);
	A_Splitter *canvasSplitRight = new A_Splitter(A_Splitter::SPLIT_HORIZONTAL);
	canvasSplit->setResizable(true);

	layerCombo = new A_Combobox(
		7,
		al_color_name("cyan"),
		al_color_name("black"),
		al_color_name("darkcyan")
	);
	sheetCombo = new A_Combobox(
		7,
		al_color_name("cyan"),
		al_color_name("black"),
		al_color_name("darkcyan")
	);
	for (int i = 0; i < General::startLayers; i++) {
		char buf[20];
		sprintf(buf, "Layer %d", i);
		layerCombo->addValue(std::string(buf));
	}

	if (argc > 1) {
		tile_load_path = argv[1];
		loadTileSheets(argv[1]);
	}
	else {
		tile_load_path = ".";
		loadTileSheets(".");
	}
	
	levelScrollpane = new A_Scrollpane(
		al_color_name("gray"),
		al_color_name("lightgrey")
	);
	tileScrollpane = new A_Scrollpane(
		al_color_name("gray"),
		al_color_name("lightgrey")
	);
	tileSelector = new A_Tileselector(tileDrawCallback);
	tileScrollpane->addScrollable(tileSelector);
	if (tileSheets.size()) {
		tileScrollpane->setScrollSize(
			al_get_bitmap_width(tileSheets[0]),
			al_get_bitmap_height(tileSheets[0])
		);
	}
	levelEditor = new A_Leveleditor(levelDrawCallback, General::startLayers, tileSelector);
	levelScrollpane->addScrollable(levelEditor);
	levelScrollpane->setScrollSize(
		levelEditor->getWidth()*(General::tileSize*General::scale),
		levelEditor->getHeight()*(General::tileSize*General::scale)
	);

	A_Label *statusLabel = new A_Label("Status", al_color_name("black"));

	setTitle();
	for (int i = 0; i < levelEditor->getNumLayers(); i++) {
		draw_solids.push_back(true);
	}

	tgui::setNewWidgetParent(0);
	tgui::addWidget(mainSplitBottom);
	mainSplitBottom->addToSecondPane(statusLabel);
	mainSplitBottom->addToFirstPane(canvasSplit);
	canvasSplit->addToFirstPane(canvasSplitLeft);
	canvasSplit->addToSecondPane(canvasSplitRight);
	canvasSplitLeft->addToFirstPane(layerCombo);
	canvasSplitLeft->addToSecondPane(levelScrollpane);
	canvasSplitRight->addToFirstPane(sheetCombo);
	canvasSplitRight->addToSecondPane(tileScrollpane);

	tgui::setFocus(mainSplitBottom);

	mainSplitBottom->setSplitSize(-1, 20);
	canvasSplitLeft->setSplitSize(20, -1);
	canvasSplitRight->setSplitSize(20, -1);

	tgui::resize(NULL); // similar to java pack(), on all widgets

	if (argc > 2) {
      std::string level_data = argv[2];
		levelEditor->load(level_data);
		levelEditor->setLastSaveName(level_data);
	}

	draw();
	int draw_ticks = 0;
	
	while (true) {
		ALLEGRO_EVENT event;
		do {
			al_wait_for_event(queue, &event);
			if (event.type == ALLEGRO_EVENT_TIMER) {
				draw_ticks++;
			}
			else if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
				al_acknowledge_resize(event.display.source);
				resize = true;
				draw();
			}
			else if ((event.type == ALLEGRO_EVENT_KEY_DOWN &&
				event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) ||
				event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
				
				int ret;
				if (levelEditor->getChanged()) {
					ret = al_show_native_message_box(display, "Confirm Exit", "The level has changed!", "Really exit?", 0, ALLEGRO_MESSAGEBOX_YES_NO);
					tgui::clearKeyState();
				}
				else {
					ret = 1;
				}
				if (ret == 1) {
					goto done;
				}
			}
			else if ((event.type == ALLEGRO_EVENT_KEY_DOWN &&
				event.keyboard.keycode == ALLEGRO_KEY_ENTER)) {
				// Save whole area as png
				int w = levelEditor->getWidth()*General::tileSize*General::scale;
				int h = levelEditor->getHeight()*General::tileSize*General::scale;
				//al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
				ALLEGRO_BITMAP *png = al_create_bitmap(w, h);
				ALLEGRO_BITMAP *old_target = al_get_target_bitmap();
			
				//int j;
				//for(j = 0; j < tileSheets.size(); j++) {
				//	al_lock_bitmap(tileSheets[j], ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
				//}
			
				al_set_target_bitmap(png);
				al_clear_to_color(al_map_rgba(0, 0, 0, 0));
				draw_yellow_and_purple = false;
				levelDrawCallback(0, 0, 0, 0, w, h, -1);
				draw_yellow_and_purple = true;
				al_set_target_bitmap(old_target);
				int i;
				for (i = 0; i < 9999; i++) {
					char buf[100];
					sprintf(buf, "ashedit-save-%04d.png", i);
					if (al_filename_exists(buf)) continue;
					al_save_bitmap(buf, png);
					break;
				}
				al_destroy_bitmap(png);
				
				//for(j = 0; j < tileSheets.size(); j++) {
				//	al_unlock_bitmap(tileSheets[j]);
				//}
			}
			else if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_T && tgui::isKeyDown(ALLEGRO_KEY_ALT)) {
				int layer = levelEditor->getCurrentLayer();
				draw_solids[layer] = !draw_solids[layer];
			}
			else if ((event.type == ALLEGRO_EVENT_KEY_CHAR && event.keyboard.keycode == ALLEGRO_KEY_S &&
						(event.keyboard.modifiers & ALLEGRO_KEYMOD_CTRL)))
			{
				levelEditor->save(false);
				setTitle();
				continue;
			}
			else if ((event.type == ALLEGRO_EVENT_KEY_UP && event.keyboard.keycode >= ALLEGRO_KEY_F1 && event.keyboard.keycode <= ALLEGRO_KEY_F12)) {
				int layer = (event.keyboard.keycode - ALLEGRO_KEY_F1);
				if (layer < 0 || layer >= levelEditor->getNumLayers())
					continue;

				if (levelEditor->getLayer() != layer) {
					levelEditor->setLayer(layer);
					layerCombo->setSelected(layer);
				} else {
					levelEditor->toggleLayerVisibility(layer);
				}
			}
			else if (event.type == ALLEGRO_EVENT_MENU_CLICK) {
				if (event.user.data1 == FILE_OPEN_ID) {
					int ret;
					if (levelEditor->getChanged()) {
						ret = al_show_native_message_box(display, "Warning", "The level has changed!", "Really load a new level?", 0, ALLEGRO_MESSAGEBOX_YES_NO);
						tgui::clearKeyState();
					}
					else {
						ret = 1;
					}
					if (ret == 1) {
						levelEditor->load();
						setTitle();
						draw_solids.clear();
						for (int i = 0; i < levelEditor->getNumLayers(); i++) {
							draw_solids.push_back(true);
						}
					}
				}
				else if (event.user.data1 == FILE_SAVE_ID) {
					levelEditor->save(false);
					setTitle();
				}
				else if (event.user.data1 == FILE_SAVE_AS_ID) {
					levelEditor->save(true);
					setTitle();
				}
				else if (event.user.data1 == FILE_RELOAD_TILES_ID) {
					loadTileSheets(tile_load_path.c_str());

					if (tileSheets.size()) {
						tileScrollpane->setScrollSize(
							al_get_bitmap_width(tileSheets[0]),
							al_get_bitmap_height(tileSheets[0])
						);
					}
				}
				else if (event.user.data1 == FILE_LOAD_TILES_ID) {
					const char *start = argc > 1 ? argv[1] : al_get_current_directory();
					tile_load_path = selectDir(start);
					loadTileSheets(tile_load_path.c_str());

					if (tileSheets.size()) {
						tileScrollpane->setScrollSize(
							al_get_bitmap_width(tileSheets[0]),
							al_get_bitmap_height(tileSheets[0])
						);
					}
				}
				else if (event.user.data1 == FILE_EXIT_ID) {
					int ret;
					if (levelEditor->getChanged()) {
						ret = al_show_native_message_box(display, "Confirm Exit", "The level has changed!", "Really exit?", 0, ALLEGRO_MESSAGEBOX_YES_NO);
						tgui::clearKeyState();
					}
					else {
						ret = 1;
					}
					if (ret == 1) {
						goto done;
					}
				}
				else if (event.user.data1 == EDIT_UNDO_ID) {
					levelEditor->doUndo();
				}
				else if (event.user.data1 == EDIT_REDO_ID) {
					levelEditor->doRedo();
				}
				else if (event.user.data1 == SCALE_1_ID) {
					General::scale = 1;
					levelEditor->resizeScrollpane();
					reloadTiles();
				}
				else if (event.user.data1 == SCALE_2_ID) {
					General::scale = 2;
					levelEditor->resizeScrollpane();
					reloadTiles();
				}
				else if (event.user.data1 == SCALE_3_ID) {
					General::scale = 3;
					levelEditor->resizeScrollpane();
					reloadTiles();
				}
				else if (event.user.data1 == SCALE_4_ID) {
					General::scale = 4;
					levelEditor->resizeScrollpane();
					reloadTiles();
				}
				else if (event.user.data1 == SCALE_5_ID) {
					General::scale = 5;
					levelEditor->resizeScrollpane();
					reloadTiles();
				}
				else if (event.user.data1 == SCALE_6_ID) {
					General::scale = 6;
					levelEditor->resizeScrollpane();
					reloadTiles();
				}
				else if (event.user.data1 == SCALE_7_ID) {
					General::scale = 7;
					levelEditor->resizeScrollpane();
					reloadTiles();
				}
				else if (event.user.data1 == SCALE_8_ID) {
					General::scale = 8;
					levelEditor->resizeScrollpane();
					reloadTiles();
				}
				else if (event.user.data1 == SCALE_9_ID) {
					General::scale = 9;
					levelEditor->resizeScrollpane();
					reloadTiles();
				}
#ifdef MO3
				else if (event.user.data1 == GROUP_OBJECT_ID || event.user.data1 == GROUP_SHADOW_ID) {
					bool object_checked = al_get_menu_item_flags(menu, GROUP_OBJECT_ID) & ALLEGRO_MENU_ITEM_CHECKED;
					bool shadow_checked = al_get_menu_item_flags(menu, GROUP_SHADOW_ID) & ALLEGRO_MENU_ITEM_CHECKED;
					int group_type = 0;
					if (object_checked) group_type |= 1;
					if (shadow_checked) group_type |= 2;
					levelEditor->set_group_type(group_type);
				}
#endif
				else if (event.user.data1 == HELP_QUICK_REFERENCE_ID) {
					quickRefFrame = new A_Frame(al_color_name("blue"), 450, 450);
					quickRefFrame->setPosition(50, 50);
					quickRefSplitter = new A_Splitter(A_Splitter::SPLIT_HORIZONTAL);
					quickRefBottomSplitter = new A_Splitter(A_Splitter::SPLIT_VERTICAL);
					quickRefContent1 = new A_Label(
						" Ctrl-R\n"
						" Shift-Ctrl-R\n"
						" Ctrl-C\n"
						" Shift-Ctrl-C\n"
						" Ctrl-Delete\n"
						" Shift-Ctrl-Delete\n"
						" Ctrl-L\n"
						" Shift-Ctrl-L\n"
						" Ctrl-Alt-L\n"
						" B\n"
						" C\n"
						" S\n"
						" M\n"
						" K\n"
						" V\n"
						" F\n"
						" Shift-F\n"
						" R\n"
						" T\n"
						" Alt-T\n"
						" Enter\n"
						" Q\n"
						" Comma\n"
						" Period\n"
						" Slash\n"
						" Space\n"
						" G\n"
						" Alt-G\n",
						al_color_name("white")
					);
					quickRefContent2 = new A_Label(
						"Insert row before cursor\n"
						"Insert row after cursor\n"
						"Insert column before cursor\n"
						"Insert column after cursor\n"
						"Delete row\n"
						"Delete column\n"
						"Insert layer before current layer\n"
						"Insert layer after current layer\n"
						"Delete current layer\n"
						"Switch to the brush tool\n"
						"Switch to the clear tool\n"
						"Switch to the solids tool\n"
						"Switch to the macro tool\n"
						"Switch to the clone tool\n"
						"Switch to the layer mover tool\n"
						"Switch to fill tool (test current layer)\n"
						"Switch to fill tool (test all layers)\n"
						"Start/stop recording macro\n"
						"Toggle current layer drawing\n"
						"Toggle current layer solids/groups drawing\n"
						"Save as PNG\n"
						"Marquee tool\n"
						"Copy (all layers with Ctrl)\n"
						"Cut (all layers with Ctrl)\n"
						"Paste\n"
						"Anchor floating selection\n"
						"Add a group (uses layer and marquee)\n"
						"Delete a group (uses layer and marquee)\n",
						al_color_name("white")
					);
					quickRefContent1->setX(5);
					quickRefContent1->setY(5);
					quickRefContent2->setY(5);
					quickRefTitlebar = new A_Titlebar(quickRefFrame, "Quick Reference", al_color_name("cyan"), A_Titlebar::CLOSE_BUTTON);
					quickRefSplitter->addToFirstPane(quickRefTitlebar);
					quickRefSplitter->addToSecondPane(quickRefBottomSplitter);
					quickRefBottomSplitter->addToFirstPane(quickRefContent1);
					quickRefBottomSplitter->addToSecondPane(quickRefContent2);
					tgui::setNewWidgetParent(NULL);
					tgui::addWidget(quickRefFrame);
					tgui::setNewWidgetParent(quickRefFrame);
					tgui::addWidget(quickRefSplitter);
					quickRefSplitter->setSplitSize(20, -1);
					quickRefBottomSplitter->setSplitRatio(0.3, 0.7);
					tgui::resize(NULL);
				}
			}
			else if (event.type == ALLEGRO_GET_EVENT_TYPE('T', 'G', 'U', 'I')) {
				ALLEGRO_USER_EVENT *u = (ALLEGRO_USER_EVENT *)&event;
				int type = (int)u->data1;
				if (type == TGUI_EVENT_OBJECT) {
					tgui::TGUIWidget *widget = (tgui::TGUIWidget *)u->data2;
					if (widget && widget == quickRefTitlebar) {
						quickRefFrame->remove();
						delete quickRefTitlebar;
						delete quickRefContent1;
						delete quickRefContent2;
						delete quickRefSplitter;
						delete quickRefBottomSplitter;
						delete quickRefFrame;
						quickRefTitlebar = NULL;
						quickRefContent1 = NULL;
						quickRefContent2 = NULL;
						quickRefSplitter = NULL;
						quickRefBottomSplitter = NULL;
						quickRefFrame = NULL;
					}
				}
			}
			else {
				if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
					mouse_x = mouse_y = -1;
					levelX = levelY = -1;
					if (pointOnWidget(levelEditor, event.mouse.x, event.mouse.y)) {
						int mx = event.mouse.x;
						int my = event.mouse.y;
						mx += levelScrollpane->getOffsetX();
						my += levelScrollpane->getOffsetY();
						int abs_x, abs_y;
						determineAbsolutePosition(levelScrollpane, &abs_x, &abs_y);
						mouse_x = mx - abs_x;
						mouse_y = my - abs_y;
						int xx = mouse_x - (mouse_x % General::tileSize);
						int yy = mouse_y - (mouse_y % General::tileSize);
						levelX = (xx+General::tileSize/2) / General::tileSize;
						levelY = (yy+General::tileSize/2) / General::tileSize;
					}
				}
            
				if (quickRefFrame)
					quickRefFrame->raise();
				levelEditor->setLayer(layerCombo->getSelected());
				if (levelEditor->getTool() == "Mover") {
					levelEditor->setMoverDestLayer(layerCombo->getSelected());
				}
				int tileX, tileY;
				tileSelector->getSelected(&tileX, &tileY, NULL, NULL);
				
				int tileNumber = 0;
				if (tileSheets.size()) {
					levelEditor->setTile(tileY*(al_get_bitmap_width(tileSheets[0])/(General::tileSize*General::scale))+tileX);
					levelEditor->setSheet(sheetCombo->getSelected());
					tileNumber = tileY*(al_get_bitmap_width(tileSheets[0])/(General::tileSize*General::scale))+tileX;
				}
				
				tgui::handleEvent(&event);
				// build status string
				std::stringstream ss;
				if (levelEditor->getRecording())
					ss << "(Recording) ";
				ss << levelEditor->getTool() << " -- Level (" << (levelX/General::scale) << "," << (levelY/General::scale) << ") Tile (" << tileNumber << "," << tileX << "," << tileY << ") Pixel (" << (mouse_x/General::scale) << "," << (mouse_y/General::scale) << ")";
				statusLabel->setText(ss.str());
				if (levelEditor->getNumLayers() != layerCombo->getSize()) {
					layerCombo->clearValues();
					layerCombo->setSelected(levelEditor->getLayer());
					for (int i = 0; i < levelEditor->getNumLayers(); i++) {
						char buf[20];
						sprintf(buf, "Layer %d", i);
						layerCombo->addValue(std::string(buf));
					}
				}
			}
		} while (!al_event_queue_is_empty(queue));

		if (draw_ticks > 0)
			draw();

		draw_ticks = 0;

		al_rest(0.001);
	}
done:
	return 0;
}

