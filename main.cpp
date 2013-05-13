#include "general.h"
#include "widgets.h"

#include <sstream>
#include <cstdio>

#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_memfile.h>

#include "ryori.h"

ALLEGRO_DISPLAY *display;
ALLEGRO_BITMAP *ryori_bmp;
ALLEGRO_EVENT_QUEUE *queue;
static bool resize;
std::vector<ALLEGRO_BITMAP *> tileSheets;
static A_Combobox *layerCombo, *sheetCombo;
static A_Tileselector *tileSelector;
static A_Leveleditor *levelEditor;
static bool draw_yellow_and_purple = true;
static A_Scrollpane *levelScrollpane;
static int levelX = -1, levelY = -1;

static int mouse_x = 0, mouse_y = 0;

/* Quick reference window stuff
 */
A_Frame *quickRefFrame;
A_Splitter *quickRefSplitter;
A_Splitter *quickRefBottomSplitter;
A_Label *quickRefContent1, *quickRefContent2;
A_Titlebar *quickRefTitlebar;

static void setTitle(void) {
	std::string filename = levelEditor->getOperatingFilename();
	if (filename == "") {
		al_set_window_title(display, "AshEdit2");
	}
	else {
		std::string name = std::string("AshEdit2") + " - " + filename;
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
	
	if(!tileSheets.size() || sheetCombo->getSelected() > (int)tileSheets.size())
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

	int sel_x, sel_y;
	tileSelector->getSelected(&sel_x, &sel_y);

	int rx = dx + ((sel_x*General::tileSize*General::scale) - ox);
	int ry = dy + ((sel_y*General::tileSize*General::scale) - oy);

	al_draw_rectangle(
		rx+0.5,
		ry+0.5,
		rx+General::tileSize*General::scale-0.5,
		ry+General::tileSize*General::scale-0.5,
		al_color_name("black"),
		1
	);
	al_draw_rectangle(
		rx+1.5,
		ry+1.5,
		rx+General::tileSize*General::scale-1.5,
		ry+General::tileSize*General::scale-1.5,
		al_color_name("yellow"),
		1
	);
	al_draw_rectangle(
		rx+2.5,
		ry+2.5,
		rx+General::tileSize*General::scale-2.5,
		ry+General::tileSize*General::scale-2.5,
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
			if (solids[layerCombo->getSelected()]) {
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
}

static void simulate(void)
{
	int offsx = 0;
	int offsy = 0;
	int dx = 0;
	int dy = 0;
	int layers = levelEditor->getLayers();
	int currLayer = levelEditor->getLayer();
	int w = al_get_display_width(display);
	int h = al_get_display_height(display);

	while (true) {
		ALLEGRO_EVENT event;
		al_wait_for_event(queue, &event);
		if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
			if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
				offsx -= 10;
				if (offsx < 0) {
					offsx += 10;
					dx += 10;
				}
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
				if (dx > 0)
					dx -= 10;
				else
					offsx += 10;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_UP) {
				offsy -= 10;
				if (offsy < 0) {
					offsy += 10;
					dy += 10;
				}
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_DOWN) {
				if (dy > 0)
					dy -= 10;
				else
					offsy += 10;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				return;
			}
		}
		al_clear_to_color(al_map_rgb(0, 0, 0));
		for (int i = 0; i <= currLayer; i++) {
			levelDrawCallback(offsx, offsy, dx, dy, w, h, i);
		}
		al_draw_bitmap(ryori_bmp, (w-al_get_bitmap_width(ryori_bmp))/2, (h-al_get_bitmap_height(ryori_bmp))/2, 0);
		for (int i = currLayer+1; i < layers; i++) {
			levelDrawCallback(offsx, offsy, dx, dy, w, h, i);
		}
		al_flip_display();
	}
}

std::string selectDir(const std::string &start)
{
	std::string tmp = start;
	tmp += std::string("/");
	ALLEGRO_FILECHOOSER *diag = al_create_native_file_dialog(tmp.c_str(), "Select Tile Set", 0, ALLEGRO_FILECHOOSER_FOLDER | ALLEGRO_FILECHOOSER_FILE_MUST_EXIST);
	al_show_native_file_dialog(display, diag);
	
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
	if(!al_open_directory(path_entry)) {
		fprintf(stderr, "failed to list directory '%s': %s\n", path, strerror(errno));
		return;
	}
	
	std::vector<ALLEGRO_PATH *> items;
	ALLEGRO_FS_ENTRY *ent = 0;
	while((ent = al_read_directory(path_entry))) {
		const char *ent_path = al_get_fs_entry_name(ent);
		
		ALLEGRO_PATH *path = al_create_path(ent_path);
		if(!path) {
			fprintf(stderr, "failed to create path object for loading tile sheet '%s': %s\n", ent_path, strerror(errno));
			continue;
		}
		
		const char *ent_name = al_get_path_filename(path);
		if(ent_name[0] == '.') continue; // skip hidden files, the parent ref, and self ref entries
		
		const char *ent_ext = al_get_path_extension(path);
		if(strcmp(ent_ext, ".png") == 0 && strncmp(ent_name, "tiles", 5) == 0) {
			//fprintf(stderr, "found: %s\n", al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP));
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
		//fprintf(stderr, "loaded tilesheet: %s\n", al_path_cstr(items[i], ALLEGRO_NATIVE_PATH_SEP));
		al_destroy_path(items[i]);
		
		char buf[20];
		sprintf(buf, "Sheet %d", i);
		sheetCombo->addValue(std::string(buf));
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

	al_set_new_display_flags(ALLEGRO_RESIZABLE);
	display = al_create_display(800, 600);
	ALLEGRO_FONT *font = al_load_ttf_font("DejaVuSans.ttf", 12, 0);
	queue = al_create_event_queue();
	ALLEGRO_TIMER *draw_timer = al_create_timer(1.0/20.0);
	al_start_timer(draw_timer);

	ALLEGRO_FILE *f = al_open_memfile(Ryori_png, sizeof(Ryori_png), "rb");
	ryori_bmp = al_load_bitmap_f(f, ".png");
	al_fclose(f);

	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_timer_event_source(draw_timer));

	tgui::init(display);
	tgui::setFont(font);
	
	al_register_event_source(queue, tgui::getEventSource());

	A_Splitter *mainSplitTop = new A_Splitter(A_Splitter::SPLIT_HORIZONTAL);
	A_Splitter *mainSplitBottom= new A_Splitter(A_Splitter::SPLIT_HORIZONTAL);
	A_Splitter *canvasSplit = new A_Splitter(A_Splitter::SPLIT_VERTICAL);
	A_Splitter *canvasSplitLeft = new A_Splitter(A_Splitter::SPLIT_HORIZONTAL);
	A_Splitter *canvasSplitRight = new A_Splitter(A_Splitter::SPLIT_HORIZONTAL);
	canvasSplit->setResizable(true);

	A_Menubar *menubar = new A_Menubar();
	A_Menu *fileMenu = new A_Menu("File");
	A_Menu *editMenu = new A_Menu("Edit");
	A_Menu *helpMenu = new A_Menu("Help");
	A_Menuitem *fileOpen = new A_Menuitem("Open...");
	A_Menuitem *fileLoadTiles = new A_Menuitem("Load Tiles...");
	A_Menuitem *fileSave = new A_Menuitem("Save");
	A_Menuitem *fileSaveAs = new A_Menuitem("Save As...");
	A_Menuitem *fileSimulate = new A_Menuitem("Simulate");
	A_Menuitem *editUndo = new A_Menuitem("Undo   Ctrl-Z");
	A_Menuitem *editRedo = new A_Menuitem("Redo   Ctrl-Shift-Z");
	A_Menuitem *helpQuickRef = new A_Menuitem("Quick Reference");
	menubar->addMenu(fileMenu);
	menubar->addMenu(editMenu);
	menubar->addMenu(helpMenu);
	fileMenu->addMenuitem(fileOpen);
	fileMenu->addMenuitem(fileSave);
	fileMenu->addMenuitem(fileSaveAs);
	fileMenu->addMenuitem(fileSimulate);
	fileMenu->addMenuitem(fileLoadTiles);
	editMenu->addMenuitem(editUndo);
	editMenu->addMenuitem(editRedo);
	helpMenu->addMenuitem(helpQuickRef);

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
		loadTileSheets(argv[1]);
	}
	else {
		loadTileSheets(".");
	}
	
	levelScrollpane = new A_Scrollpane(
		al_color_name("gray"),
		al_color_name("lightgrey")
	);
	A_Scrollpane *tileScrollpane = new A_Scrollpane(
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

	tgui::setNewWidgetParent(0);
	tgui::addWidget(mainSplitTop);
	mainSplitTop->addToFirstPane(menubar);
	mainSplitTop->addToSecondPane(mainSplitBottom);
	mainSplitBottom->addToSecondPane(statusLabel);
	mainSplitBottom->addToFirstPane(canvasSplit);
	canvasSplit->addToFirstPane(canvasSplitLeft);
	canvasSplit->addToSecondPane(canvasSplitRight);
	canvasSplitLeft->addToFirstPane(layerCombo);
	canvasSplitLeft->addToSecondPane(levelScrollpane);
	canvasSplitRight->addToFirstPane(sheetCombo);
	canvasSplitRight->addToSecondPane(tileScrollpane);

	tgui::setFocus(mainSplitTop);

	mainSplitTop->setSplitSize(20, -1);
	mainSplitBottom->setSplitSize(-1, 20);
	canvasSplitLeft->setSplitSize(20, -1);
	canvasSplitRight->setSplitSize(20, -1);

	tgui::resize(NULL); // similar to java pack(), on all widgets

	if (argc > 2) {
      std::string level_data = argv[2];
      printf("loading level data: %s\n", level_data.c_str());
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
				
				int ret = al_show_native_message_box(display, "Confirm Exit", "Really?", "Seriously?", 0, ALLEGRO_MESSAGEBOX_YES_NO);
				if(ret == 1) {
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
			else if ((event.type == ALLEGRO_EVENT_KEY_CHAR && event.keyboard.keycode == ALLEGRO_KEY_S &&
						(event.keyboard.modifiers & ALLEGRO_KEYMOD_CTRL)))
			{
				levelEditor->save(false);
				setTitle();
				continue;
			}
			else if ((event.type == ALLEGRO_EVENT_KEY_UP && event.keyboard.keycode >= ALLEGRO_KEY_F1 &&
                   event.keyboard.keycode <= ALLEGRO_KEY_F12))
         {
            int layer = (event.keyboard.keycode - ALLEGRO_KEY_F1);
            if(layer < 0 || layer >= levelEditor->getNumLayers())
               continue;
            
            if(levelEditor->getLayer() != layer) {
               levelEditor->setLayer(layer);
               layerCombo->setSelected(layer);
            } else {
               levelEditor->toggleLayerVisibility(layer);
            }
         }
			else if (event.type == ALLEGRO_GET_EVENT_TYPE('T', 'G', 'U', 'I')) {
				ALLEGRO_USER_EVENT *u = (ALLEGRO_USER_EVENT *)&event;
				int type = (int)u->data1;
				if (type == tgui::TGUI_EVENT_OBJECT) {
					tgui::TGUIWidget *widget = (tgui::TGUIWidget *)u->data2;
					if (widget == fileOpen) {
						levelEditor->load();
						setTitle();
					}
					else if (widget == fileSave) {
						levelEditor->save(false);
						setTitle();
					}
					else if (widget == fileSaveAs) {
						levelEditor->save(true);
						setTitle();
					}
					else if (widget == fileSimulate) {
						simulate();
					}
					else if (widget == fileLoadTiles) {
						const char *start = argc > 1 ? argv[1] : al_get_current_directory();
						std::string load_path = selectDir(start);
						loadTileSheets(load_path.c_str());
						
						if(tileSheets.size()) {
							tileScrollpane->setScrollSize(
								al_get_bitmap_width(tileSheets[0]),
								al_get_bitmap_height(tileSheets[0])
							);
						}
					}
					else if (widget == editUndo) {
						levelEditor->doUndo();
					}
					else if (widget == editRedo) {
						levelEditor->doRedo();
					}
					else if (widget && widget == quickRefTitlebar) {
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
					else if (widget == helpQuickRef) {
						quickRefFrame = new A_Frame(al_color_name("blue"), 450, 450);
						quickRefFrame->setPosition(50, 50);
						quickRefSplitter = new A_Splitter(A_Splitter::SPLIT_HORIZONTAL);
						quickRefBottomSplitter = new A_Splitter(A_Splitter::SPLIT_VERTICAL);
						quickRefContent1 = new A_Label(
							"NO UNDO for these!\n"
							"\n"
							"Ctrl-R\n"
							"Shift-Ctrl-R\n"
							"Ctrl-C\n"
							"Shift-Ctrl-C\n"
							"Ctrl-Delete\n"
							"Shift-Ctrl-Delete\n"
							"Ctrl-L\n"
							"Shift-Ctrl-L\n"
							"Ctrl-Alt-L\n"
							"B\n"
							"C\n"
							"S\n"
							"M\n"
							"K\n"
							"V\n"
							"F\n"
							"Shift-F\n"
							"R\n"
							"T\n"
							"Enter\n",
							al_color_name("white")
						);
						quickRefContent2 = new A_Label(
							"\n"
							"\n"
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
							"Save as PNG\n",
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
				tileSelector->getSelected(&tileX, &tileY);
				
				int tileNumber = 0;
				if(tileSheets.size()) {
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

