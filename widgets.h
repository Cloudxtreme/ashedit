#ifndef WIDGET_H
#define WIDGETS_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>

#include "tgui2.hpp"

#include <string>
#include <cmath>
#include <cstdio>
#include <stack>

#include "general.h"

extern ALLEGRO_DISPLAY *display;
extern std::vector<ALLEGRO_BITMAP *> tileSheets;

struct Point {
	float x, y;
};

class A_Splitter_Resizable : public tgui::TGUIWidget {
public:
	virtual void paneResize(tgui::TGUIWidget *pane, float *xx, float *yy, float *w, float *h) = 0;
};

class A_Splitter_Panel : public tgui::TGUIWidget {
public:
	virtual void translate(int xx, int yy) {
	}

	virtual void chainDraw(void) {
		int prev_x, prev_y, prev_w, prev_h;
		int abs_x, abs_y;
		tgui::getClip(&prev_x, &prev_y, &prev_w, &prev_h);
		tgui::determineAbsolutePosition(this, &abs_x, &abs_y);
		tgui::setClip(abs_x, abs_y, width, height);
		if (child) {
			child->chainDraw();
		}
		tgui::setClip(prev_x, prev_y, prev_w, prev_h);
	}

	virtual void resize(void) {
		splitter->paneResize(this, &x, &y, &width, &height);
		resize_child();
	}

	A_Splitter_Panel(A_Splitter_Resizable *splitter) :
		splitter(splitter)
	{
		x = 0;
		y = 0;
		width = 0;
		height = 0;
	}

	A_Splitter_Resizable *splitter;
};



class A_Splitter : public A_Splitter_Resizable {
public:
	static const int MOUSE_MOVE = 0;
	static const int MOUSE_UP = 1;
	static const int MOUSE_DOWN = 2;

	static const int SPLIT_HORIZONTAL = 0;
	static const int SPLIT_VERTICAL = 1;

	virtual void chainKeyDown(int keycode) {
		if (first_pane->getChild()) {
			first_pane->getChild()->chainKeyDown(keycode);
		}
		if (second_pane->getChild()) {
			second_pane->getChild()->chainKeyDown(keycode);
		}
	}

	virtual void chainKeyUp(int keycode) {
		if (first_pane->getChild()) {
			first_pane->getChild()->chainKeyUp(keycode);
		}
		if (second_pane->getChild()) {
			second_pane->getChild()->chainKeyUp(keycode);
		}
	}

	virtual void remove(void) {
		TGUIWidget::remove();

		if (first_pane) {
			first_pane->remove();
		}
		if (second_pane) {
			second_pane->remove();
		}
	}

	virtual void translate(int xx, int yy) {
	/*
		x += xx;
		y += yy;

		if (first_pane) {
			first_pane->translate(xx, yy);
		}
		if (second_pane) {
			second_pane->translate(xx, yy);
		}
		*/

		resize();
	}

	virtual void raise(void) {
		TGUIWidget::raise();
	
		if (first_pane) {
			first_pane->raise();
			/*
			TGUIWidget *child = first_pane->getChild();
			if (child) {
				child->raise();
			}
			*/
		}
		if (second_pane) {
			second_pane->raise();
			/*
			TGUIWidget *child = second_pane->getChild();
			if (child) {
				child->raise();
			}
			*/
		}
	}

	virtual void lower(void) {
		TGUIWidget::lower();

		if (first_pane && first_pane->getChild()) {
			first_pane->getChild()->lower();
		}
		if (second_pane && second_pane->getChild()) {
			second_pane->getChild()->lower();
		}
	}

	virtual void draw(int abs_x, int abs_y) {
		if (!resizable) return;

		// Draw the divider

		ALLEGRO_COLOR colors[4] = {
			al_map_rgb(0, 0, 0),
			al_map_rgb(255, 255, 255),
			al_map_rgb(255, 255, 255),
			al_map_rgb(0, 0, 0)
		};

		for (int i = 0; i < 4; i++) {
			if (split_type == SPLIT_HORIZONTAL) {
				float yy = abs_y+first_pixels+0.5+i;
				al_draw_line(abs_x, yy, abs_x+width, yy,
					colors[i], 1);
			}
			else {
				float xx = abs_x+first_pixels+0.5+i;
				al_draw_line(xx, abs_y, xx, abs_y+height,
					colors[i], 1);
			}
		}
	}

	TGUIWidget *passMouseOn(int type, int rel_x, int rel_y, int abs_x, int abs_y, int mb, int z, int w) {
		TGUIWidget *target;
		if (split_type == SPLIT_HORIZONTAL) {
			if (rel_y < first_pixels) {
				target = first_pane->getChild();
			}
			else if (rel_y >= first_pixels+4) {
				target = second_pane->getChild();
			}
			else {
				target = this;
			}
		}
		else {
			if (rel_x < first_pixels) {
				target = first_pane->getChild();
			}
			else if (rel_x >= first_pixels+4) {
				target = second_pane->getChild();
			}
			else {
				target = this;
			}
		}

		abs_x /= General::scale;
		abs_y /= General::scale;
		int target_x, target_y;
		tgui::determineAbsolutePosition(target, &target_x, &target_y);
		rel_x = abs_x - target_x;
		rel_y = abs_y - target_y;

		if (target == this) {
			switch (type) {
				case MOUSE_MOVE:
					mouseMove(rel_x, rel_y, abs_x, abs_y);
					mouseScroll(z, w);
					break;
				case MOUSE_DOWN:
					mouseDown(rel_x, rel_y, abs_x, abs_y, mb);
					break;
				default:
					mouseUp(rel_x, rel_y, abs_x, abs_y, mb);
					break;
			}
			return this;
		}
	
		if (type == MOUSE_MOVE) {
			return target->chainMouseMove(rel_x, rel_y, abs_x, abs_y, z, w);
		}
		else if (type == MOUSE_DOWN) {
			return target->chainMouseDown(rel_x, rel_y, abs_x, abs_y, mb);
		}
		else {
			return target->chainMouseUp(rel_x, rel_y, abs_x, abs_y, mb);
		}
	}

	virtual TGUIWidget *chainMouseMove(int rel_x, int rel_y, int abs_x, int abs_y, int z, int w)
	{
		TGUIWidget *ret = NULL;

		if (pointOnWidget(this, abs_x, abs_y)) {
			ret = passMouseOn(MOUSE_MOVE, rel_x, rel_y, abs_x, abs_y, 0, z, w);
		}

		return ret;
	}

	virtual TGUIWidget *chainMouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
	{
		TGUIWidget *ret = NULL;

		if (pointOnWidget(this, abs_x, abs_y)) {
			ret = passMouseOn(MOUSE_DOWN, rel_x, rel_y, abs_x, abs_y, mb, 0, 0);
		}

		return ret;
	}

	virtual TGUIWidget *chainMouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
	{
		TGUIWidget *ret = NULL;

		if (pointOnWidget(this, abs_x, abs_y)) {
			ret = passMouseOn(MOUSE_UP, rel_x, rel_y, abs_x, abs_y, mb, 0, 0);
		}

		return ret;
	}

	virtual void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y) {
		if (down_on_slider) {
			int slider_delta;
			int size;
			if (split_type == SPLIT_HORIZONTAL) {
				slider_delta = abs_y - last_slider_pos;
				size = height;
			}
			else {
				slider_delta = abs_x - last_slider_pos;
				size = width;
			}
			first_pixels += slider_delta;
			second_pixels -= slider_delta;
			last_slider_pos += slider_delta;
			ratio = (float)first_pixels / (size-4);
			resize();
		}
		else {
			int my_x, my_y;
			bool on;
			tgui::determineAbsolutePosition(this, &my_x, &my_y);
			if (split_type == SPLIT_HORIZONTAL) {
				if (abs_x >= my_x && abs_x < my_x+width)
					on = (abs_y >= first_pixels && abs_y < first_pixels+4);
				else
					on = false;
			}
			else {
				if (abs_y >= my_y && abs_y < my_y+height)
					on = (abs_x >= first_pixels && abs_x < first_pixels+4);
				else
					on = false;
			}
			if (resizable && on) {
				ALLEGRO_SYSTEM_MOUSE_CURSOR type = (split_type == SPLIT_HORIZONTAL) ? ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_N : ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_W;
				if (cursor_type != type) {
					al_set_system_mouse_cursor(al_get_current_display(), type);
					cursor_type = type;
				}
			}
			else {
				if (cursor_type != ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT) {
					al_set_system_mouse_cursor(al_get_current_display(), ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
					cursor_type = ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT;
				}
			}
		}
	}

	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		int my_x, my_y;
		bool on;
		tgui::determineAbsolutePosition(this, &my_x, &my_y);
		if (split_type == SPLIT_HORIZONTAL) {
			if (abs_x >= my_x && abs_x < my_x+width)
				on = (abs_y >= first_pixels && abs_y < first_pixels+4);
			else
				on = false;
		}
		else {
			if (abs_y >= my_y && abs_y < my_y+height)
				on = (abs_x >= first_pixels && abs_x < first_pixels+4);
			else
				on = false;
		}
		if (resizable && on) {
			down_on_slider = true;
			last_slider_pos = (split_type == SPLIT_HORIZONTAL) ? abs_y : abs_x;
		}
	}
	
	void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (down_on_slider) {
			down_on_slider = false;
		}
	}

	void setSplitRatio(float p1, float p2) {
		if (split_type == SPLIT_HORIZONTAL) {
			first_pixels = (height-4) * p1;
			second_pixels = (height-4) * p2;
		}
		else {
			first_pixels = (width-4) * p1;
			second_pixels = (width-4) * p2;
		}
		ratio = p1;
		adjusted_size = true;
	};

	void setSplitSize(int a, int b) {
		if (a != -1 && b != -1) {
			first_pixels = a;
			second_pixels = b;
			first_fixed = second_fixed = true;
		}
		else if (split_type == SPLIT_HORIZONTAL) {
			if (a == -1 && b == -1) {
				first_pixels = (height-4)/2;
				second_pixels = (height-4) - first_pixels;
			}
			else if (a == -1) {
				second_pixels = b;
				first_pixels = (height-4) - second_pixels;
				second_fixed = true;
			}
			else if (b == -1) {
				first_pixels = a;
				second_pixels = (height-4) - first_pixels;
				first_fixed = true;
			}
			ratio = first_pixels / (height-4);
		}
		else {
			if (a == -1 && b == -1) {
				first_pixels = (width-4)/2;
				second_pixels = (width-4) - first_pixels;
			}
			else if (a == -1) {
				second_pixels = b;
				first_pixels = (width-4) - second_pixels;
				second_fixed = true;
			}
			else if (b == -1) {
				first_pixels = a;
				second_pixels = (width-4) - first_pixels;
				first_fixed = true;
			}
			ratio = first_pixels / (width-4);
		}
		adjusted_size = true;
	}

	void setResizable(bool r) {
		resizable = r;
	}

	// you should not add widgets to things retrieved from these 2 functions
	A_Splitter_Panel *getFirstPane(void) { return first_pane; }
	A_Splitter_Panel *getSecondPane(void) { return second_pane; }

	void addToFirstPane(TGUIWidget *widget) {
		first_pane->setChild(widget);
		widget->setParent(first_pane);
		TGUIWidget *oldParent = tgui::getNewWidgetParent();
		tgui::setNewWidgetParent(first_pane);
		tgui::addWidget(widget);
		tgui::setNewWidgetParent(oldParent);
	}

	void addToSecondPane(TGUIWidget *widget) {
		second_pane->setChild(widget);
		widget->setParent(second_pane);
		TGUIWidget *oldParent = tgui::getNewWidgetParent();
		tgui::setNewWidgetParent(second_pane);
		tgui::addWidget(widget);
		tgui::setNewWidgetParent(oldParent);
	}

	virtual void resize(void) {
		resize_self();

		if (first_fixed) {
			setSplitSize(first_pixels, -1);
		}
		else if (second_fixed) {
			setSplitSize(-1, second_pixels);
		}
		else if (!first_fixed && !second_fixed) {
			setSplitRatio(ratio, 1.0-ratio);
		}

		if (first_pane)
			first_pane->resize();
		if (second_pane)
			second_pane->resize();
	}

	A_Splitter(int split_type) :
		split_type(split_type),
		first_pixels(0),
		second_pixels(0),
		resizable(false),
		down_on_slider(false),
		cursor_type(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT),
		adjusted_size(false),
		first_fixed(false),
		second_fixed(false),
		ratio(0.5)
	{
		x = 0;
		y = 0;
		width = 0;
		height = 0;

		first_pane = new A_Splitter_Panel(this);
		second_pane = new A_Splitter_Panel(this);
		TGUIWidget *oldParent = tgui::getNewWidgetParent();
		tgui::setNewWidgetParent(NULL);
		tgui::addWidget(first_pane);
		tgui::addWidget(second_pane);
		tgui::setNewWidgetParent(oldParent);
	}

protected:

	void paneResize(TGUIWidget *pane, float *xx, float *yy, float *w, float *h)
	{
		int abs_x, abs_y;
		tgui::determineAbsolutePosition(this, &abs_x, &abs_y);

		if (pane == first_pane) {
			*xx = abs_x;
			*yy = abs_y;

			if (split_type == SPLIT_HORIZONTAL) {
				*w = width;
				*h = first_pixels;
			}
			else {
				*w = first_pixels;
				*h = height;
			}
		}
		else {
			if (split_type == SPLIT_HORIZONTAL) {
				*xx = abs_x;
				*yy = abs_y+first_pixels+4;
				*w = width;
				*h = second_pixels;
			}
			else {
				*xx = abs_x+first_pixels+4;
				*yy = abs_y;
				*w = second_pixels;
				*h = height;
			}
		}
	}

	A_Splitter_Panel *first_pane, *second_pane;
	int split_type;
	float first_pixels;
	float second_pixels;
	bool resizable;
	bool down_on_slider;
	int last_slider_pos;
	int cursor_type;
	bool adjusted_size;
	bool first_fixed;
	bool second_fixed;
	float ratio;
};
	
class A_Image : public tgui::TGUIWidget {
public:
	virtual void draw(int abs_x, int abs_y) {
		al_draw_scaled_bitmap(
			image,
			0,
			0,
			al_get_bitmap_width(image),
			al_get_bitmap_height(image),
			abs_x,
			abs_y,
			width, height,
			0
		);
	}

	A_Image(ALLEGRO_BITMAP *bmp)
	{
		this->image = bmp;
		x = 0;
		y = 0;
		width = al_get_bitmap_width(bmp);
		height = al_get_bitmap_height(bmp);
	}
protected:
	ALLEGRO_BITMAP *image;
};

class A_Frame : public tgui::TGUIWidget {
public:
	static const int PADDING = 5;

	virtual void translate(int xx, int yy) {
		x += xx;
		y += yy;

		if (child)
			child->translate(xx, yy);
	}

	virtual void draw(int abs_x, int abs_y) {
		al_draw_filled_rectangle(
			abs_x+PADDING,
			abs_y+PADDING,
			abs_x+width+PADDING,
			abs_y+height+PADDING,
			al_map_rgba(0, 0, 0, 100)
		);

		al_draw_filled_rectangle(
			abs_x,
			abs_y,
			abs_x+width,
			abs_y+height,
			color
		);
		
		al_draw_line(
			abs_x+0.5,
			abs_y+0.5,
			abs_x+width-0.5,
			abs_y+0.5,
			al_map_rgba(255, 255, 255, 150),
			1
		);

		al_draw_line(
			abs_x+0.5,
			abs_y+1.5,
			abs_x+0.5,
			abs_y+height-0.5,
			al_map_rgba(255, 255, 255, 150),
			1
		);

		al_draw_line(
			abs_x+width-0.5,
			abs_y+1.5,
			abs_x+width-0.5,
			abs_y+height-0.5,
			al_map_rgba(0, 0, 0, 150),
			1
		);

		al_draw_line(
			abs_x+1.5,
			abs_y+height-0.5,
			abs_x+width-1.5,
			abs_y+height-0.5,
			al_map_rgba(0, 0, 0, 150),
			1
		);

	}

	void setPosition(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	virtual void resize(void) {
		resize_child();
	}

	void setSize(int w, int h) {
		width = w;
		height = h;
	}

	A_Frame(ALLEGRO_COLOR color)
	{
		this->color = color;
	}

	A_Frame(ALLEGRO_COLOR color, int w, int h)
	{
		this->color = color;
		width = w;
		height = h;
	}

protected:
	ALLEGRO_COLOR color;
};

class A_Titlebar : public tgui::TGUIWidget {
public:
	static const int CLOSE_BUTTON = 1;

	virtual void draw(int abs_x, int abs_y) {
		al_draw_filled_rectangle(
			abs_x+1,
			abs_y+1,
			abs_x+width-1,
			abs_y+height-1,
			color
		);

		al_draw_text(
			tgui::getFont(),
			al_color_name("white"),
			abs_x+4,
			abs_y+4,
			0,
			caption.c_str()
		);
		al_draw_text(
			tgui::getFont(),
			al_color_name("black"),
			abs_x+3,
			abs_y+3,
			0,
			caption.c_str()
		);

		if (flags & CLOSE_BUTTON) {
			al_draw_filled_rectangle(
				abs_x+width-(CLOSE_BUTTON_SIZE+2),
				abs_y+2,
				abs_x+width-2,
				abs_y+CLOSE_BUTTON_SIZE+2,
				al_color_name("red")
			);
			al_draw_rectangle(
				(abs_x+width-(CLOSE_BUTTON_SIZE+2))+0.5,
				(abs_y+2)+0.5,
				(abs_x+width-2)+0.5,
				(abs_y+CLOSE_BUTTON_SIZE+2)+0.5,
				al_color_name("white"),
				1
			);
		}
	}

	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		int a, b;
		tgui::determineAbsolutePosition(this, &a, &b);
		if (mb == 1 && rel_x >= 0) {
			if (
				(flags & CLOSE_BUTTON) && 
				(rel_x >= width-(CLOSE_BUTTON_SIZE+2)) &&
				(rel_x < width-2) &&
				(rel_y >= 2) &&
				(rel_y < CLOSE_BUTTON_SIZE+2)
			) {
				tgui::pushEvent(tgui::TGUI_EVENT_OBJECT, (void *)this);
				return;
			}
			down = true;
			downX = abs_x;
			downY = abs_y;
		}
	}

	virtual void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (mb == 1) {
			down = false;
		}
	}

	virtual void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y) {
		if (down) {
			int dx = abs_x - downX;
			int dy = abs_y - downY;
			downX = abs_x;
			downY = abs_y;
			parentFrame->translate(dx, dy);
		}
	}

	A_Titlebar(TGUIWidget *parentFrame, std::string caption, ALLEGRO_COLOR color, int flags) :
		parentFrame(parentFrame),
		caption(caption),
		color(color),
		flags(flags),
		down(false)
	{
		x = y = 0;
	}

protected:
	static const int CLOSE_BUTTON_SIZE = 12;

	TGUIWidget *parentFrame;
	std::string caption;
	ALLEGRO_COLOR color;
	int flags;
	bool down;
	int downX, downY;
};

class A_Button : public A_Frame {
public:
	static const int PADDING = 5;

	virtual void draw(int abs_x, int abs_y) {
		A_Frame::draw(abs_x, abs_y);

		al_draw_text(
			tgui::getFont(),
			al_color_name("white"),
			abs_x+PADDING,
			abs_y+PADDING,
			0,
			text.c_str()
		);

	}

	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (rel_x >= 0)
			exit(0);
	}

	A_Button(std::string text, ALLEGRO_COLOR color) :
		A_Frame(color)
	{
		this->text = text;
		this->color = color;
		width = al_get_text_width(
			tgui::getFont(),
			text.c_str()
		) + PADDING*2;
		height = al_get_font_line_height(
			tgui::getFont()
		) + PADDING*2;
	}

protected:
	std::string text;
};

class A_Menu;
typedef void (A_Menu::*A_MenuCloseMethod)(void);

class A_Menuitem : public tgui::TGUIWidget {
public:
	virtual void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y) {
		if (rel_x >= 0)
			highlighted = true;
		else
			highlighted = false;
	}

	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (rel_x >= 0 && mb == 1) {
			tgui::pushEvent(tgui::TGUI_EVENT_OBJECT, (void *)this);
			((*menu).*(closeMethod))();
		}
	}

	virtual void draw(int abs_x, int abs_y) {
		ALLEGRO_COLOR tcolor, bcolor;
		
		if (highlighted) {
			tcolor = al_color_name("white");
			bcolor = al_color_name("blue");
		}
		else {
			tcolor = textColor;
			bcolor = bgColor;
		}

		al_draw_filled_rectangle(
			abs_x,
			abs_y,
			abs_x+width,
			abs_y+height,
			bcolor	
		);

		if (divider) {
			al_draw_line(
				abs_x+5,
				abs_y+height/2,
				abs_x+width-10,
				abs_y+height/2,
				al_color_name("gray"),
				1
			);
		}
		else {
			al_draw_text(
				tgui::getFont(),
				tcolor,
				abs_x+3,
				abs_y+3,
				0,
				text.c_str()
			);
		}
	}

	std::string getText(void) {
		return text;
	}
	void setText(std::string text) {
		this->text = text;
	}

	int getHeight(void) {
		return height;
	}
	void setHeight(int h) {
		height = h;
	}

	void setWidth(int w) {
		width = w;
	}

	void setMenuCloseMethod(A_Menu *menu, A_MenuCloseMethod closeMethod) {
		this->menu = menu;
		this->closeMethod = closeMethod;
	}

	A_Menuitem(std::string text) :
		text(text)
	{
		if (text == "-")
			divider = true;
		else
			divider = false;
		x = 0;
		y = 0;
		width = 0;
		height = divider ? 8 : 20;
		bgColor = al_color_name("lightgrey");
		textColor = al_color_name("black");
		highlighted = false;
		parent = NULL;
	}

protected:
	std::string text;
	bool divider;
	ALLEGRO_COLOR bgColor;
	ALLEGRO_COLOR textColor;
	bool highlighted;
	A_Menu *menu;
	A_MenuCloseMethod closeMethod;
};

class A_Menu : public tgui::TGUIWidget {
public:
	static const int PADDING = 5;

	virtual void draw(int abs_x, int abs_y) {
		al_draw_filled_rectangle(
			abs_x+PADDING,
			abs_y+PADDING,
			abs_x+widest+PADDING,
			abs_y+totalHeight+PADDING,
			al_map_rgba(0, 0, 0, 100)
		);

		al_draw_rectangle(
			abs_x-0.5,
			abs_y-0.5,
			abs_x+widest+0.5,
			abs_y+totalHeight+0.5,
			al_map_rgb(0, 0, 0),
			1
		);
	}

	void open(void) {
		TGUIWidget *oldParent = tgui::getNewWidgetParent();
		tgui::setNewWidgetParent(NULL);
		tgui::addWidget(this);
		raise();
		int dy = y;
		for (unsigned int i = 0; i < menuItems.size(); i++) {
			menuItems[i]->setX(x);
			menuItems[i]->setY(dy);
			menuItems[i]->setWidth(widest);
			tgui::addWidget(menuItems[i]);
			menuItems[i]->raise();
			dy += menuItems[i]->getHeight();
		}
		tgui::setNewWidgetParent(oldParent);
	}

	void close(void) {
		for (unsigned int i = 0; i < menuItems.size(); i++) {
			menuItems[i]->remove();
		}
		remove();
	}

	void addMenuitem(A_Menuitem *m) {
		m->setMenuCloseMethod(this, &A_Menu::close);
		menuItems.push_back(m);
		totalHeight += m->getHeight();
		std::string text = m->getText();
		int w = al_get_text_width(
			tgui::getFont(),
			text.c_str()
		);
		if (w+PADDING*2 > widest) {
			widest = w+PADDING*2;
		}
		width = widest;
		height = totalHeight;
	}

	std::string getText(void) {
		return text;
	}

	A_Menu(std::string text) :
		text(text),
		widest(0),
		totalHeight(0)
	{
		parent = NULL;
	};

protected:
	std::string text;
	std::vector<A_Menuitem *> menuItems;
	int widest;
	int totalHeight;
};

class A_Menubar : public tgui::TGUIWidget {
public:
	static const int PADDING = 10;

	virtual void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y) {
		if (rel_x >= 0) {
			int i = getClosestMenu(rel_x);
			highlighted = i;
		}
		else {
			highlighted = -1;
		}
	}

	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (mb != 1) return;

		if (rel_x < 0) {
			if (openMenu >= 0) {
				int mx = menus[openMenu]->getX();
				int my = menus[openMenu]->getY();
				int mw = menus[openMenu]->getWidth();
				int mh = menus[openMenu]->getHeight();
				bool onMenu = (abs_x >= mx && abs_x < mx+mw && abs_y >= my && abs_y < my+mh);
				if (!onMenu) {
					menus[openMenu]->close();
					openMenu = -1;
				}
			}
			return;
		}
	
		int closest = getClosestMenu(rel_x);
		if (openMenu >= 0) {
			menus[openMenu]->close();
		}
		openMenu = closest;

		if (openMenu >= 0) {
			int abs_x, abs_y;
			tgui::determineAbsolutePosition(this, &abs_x, &abs_y);
			int wx = abs_x+textOffsets[openMenu]-PADDING;
			int wy = abs_y+height;
			menus[openMenu]->setX(wx);
			menus[openMenu]->setY(wy);
			menus[openMenu]->open();
		}
	}

	virtual void draw(int abs_x, int abs_y) {
		al_draw_filled_rectangle(
			abs_x, abs_y,
			abs_x+width, abs_y+height,
			bgColor
		);

		al_draw_line(abs_x, abs_y+height, abs_x+width, abs_y+height, al_color_name("black"), 1);

		for (unsigned int i = 0; i < menus.size(); i++) {
			ALLEGRO_COLOR color = textColor;
			if ((int)i == highlighted) {
				int next;
				if (i == textOffsets.size()-1) {
					next = currTextOffset;
				}
				else {
					next = textOffsets[i+1];
				}
				al_draw_filled_rectangle(
					abs_x+textOffsets[i]-PADDING,
					abs_y,
					abs_x+next-PADDING,
					abs_y+height,
					al_color_name("blue")
				);
				color = al_color_name("white");
			}
			al_draw_text(
				tgui::getFont(),
				color,
				abs_x+textOffsets[i],
				abs_y+3,
				0,
				menus[i]->getText().c_str()
			);
		}
	}

	void addMenu(A_Menu *m)
	{
		menus.push_back(m);
		textOffsets.push_back(currTextOffset);
		currTextOffset += al_get_text_width(tgui::getFont(), m->getText().c_str())+PADDING*2;
	}
	
	A_Menubar(void) :
		currTextOffset(PADDING),
		openMenu(-1),
		highlighted(-1)
	{
		x = y = 0;
		bgColor = al_color_name("lightgrey");
		textColor = al_color_name("black");
	}

protected:
	int getClosestMenu(int rel_x) {
		if (rel_x > currTextOffset) {
			return -1;
		}

		int closest = -1;
		int closest_dist = INT_MAX;

		for (unsigned int i = 0; i < textOffsets.size(); i++) {
			int next;
			if (i == textOffsets.size()-1) {
				next = currTextOffset;
			}
			else {
				next = textOffsets[i+1];
			}
			int curr = textOffsets[i];
			int mid = ((next-curr)-PADDING*2)/2 + curr;
			int dist = abs(mid-rel_x);
			if (dist < closest_dist) {
				closest = i;
				closest_dist = dist;
			}
		}

		return closest;
	}

	std::vector<A_Menu *> menus;
	std::vector<int> textOffsets;
	int currTextOffset;
	int openMenu;
	ALLEGRO_COLOR bgColor;
	ALLEGRO_COLOR textColor;
	int highlighted;
};

class A_Combobox : public tgui::TGUIWidget {
public:
	static const int HEIGHT = 20;

	int getSelected(void) {
		return selected;
	}

	void setSelected(int s) {
		selected = s;
	}

	int getShow(void)
	{
		int show;
		if ((int)values.size() < maxShown) {
			show = values.size();
		}
		else {
			show = maxShown;
		}
		return show;
	}
	
	int getIndex(int abs_x, int abs_y) {
		int widget_x, widget_y;
		tgui::determineAbsolutePosition(this, &widget_x, &widget_y);
		int rel_x = abs_x - widget_x;
		int rel_y = abs_y - widget_y;

		int show = getShow();

		if (rel_x < 0 || rel_y < 0 || rel_x > width || rel_y > (show+2)*HEIGHT)
			return -4;

		int index_tmp = rel_y / HEIGHT;
		
		if (index_tmp == 0)
			return -1;
		else if (index_tmp == show+1)
			return -2;
		else
			return index_tmp-1;
	}

	int getSize(void) {
		return (int)values.size();
	}

	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (mb != 1) return;

		int show = getShow();

		if (rel_x >= 0 && !opened) {
			tgui::getTopLevelParent(this)->raise();
			height = (getShow()+2)*HEIGHT;

			opened = true;

			if (selected+show >= (int)values.size()) {
				top = values.size() - show;
			}
			else {
				top = selected;
			}
			hover = -1;
			return;
		}

		if (!opened)
			return;

		// was opened
	
		int index = getIndex(abs_x, abs_y);

		if (index == -1) {
			if (top > 0) {
				top--;
			}
		}
		else if (index == -2) {
			if (top < (int)values.size()-show) {
				top++;
			}
		}
		else if (index >= 0) {
			selected = top+index;
			opened = false;
			hover = -4;
			height = HEIGHT;
		}
		else {
			opened = false;
			hover = -4;
			height = HEIGHT;
		}
	}

	virtual void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y) {
		if (!opened) {
			if (rel_x >= 0)
				hover = -3;
			else
				hover = -4;
		}
		else {
			int index = getIndex(abs_x, abs_y);

			if (index < 0)
				hover = index;
			else
				hover = top+index;
		}
	}

	virtual void draw(int abs_x, int abs_y) {
		int prev_x, prev_y, prev_w, prev_h;
		tgui::getClip(&prev_x, &prev_y, &prev_w, &prev_h);
		int w, h;
		tgui::getScreenSize(&w, &h);
		tgui::setClip(0, 0, w, h);

		if (opened) {
			int show = getShow();
			int y;
	
			y = abs_y;
			al_draw_filled_rectangle(
				abs_x,
				y,
				abs_x+width,
				y+HEIGHT,
				hover == -1 ? al_color_name("blue") : bgColor
			);
			al_draw_filled_triangle(
				abs_x+width/2, y+2,
				abs_x+width/2-((HEIGHT-4)/2), y+HEIGHT-2,
				abs_x+width/2+((HEIGHT-4)/2), y+HEIGHT-2,
				top == 0 ? highlightColor : textColor
			);
			y = abs_y+HEIGHT*(show+1);
			al_draw_filled_rectangle(
				abs_x,
				y,
				abs_x+width,
				y+HEIGHT,
				hover == -2 ? al_color_name("blue") : bgColor
			);
			al_draw_filled_triangle(
				abs_x+width/2, y+HEIGHT-2,
				abs_x+width/2-((HEIGHT-4)/2), y+2,
				abs_x+width/2+((HEIGHT-4)/2), y+2,
				top+show >= (int)values.size() ? highlightColor : textColor
			);

			y = abs_y + HEIGHT;

			if(values.size()) {
				for (int i = 0; i < show; i++) {
					ALLEGRO_COLOR color;
					ALLEGRO_COLOR color2;

					if (hover == top+i) {
						color = highlightColor;
						color2 = al_color_name("white");
					}
					else {
						color = bgColor;
						color2 = textColor;
					}

					al_draw_filled_rectangle(
						abs_x,
						y,
						abs_x+width,
						y+HEIGHT,
						color
					);

					al_draw_text(
						tgui::getFont(),
						color2,
						abs_x+4,
						y+4,
						0,
						((top+i == selected ? "*" : "") + values[top+i]).c_str()
					);

					y += HEIGHT;
				}
			}
		}
		else {
			ALLEGRO_COLOR color;

			if (hover == -3) {
				color = highlightColor;
			}
			else {
				color = bgColor;
			}

			al_draw_filled_rectangle(
				abs_x,
				abs_y,
				abs_x+width,
				abs_y+HEIGHT,
				color
			);

			int tx = abs_x+width-15;
			int ty = abs_y+2;

			al_draw_filled_triangle(
				tx, ty,
				tx-5, ty+5,
				tx+5, ty+5,
				textColor
			);

			ty = abs_y + 12;

			al_draw_filled_triangle(
				tx-5, ty,
				tx+5, ty,
				tx, ty+5,
				textColor
			);

			if(values.size()) {
				al_draw_text(
					tgui::getFont(),
					textColor,
					abs_x+4,
					abs_y+4,
					0,
					values[selected].c_str()
				);
			}
		}

		tgui::setClip(prev_x, prev_y, prev_w, prev_h);
	};

	void addValue(std::string value) {
		values.push_back(value);
	}

	void clearValues(void) {
		values.clear();
	}

	A_Combobox(
		int maxShown,
		ALLEGRO_COLOR bgColor,
		ALLEGRO_COLOR textColor,
		ALLEGRO_COLOR highlightColor
	) :
		maxShown(maxShown),
		bgColor(bgColor),
		textColor(textColor),
		highlightColor(highlightColor),
		opened(false),
		hover(-4),
		top(0),
		selected(0)
	{
		x = y = 0;
		if (this->maxShown < 0) {
			this->maxShown = 5;
		}
	}

protected:
	std::vector<std::string> values;
	int maxShown;
	ALLEGRO_COLOR bgColor;
	ALLEGRO_COLOR textColor;
	ALLEGRO_COLOR highlightColor;
	bool opened;
	int hover;
	int top;
	int selected;
};

class A_Scrollable {
public:
	virtual void shelteredDraw(int offs_x, int offs_y, int w, int h) {};
};

class A_Scrollpane : public tgui::TGUIWidget {
public:
	static const int THICKNESS = 20;

	TGUIWidget *chainMouseMove(int rel_x, int rel_y, int abs_x, int abs_y, int z, int w)
	{
		TGUIWidget *widget = _mouseMove(rel_x, rel_y, abs_x, abs_y);
		mouseScroll(z, w);
		return widget;
	}

	virtual void mouseScroll(int z, int w)
	{
		static int prev_z = 0;
		static int prev_w = 0;

		int dz = 32 * (prev_z - z);
		int dw = 32 * (prev_w - w);

		scrollBy(dz, dw);

		prev_z = z;
		prev_w = w;
	}

	int getBarLength(int widgetSize, int scrollSize) {
		if (widgetSize > scrollSize)
			return 0;
		if (widgetSize == 0)
			return 0;
		double screens = (double)scrollSize / widgetSize;
		if (screens == 0)
			return 0;
		int len = widgetSize / screens;
		if (len > widgetSize-1)
			len = widgetSize-1;
		/*
		if (len < 100) len = 100;
		*/
		return len;
	}
	
	int getTotalLength(int widgetSize, int barSize) {
		return widgetSize-barSize-THICKNESS-1;
	}

	void draw(int abs_x, int abs_y) {
		int xx = abs_x+width-THICKNESS;
		int yy = abs_y;
		int xx2 = xx+THICKNESS;
		int yy2 = yy+height-THICKNESS;

		al_draw_filled_rectangle(
			xx, yy, xx2, yy2,
			troughColor
		);

		int barSize = getBarLength(height-THICKNESS, size_y);
		int totalLength = getTotalLength(height, barSize);
		int bpos = p1 * totalLength;

		al_draw_filled_rectangle(
			xx+1,
			abs_y+bpos+1,
			xx2-1,
			abs_y+bpos+barSize,
			sliderColor
		);

		xx = abs_x;
		yy = abs_y+height-THICKNESS;
		xx2 = xx+width-THICKNESS;
		yy2 = yy+THICKNESS;

		al_draw_filled_rectangle(
			xx, yy, xx2, yy2,
			troughColor
		);
		
		barSize = getBarLength(width-THICKNESS, size_x);
		totalLength = getTotalLength(width, barSize);
		bpos = p2 * totalLength;

		al_draw_filled_rectangle(
			abs_x+bpos+1,
			yy+1,
			abs_x+bpos+barSize,
			yy2-1,
			sliderColor
		);
	}

	void setScrollSize(int w, int h) {
		size_x = w;
		size_y = h;
	}

	void getScrollSize(int *w, int *h) {
		if (w)
			*w = size_x;
		if (h)
			*h = size_y;
	}

	void addScrollable(TGUIWidget *w) {
		A_Scrollable *test = dynamic_cast<A_Scrollable *>(w);
		if (!test) {
			return;
		}
		
		w->setParent(this);
		child = w;
	}

	int getOffsetX(void) {
		int ox = p2 * (size_x-(width-THICKNESS));
		if (ox < 0) ox = 0;
		return ox;
	}

	int getOffsetY(void) {
		int oy = p1 * (size_y-(height-THICKNESS));
		if (oy < 0) oy = 0;
		return oy;
	}

	int getSizeX(void) {
		return size_x;
	}

	int getSizeY(void) {
		return size_y;
	}

	virtual void scrollBy(int rel_x, int rel_y) {
		int sw, sh;
		getScrollSize(&sw, &sh);
		p1 += (double)rel_x / sw;
		p2 += (double)rel_y / sh;
		if(p1 < 0) p1 = 0;
		else if(p1 > 1) p1 = 1;
		if(p2 < 0) p2 = 0;
		else if(p2 > 1) p2 = 1;
	}
   
	TGUIWidget *_mouseMove(int rel_x, int rel_y, int abs_x, int abs_y) {
		if (downOnSide) {
			int barSize = getBarLength(height-THICKNESS, size_y);
			int totalLength = getTotalLength(height, barSize);
			p1 = downP + ((float)(abs_y - downAt) / totalLength);
			if (p1 < 0) p1 = 0;
			else if (p1 > 1) p1 = 1;
			mouseUsedBy = this;
		}
		else if (downOnBottom) {
			int barSize = getBarLength(width-THICKNESS, size_x);
			int totalLength = getTotalLength(width, barSize);
			p2 = downP + ((float)(abs_x - downAt) / totalLength);
			if (p2 < 0) p2 = 0;
			else if (p2 > 1) p2 = 1;
			mouseUsedBy = this;
		}
		else if (rel_x >= 0 && child) {
         if(tgui::isKeyDown(ALLEGRO_KEY_LCTRL) && mouseUsedBy == this) {
            // move
            int diff_x = rel_x - start_x;
            int diff_y = rel_y - start_y;
            
            p2 -= diff_x / width;
            if (p2 < 0) p2 = 0;
            else if (p2 > 1) p2 = 1;
         
            p1 -= diff_y / height;
            if (p1 < 0) p1 = 0;
            else if (p1 > 1) p1 = 1;
         
            start_x = rel_x;
            start_y = rel_y;
            mouseUsedBy = this;
         }
         else {
            int cx = rel_x + getOffsetX();
            int cy = rel_y + getOffsetY();
            child->mouseMove(cx, cy, abs_x, abs_y);
            mouseUsedBy = child;
         }
		}

		return mouseUsedBy;
	}

	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (rel_x > (width-THICKNESS)) {
			int barSize = getBarLength(height-THICKNESS, size_y);
			int totalLength = getTotalLength(height, barSize);
			int bpos = p1 * totalLength;
			if (rel_y >= bpos && rel_y <= bpos+barSize) {
				downOnSide = true;
				downAt = abs_y;
				downP = p1;
			}
			mouseUsedBy = this;
		}
		else if (rel_y > (height-THICKNESS)) {
			int barSize = getBarLength(width-THICKNESS, size_x);
			int totalLength = getTotalLength(width, barSize);
			int bpos = p2 * totalLength;
			if (rel_x >= bpos && rel_x <= bpos+barSize) {
				downOnBottom = true;
				downAt = abs_x;
				downP = p2;
			}
			mouseUsedBy = this;
		}
		else if (rel_x >= 0 && child) {
         if(tgui::isKeyDown(ALLEGRO_KEY_LCTRL)) {
            start_x = rel_x;
            start_y = rel_y;
            mouseUsedBy = this;
         }
         else {
            int cx = rel_x + getOffsetX();
            int cy = rel_y + getOffsetY();
            child->mouseDown(cx, cy, abs_x, abs_y, mb);
            mouseUsedBy = child;
         }
		}
	}

	virtual void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (downOnSide || downOnBottom) {
			downOnSide = downOnBottom = false;
			mouseUsedBy = this;
		}
		else if (rel_x >= 0 && child) {
         if(tgui::isKeyDown(ALLEGRO_KEY_LCTRL) && mouseUsedBy == this) {
            // nothing?
            mouseUsedBy = child;
         }
         else {
            int cx = rel_x + getOffsetX();
            int cy = rel_y + getOffsetY();
            child->mouseUp(cx, cy, abs_x, abs_y, mb);
            mouseUsedBy = child;
         }
		}
	}

	/*
	virtual TGUIWidget *chainMouseMove(int rel_x, int rel_y, int abs_x, int abs_y) {
		mouseMove(rel_x, rel_y, abs_x, abs_y);
		return mouseUsedBy;
	}
	*/

	virtual TGUIWidget *chainMouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		mouseDown(rel_x, rel_y, abs_x, abs_y, mb);
		return mouseUsedBy;
	}

	virtual TGUIWidget *chainMouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		mouseUp(rel_x, rel_y, abs_x, abs_y, mb);
		return mouseUsedBy;
	}

	virtual void keyDown(int keycode) {
		int diff_x = 0;
		int diff_y = 0;

		if(tgui::getFocussedWidget() != this && tgui::getFocussedWidget() != child)
			return;
		
		if(keycode == ALLEGRO_KEY_UP) {
			diff_y = General::tileSize;
		}
		else if(keycode == ALLEGRO_KEY_DOWN) {
			diff_y = -General::tileSize;
		}
		else if(keycode == ALLEGRO_KEY_LEFT) {
			diff_x = General::tileSize;
		}
		else if(keycode == ALLEGRO_KEY_RIGHT) {
			diff_x = -General::tileSize;
		} 
		
		p2 -= diff_x / width;
		if (p2 < 0) p2 = 0;
		else if (p2 > 1) p2 = 1;
	
		p1 -= diff_y / height;
		if (p1 < 0) p1 = 0;
		else if (p1 > 1) p1 = 1;
	}

	virtual void chainDraw(void)
	{
		int abs_x, abs_y;
		tgui::determineAbsolutePosition(this, &abs_x, &abs_y);
		draw(abs_x, abs_y);

		if (!child)
			return;
		int prev_x, prev_y, prev_w, prev_h;
		tgui::getClip(&prev_x, &prev_y, &prev_w, &prev_h);
		tgui::setClip(abs_x, abs_y, width-THICKNESS, height-THICKNESS);
		int offs_x = getOffsetX();
		int offs_y = getOffsetY();
		A_Scrollable *scrollable = dynamic_cast<A_Scrollable *>(child);
		scrollable->shelteredDraw(offs_x, offs_y, width-THICKNESS, height-THICKNESS);
		tgui::setClip(prev_x, prev_y, prev_w, prev_h);
	}

	A_Scrollpane(ALLEGRO_COLOR troughColor, ALLEGRO_COLOR sliderColor) :
		size_x(0),
		size_y(0),
		p1(0),
		p2(0),
		troughColor(troughColor),
		sliderColor(sliderColor),
		downOnBottom(false),
		downOnSide(false)
	{
		x = y = 0;
	}

protected:
	int size_x;
	int size_y;
	float p1, p2;
	ALLEGRO_COLOR troughColor;
	ALLEGRO_COLOR sliderColor;
	bool downOnBottom;
	bool downOnSide;
	int downAt;
	float downP;
	TGUIWidget *mouseUsedBy;
   int start_x, start_y;
};

class A_Canvas : public A_Scrollable, public tgui::TGUIWidget {
public:
	typedef void (*drawCallback)(int ox, int oy, int dx, int dy, int w, int h, int layer);

	virtual void shelteredDraw(int offs_x, int offs_y, int w, int h) {
		int abs_x, abs_y;
		tgui::determineAbsolutePosition(this, &abs_x, &abs_y);
		callback(offs_x, offs_y, abs_x, abs_y, w, h, -1);
	}

	A_Canvas(drawCallback callback) :
		callback(callback)
	{
		x = y = 0;
	}

protected:
	drawCallback callback;
};

class A_Tileselector : public A_Canvas {
public:
	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (rel_x >= 0) {
			selected_x = rel_x / General::tileSize / General::scale;
			selected_y = rel_y / General::tileSize / General::scale;
		}
	}

	void getSelected(int *x, int *y) {
		if (x)
			*x = selected_x;
		if (y)
			*y = selected_y;
	}

	void setSelected(int x, int y)
	{
		selected_x = x;
		selected_y = y;
	}

	A_Tileselector(drawCallback callback) :
		A_Canvas(callback),
		selected_x(0),
		selected_y(0)
	{
	}

protected:
	int selected_x;
	int selected_y;
};

class A_Leveleditor : public A_Canvas {
public:
	static const int TOOL_BRUSH = 0;
	static const int TOOL_CLEAR = 1;
	static const int TOOL_SOLID = 2;
	static const int TOOL_MACRO = 3;
	static const int TOOL_CLONE = 4;
	static const int TOOL_MOVER = 5;
	static const int TOOL_FILL_CURRENT = 6;
	static const int TOOL_FILL_ALL = 7;
	static const int TOOL_RAISER = 8;

	struct _TilePlusPlus {
		int x, y, layer, number, sheet;
		bool solid;
		int tool;
	};

	struct _Tile {
		int number;
		int sheet;
		bool solid;
	};

	bool getRecording(void) {
		return recording;
	}

	int getNumLayers(void) {
		return layers;
	}

	int getLayers(void) {
		return layers;
	}

	_Tile createEmpty_Tile(void) {
		_Tile t;
		t.number = -1;
		t.sheet = -1;
		t.solid = false;
		return t;
	}

	std::vector<_Tile> createEmptyTile(void) {
		std::vector<_Tile> tile;
		for (int k = 0; k < layers; k++) {
			_Tile t = createEmpty_Tile();
			tile.push_back(t);
		}
		return tile;
	}
	
	std::vector< std::vector<_Tile> > createRow(int w) {
		std::vector< std::vector<_Tile> > row;
		std::vector<_Tile> tile = createEmptyTile();
		for (int j = 0; j < w; j++) {
			row.push_back(tile);
		}
		return row;
	}

	void resizeScrollpane(void) {
		A_Scrollpane *scrollpane = dynamic_cast<A_Scrollpane *>(parent);
		if (scrollpane) {
			scrollpane->setScrollSize(
				tiles[0].size()*(General::tileSize*General::scale),
				tiles.size()*(General::tileSize*General::scale)
			);
		}
	}

	void clearUndo(void) {
		undo.clear();
		undoes.clear();
	}

	void deleteLayer(int i) {
		if (layers < 2) return;

		for (unsigned int y = 0; y < tiles.size(); y++) {
			for (unsigned int x = 0; x < tiles[y].size(); x++) {
				std::vector<_Tile>::iterator it = tiles[y][x].begin() + i;
				tiles[y][x].erase(it);
			}
		}
		visible.erase(visible.begin() + i);
		layers--;
	}

	void insertLayer(int i) {
		layers++;
		_Tile t = createEmpty_Tile();
		for (unsigned int y = 0; y < tiles.size(); y++) {
			for (unsigned int x = 0; x < tiles[y].size(); x++) {
				std::vector<_Tile>::iterator it;
				if (i < 0)
					it = tiles[y][x].end();
				else
					it = tiles[y][x].begin() + i;
				tiles[y][x].insert(it, t);
			}
		}

		std::vector<bool>::iterator it;
		if (i < 0)
			it = visible.end();
		else
			it = visible.begin() + i;
		visible.insert(it, true);
	}

	void deleteRow(int i) {
		tiles.erase(tiles.begin() + i);
		resizeScrollpane();
		clearUndo();
	}

	void deleteColumn(int i) {
		for (unsigned int y = 0; y < tiles.size(); y++) {
			tiles[y].erase(tiles[y].begin() + i);
		}
		resizeScrollpane();
		clearUndo();
	}

	void insertRow(int i) {
		std::vector< std::vector<_Tile> > row = createRow(tiles[0].size());
		std::vector< std::vector< std::vector<_Tile> > >::iterator it;
		if (i < 0)
			it = tiles.end();
		else
			it = tiles.begin() + i;
		tiles.insert(it, row);
		resizeScrollpane();
		clearUndo();
	}

	void insertColumn(int i) {
		std::vector<_Tile> t = createEmptyTile();
		for (unsigned int y = 0; y < tiles.size(); y++) {
			if (i < 0) {
				tiles[y].insert(tiles[y].end(), t);
			}
			else {
				tiles[y].insert(tiles[y].begin() + i, t);
			}
		}
		resizeScrollpane();
		clearUndo();
	}

	virtual void keyDown(int keycode) {
		bool using_mover = (tool == TOOL_MOVER);
		bool using_raiser = (tool == TOOL_RAISER);
		if (keycode == ALLEGRO_KEY_Z) {
			if ((tgui::isKeyDown(ALLEGRO_KEY_LSHIFT) || tgui::isKeyDown(ALLEGRO_KEY_RSHIFT)) && (tgui::isKeyDown(ALLEGRO_KEY_LCTRL) || tgui::isKeyDown(ALLEGRO_KEY_RCTRL))) {
				doRedo();
			}
			else if (tgui::isKeyDown(ALLEGRO_KEY_LCTRL) || tgui::isKeyDown(ALLEGRO_KEY_RCTRL)) {
				doUndo();
			}
			else {
				tool = TOOL_RAISER;
				already_moved.clear();
			}
		}
		else if (keycode == ALLEGRO_KEY_T) {
			visible[layer] = !visible[layer];
		}
		else if (keycode == ALLEGRO_KEY_R) {
			if (tgui::isKeyDown(ALLEGRO_KEY_LCTRL) || tgui::isKeyDown(ALLEGRO_KEY_RCTRL)) {
				if (statusX < 0 || statusY < 0)
					return;
				if (tgui::isKeyDown(ALLEGRO_KEY_LSHIFT) || tgui::isKeyDown(ALLEGRO_KEY_RSHIFT)) {
					// insert row after cursor
					if (statusY == (int)tiles.size()-1)
						insertRow(-1);
					else
						insertRow(statusY+1);
				}
				else {
					// insert row before cursor
					insertRow(statusY);
				}
			}
			else {
				record();
			}
		}
		else if (keycode == ALLEGRO_KEY_C) {
			if (tgui::isKeyDown(ALLEGRO_KEY_LCTRL) || tgui::isKeyDown(ALLEGRO_KEY_RCTRL)) {
				if (statusX < 0 || statusY < 0)
					return;
				if (tgui::isKeyDown(ALLEGRO_KEY_LSHIFT) || tgui::isKeyDown(ALLEGRO_KEY_RSHIFT)) {
					// insert row after cursor
					if (statusX == (int)tiles[0].size()-1)
						insertColumn(-1);
					else
						insertColumn(statusX+1);
				}
				else {
					// insert row before cursor
					insertColumn(statusX);
				}
			}
			else {
				tool = TOOL_CLEAR;
			}
		}
		else if (keycode == ALLEGRO_KEY_DELETE) {
			if (statusX < 0 || statusY < 0)
				return;
			if (tgui::isKeyDown(ALLEGRO_KEY_LCTRL) || tgui::isKeyDown(ALLEGRO_KEY_RCTRL)) {
				if (tgui::isKeyDown(ALLEGRO_KEY_LSHIFT) || tgui::isKeyDown(ALLEGRO_KEY_RSHIFT)) {
					deleteColumn(statusX);
				}
				else {
					deleteRow(statusY);
				}
			}
		}
		else if (keycode == ALLEGRO_KEY_B) {
			tool = TOOL_BRUSH;
		}
		else if (keycode == ALLEGRO_KEY_S) {
			tool = TOOL_SOLID;
		}
		else if (keycode == ALLEGRO_KEY_M) {
			tool = TOOL_MACRO;
		}
		else if (keycode == ALLEGRO_KEY_F) {
			if (tgui::isKeyDown(ALLEGRO_KEY_LSHIFT) || tgui::isKeyDown(ALLEGRO_KEY_RSHIFT)) {
				tool = TOOL_FILL_ALL;
			}
			else {
				tool = TOOL_FILL_CURRENT;
			}
		}
		else if (keycode == ALLEGRO_KEY_L && General::can_add_and_delete_layers) {
			if (tgui::isKeyDown(ALLEGRO_KEY_LCTRL) || tgui::isKeyDown(ALLEGRO_KEY_RCTRL)) {
				if (tgui::isKeyDown(ALLEGRO_KEY_LSHIFT) || tgui::isKeyDown(ALLEGRO_KEY_RSHIFT)) {
					if (layer == layers-1)
						insertLayer(-1);
					else
						insertLayer(layer+1);
				}
				else if (tgui::isKeyDown(ALLEGRO_KEY_ALT) || tgui::isKeyDown(ALLEGRO_KEY_ALTGR)) {
					deleteLayer(layer);
				}
				else {
					insertLayer(layer);
				}
				layer = 0;
			}
		}
		else if (keycode == ALLEGRO_KEY_K) {
			if (statusX >= 0) {
				tool = TOOL_CLONE;
			}
		}
		else if (keycode == ALLEGRO_KEY_V) {
			tool = TOOL_MOVER;
			mover_src_layer = layer;
			mover_dest_layer = -1;
			already_moved.clear();
		}
		if (using_mover && tool != TOOL_MOVER) {
			already_moved.clear();
		}
		if (using_raiser && tool != TOOL_RAISER) {
			already_moved.clear();
		}
	}

	void setMoverDestLayer(int l) {
		mover_dest_layer = l;
	}

	std::string getTool(void) {
		switch (tool) {
			case TOOL_BRUSH:
				return "Brush";
			case TOOL_CLEAR:
				return "Clear";
			case TOOL_SOLID:
				return "Solid";
			case TOOL_MACRO:
				return "Macro";
			case TOOL_CLONE:
				return "Clone";
			case TOOL_MOVER:
				return "Mover";
			case TOOL_FILL_CURRENT:
				return "Fill Current";
			case TOOL_FILL_ALL:
				return "Fill All";
			case TOOL_RAISER:
				return "Raiser";
		}
		return "?";
	}

	void getCloneStart(int *cx, int *cy)
	{
		*cx = cloneStartX;
		*cy = cloneStartY;
	}

	void getHoverPosition(int *xx, int *yy) {
		if (xx)
			*xx = statusX;
		if (yy)
			*yy = statusY;
	}

	void applyUndo(_TilePlusPlus u, std::vector<_TilePlusPlus> &opposite) {
		_TilePlusPlus t;
		t.x = u.x;
		t.y = u.y;
		t.layer = u.layer;
		t.number = tiles[u.y][u.x][u.layer].number;
		t.sheet = tiles[u.y][u.x][u.layer].sheet;
		t.solid = tiles[u.y][u.x][u.layer].solid;
		opposite.push_back(t);
		tiles[u.y][u.x][u.layer].number = u.number;
		tiles[u.y][u.x][u.layer].sheet = u.sheet;
		tiles[u.y][u.x][u.layer].solid = u.solid;
	}

	void doUndo(void) {
		if (undoes.size() <= 0) return;

		std::vector<_TilePlusPlus> &u = undoes[undoes.size()-1];
		std::vector<_TilePlusPlus> r;

		for (int i = (int)u.size()-1; i >= 0; i--) {
			applyUndo(u[i], r);
		}
		
		redoes.push_back(r);
		if (redoes.size() >= MAX_UNDO)
			redoes.erase(redoes.begin());

		undoes.erase(undoes.end());
	}
	
	void doRedo(void) {
		if (redoes.size() <= 0) return;

		std::vector<_TilePlusPlus> &r = redoes[redoes.size()-1];
		std::vector<_TilePlusPlus> u;

		for (int i = (int)r.size()-1; i >= 0; i--) {
			applyUndo(r[i], u);
		}

		undoes.push_back(u);
		if (undoes.size() >= MAX_UNDO)
			undoes.erase(undoes.begin());

		redoes.erase(redoes.end());
	}
	
	void use_tool(int t, int x, int y, int l, int number, int sheet) {
		bool fill_all = false;
		std::pair<int, int> pr(x, y);

		switch (t) {
			case TOOL_MOVER: {
				if (mover_dest_layer >= -1 && std::find(already_moved.begin(), already_moved.end(), pr) == already_moved.end() && tiles[y][x][mover_src_layer].number != -1 && tiles[y][x][mover_src_layer].sheet != -1) {
					already_moved.push_back(pr);
					tiles[y][x][mover_dest_layer] =
						tiles[y][x][mover_src_layer];
					tiles[y][x][mover_src_layer].number = -1;
					tiles[y][x][mover_src_layer].sheet = -1;
				}
				break;
			}
			case TOOL_CLONE:
			case TOOL_BRUSH:
				tiles[y][x][l].number = number;
				tiles[y][x][l].sheet = sheet;
				break;
			case TOOL_CLEAR:
				tiles[y][x][l].number = -1;
				tiles[y][x][l].sheet = -1;
				break;
			case TOOL_SOLID:
				tiles[y][x][l].solid = !tiles[y][x][l].solid;
				break;
			case TOOL_FILL_ALL:
				fill_all = true;
			case TOOL_FILL_CURRENT: {
				Point p;
				std::stack<Point> stack;
				int tile_num = tiles[y][x][l].number;
				int tile_sheet = tiles[y][x][l].sheet;
	
				p.x = x;
				p.y = y;
				stack.push(p);

				while (stack.size() > 0) {
					p = stack.top();
					stack.pop();

					fill(number, sheet, l, p.x, p.y, tile_num, tile_sheet, stack, fill_all);
				}
				break;
			}
			case TOOL_RAISER: {
				if (std::find(already_moved.begin(), already_moved.end(), pr) == already_moved.end()) {
					already_moved.push_back(pr);

					int i;
					int layers = tiles[0][0].size();
					for (i = layers-1; i >= 0; i--) {
						if (tiles[y][x][i].number != -1) {
							break;
						}
					}
					if (i < 0)
						i = 0;

					int n = tiles[y][x][i].number;
					int s = tiles[y][x][i].sheet;

					tiles[y][x][i].number = -1;
					tiles[y][x][i].sheet = -1;

					tiles[y][x][l].number = n;
					tiles[y][x][l].sheet = s;
				}

				break;
			}
		}
	}

	void doMacro(int xx, int yy) {
		for (unsigned int i = 0; i < macro.size(); i++) {
			_TilePlusPlus t = macro[i];
			t.x += xx;
			t.y += yy;
			use_tool(t.tool, t.x, t.y, t.layer, t.number, t.sheet);
		}
	}

	void placeTile(int x, int y) {
		if (x < 0 || y < 0 || x >= (int)tiles[0].size() || y >= (int)tiles.size())
			return;
		if (tool == TOOL_MOVER) {
			_TilePlusPlus u1, u2;
			u1.x = x;
			u1.y = y;
			u1.layer = mover_src_layer;
			u1.number = tiles[y][x][mover_src_layer].number;
			u1.sheet = tiles[y][x][mover_src_layer].sheet;
			u1.solid = tiles[y][x][mover_src_layer].solid;
			undo.push_back(u1);
			u2.x = x;
			u2.y = y;
			u2.layer = mover_dest_layer;
			u2.number = tiles[y][x][mover_dest_layer].number;
			u2.sheet = tiles[y][x][mover_dest_layer].sheet;
			u2.solid = tiles[y][x][mover_dest_layer].solid;
			undo.push_back(u2);
		}
		else {
			_TilePlusPlus u;
			u.x = x;
			u.y = y;
			u.layer = layer;
			u.number = tiles[y][x][layer].number;
			u.sheet = tiles[y][x][layer].sheet;
			u.solid = tiles[y][x][layer].solid;
			undo.push_back(u);
		}
		_TilePlusPlus mt;
		mt.x = x;
		mt.y = y;
		mt.layer = layer;
		mt.tool = tool;
		if (tool == TOOL_MACRO) {
			doMacro(x, y);
			return;
		}
		use_tool(tool, x, y, layer, number, sheet);
		if (tool == TOOL_CLONE || tool == TOOL_BRUSH) {
			mt.number = number;
			mt.sheet = sheet;
		}
		if (recording) {
			if (macroStartX == -1) {
				macroStartX = x;
				macroStartY = y;
			}
			mt.x -= macroStartX;
			mt.y -= macroStartY;
			macro.push_back(mt);
		}
	}

	void setLayer(int l) {
		if (l != layer)
			already_moved.clear();
		layer = l;
	}

	int getLayer(void) {
		return layer;
	}

	void setTile(int n) {
		number = n;
	}

	void setSheet(int s) {
		sheet = s;
	}

	void getTile(int xx, int yy, int l, int *number, int *sheet, bool *solid, bool *tint) {
		_Tile t = tiles[yy][xx][l];
		if (number)
			*number = t.number;
		if (sheet)
			*sheet = t.sheet;
		if (solid)
			*solid = t.solid;
		std::pair<int, int> pr(xx, yy);
		if ((tool == TOOL_MOVER && l == mover_dest_layer &&
			std::find(already_moved.begin(), already_moved.end(),
			pr) != already_moved.end()) || (tool == TOOL_RAISER && std::find(already_moved.begin(), already_moved.end(), pr) != already_moved.end()))
			*tint = true;
		else
			*tint = false;
	}

	int getWidth(void) {
		return tiles[0].size();
	}

	int getHeight(void) {
		return tiles.size();
	}

	virtual void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y) {
		if (rel_x >= 0) {
			statusX = rel_x / General::tileSize / General::scale;
			statusY = rel_y / General::tileSize / General::scale;
			if (down) {
				if (statusX >= 0 && tool == TOOL_CLONE) {
					int tx = cloneTileX + (statusX - cloneStartX);
					int ty = cloneTileY + (statusY - cloneStartY);
					int tw = al_get_bitmap_width(tileSheets[0]) / General::tileSize;
					int th = al_get_bitmap_height(tileSheets[0]) / General::tileSize;
					tx %= tw;
					ty %= th;
					ts->setSelected(tx, ty);
					number = tx + ty*tw;
				}
				placeTile(statusX, statusY);
			}
			A_Scrollpane *scrollpane = dynamic_cast<A_Scrollpane *>(parent);
			if (scrollpane) {
				if (statusX*General::tileSize >= scrollpane->getSizeX()) {
					statusX = -1;
				}
				if (statusY*General::tileSize >= scrollpane->getSizeY()) {
					statusY = -1;
				}
			}
		}
		else {
			statusX = -1;
			statusY = -1;
		}
	}

	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (mb != 1) return;
      
		if (rel_x >= 0) {
			if (tool == TOOL_CLONE) {
				cloneStartX = statusX;
				cloneStartY = statusY;
				ts->getSelected(&cloneTileX, &cloneTileY);
			}
			int xx = rel_x / General::tileSize / General::scale;
			int yy = rel_y / General::tileSize / General::scale;
			placeTile(xx, yy);
			down = true;
		}
	}

	virtual void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (mb != 1 || !down) return;
      
		down = false;

		if (rel_x >= 0) {
			undoes.push_back(undo);
			if (undoes.size() >= MAX_UNDO) {
				undoes.erase(undoes.begin());
			}
			redoes.clear();
		}
		undo.clear();

		cloneStartX = -1;
	}

	void writeInt(ALLEGRO_FILE *f, int i) {
		al_fputc(f, (i >> 0) & 0xff);
		al_fputc(f, (i >> 8) & 0xff);
		al_fputc(f, (i >> 16) & 0xff);
		al_fputc(f, (i >> 24) & 0xff);
	}

	int readInt(ALLEGRO_FILE *f) {
		unsigned char b1 = al_fgetc(f);
		unsigned char b2 = al_fgetc(f);
		unsigned char b3 = al_fgetc(f);
		unsigned char b4 = al_fgetc(f);
		return b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
	}
	
	void size(int w, int h) {
		tiles.clear();

		clearUndo();

		for (int i = 0; i < h; i++) {
			std::vector< std::vector<_Tile> > row = createRow(w);
			tiles.push_back(row);
		}

		for (int i = 0; i < layers; i++) {
			visible.push_back(true);
		}
	}

	std::string getLoadSavePath(bool load) {
		ALLEGRO_FILECHOOSER *diag;
		diag = al_create_native_file_dialog(
			al_path_cstr(loadSavePath, '/'),
			load ? "Load Area" : "Save Area",
			"*.*",
			load ? ALLEGRO_FILECHOOSER_FILE_MUST_EXIST : ALLEGRO_FILECHOOSER_SAVE
		);

		al_show_native_file_dialog(display, diag);

		if (al_get_native_file_dialog_count(diag) != 1)
			return "";

		const ALLEGRO_PATH *result = al_create_path(al_get_native_file_dialog_path(diag, 0));
		al_destroy_path(loadSavePath);
		loadSavePath = al_clone_path(result);
		al_set_path_filename(loadSavePath, NULL);

		std::string path = al_path_cstr(result, ALLEGRO_NATIVE_PATH_SEP);

		al_destroy_native_file_dialog(diag);

		return path;
	}

	void load(std::string filename) {
		const char *cFilename = filename.c_str();

		ALLEGRO_FILE *f = al_fopen(cFilename, "rb");

		int w = readInt(f);
		int h = readInt(f);

#ifdef MONSTERRPG2
		layers = General::startLayers;

		size(w, h);

		resizeScrollpane();

		int nTileTypes = readInt(f);

		std::vector<int> tileTypes;

		for (int i = 0; i < nTileTypes; i++) {
			tileTypes.push_back(readInt(f));
		}

		for (unsigned int y = 0; y < tiles.size(); y++) {
			for (unsigned int x = 0; x < tiles[0].size(); x++) {
				for (int l = 0; l < General::startLayers; l++) {
					_Tile &t = tiles[y][x][l];
					int n = readInt(f);
					t.number = n < 0 ? -1 : tileTypes[n];
					t.sheet = n < 0 ? -1 : 0;
				}
				bool solid = al_fgetc(f);
				for (int l = 0; l < General::startLayers; l++) {
					_Tile &t = tiles[y][x][l];
					t.solid = solid;
				}
			}
		}
#else
		layers = readInt(f);

		size(w, h);

		resizeScrollpane();
		
		// for each layer
		for (int l = 0; l < layers; l++) {
			// read each tile: tile number and sheet
			for (unsigned int y = 0; y < tiles.size(); y++) {
				for (unsigned int x = 0; x < tiles[0].size(); x++) {
					_Tile &t = tiles[y][x][l];
					t.number = readInt(f);
					t.sheet = al_fgetc(f);
					t.solid = al_fgetc(f);
				}
			}
		}
#endif

		al_fclose(f);
	}

	void setLastSaveName(std::string name)
	{
		lastSaveName = name;
	}

	void save(std::string filename) {
#ifdef MONSTERRPG2
		ALLEGRO_FILE *f = al_fopen(filename.c_str(), "wb");
		
		writeInt(f, tiles[0].size()); // width
		writeInt(f, tiles.size()); // height

		std::vector<int> used_tiles;

		for (int l = 0; l < layers; l++) {
			for (unsigned int y = 0; y < tiles.size(); y++) {
				for (unsigned int x = 0; x < tiles[0].size(); x++) {
					_Tile t = tiles[y][x][l];
					if (t.number < 0) continue;
					if (std::find(used_tiles.begin(), used_tiles.end(), t.number) == used_tiles.end()) {
						used_tiles.push_back(t.number);
					}
				}
			}
		}

		writeInt(f, used_tiles.size());

		for (size_t i = 0; i < used_tiles.size(); i++) {
			writeInt(f, used_tiles[i]);
		}

		for (unsigned int y = 0; y < tiles.size(); y++) {
			for (unsigned int x = 0; x < tiles[0].size(); x++) {
				bool solid = false;
				for (int l = 0; l < General::startLayers; l++) {
					_Tile t = tiles[y][x][l];
					int idx = 0;
					for (; idx < used_tiles.size(); idx++) {
						if (t.number == used_tiles[idx]) {
							break;
						}
					}
					writeInt(f, t.number < 0 ? -1 : idx);
					if (t.solid) {
						solid = true;
					}
				}
				al_fputc(f, solid);
			}
		}

		al_fclose(f);
#else
		const char *cFilename = filename.c_str();

		ALLEGRO_FILE *f = al_fopen(cFilename, "wb");

		writeInt(f, tiles[0].size()); // width
		writeInt(f, tiles.size()); // height
		writeInt(f, layers);

		// for each layer
		for (int l = 0; l < layers; l++) {
			// write each tile: tile number and sheet
			for (unsigned int y = 0; y < tiles.size(); y++) {
				for (unsigned int x = 0; x < tiles[0].size(); x++) {
					_Tile t = tiles[y][x][l];
					writeInt(f, t.number);
					al_fputc(f, t.sheet);
					al_fputc(f, t.solid);
				}
			}
		}

		al_fclose(f);
#endif
	}

	void load(void) {
		std::string path = getLoadSavePath(true);
		if (path == "") return;
		lastSaveName = path;
		ALLEGRO_PATH *tmp = al_create_path(path.c_str());
		operatingFilename = std::string(al_get_path_filename(tmp));
		al_destroy_path(tmp);
		load(path);
	}

	void save(bool choose) {
		std::string name;
		if (!choose) {
			if (lastSaveName == "") {
				choose = true;
			}
			else {
				name = lastSaveName;
			}
		}
		if (choose) {
			name = getLoadSavePath(false);
			if (name == "") {
				return;
			}
		}
		lastSaveName = name;
		ALLEGRO_PATH *tmp = al_create_path(name.c_str());
		operatingFilename = std::string(al_get_path_filename(tmp));
		al_destroy_path(tmp);
		save(name);
	}

	std::string getOperatingFilename(void) {
		return operatingFilename;
	}

	void createNew(void) {
		size(General::areaSize, General::areaSize);
		operatingFilename = "";
	}

	void record(void) {
		if (!recording) {
			macroStartX = macroStartY = -1;
			macro.clear();
		}
		recording = !recording;
	}

	bool isVisible(int l) {
		return visible[l];
	}

   void toggleLayerVisibility(int layer) {
      visible[layer] = !visible[layer];
   }
   
	A_Leveleditor(drawCallback callback, int layers, A_Tileselector *ts) :
		A_Canvas(callback),
		layers(layers),
		down(false),
		layer(0),
		number(-1),
		sheet(-1),
		lastSaveName(""),
		operatingFilename(""),
		statusX(-1),
		statusY(-1),
		tool(0),
		recording(false),
		cloneStartX(-1),
		ts(ts)
	{
		size(General::areaSize, General::areaSize);
		loadSavePath = al_create_path("");
	}

	~A_Leveleditor(void) {
		al_destroy_path(loadSavePath);
	}

protected:
	static const unsigned int MAX_UNDO = 10;

	Point mkpoint(int x, int y) {
		Point p;
		p.x = x;
		p.y = y;
		return p;
	}
	
	void fill(int new_tile, int new_sheet, int layer, int x, int y, int tile_num, int tile_sheet, std::stack<Point> &stack, bool check_all_layers) {
		Point neighbors_unclipped[4];
		neighbors_unclipped[0] = mkpoint(x-1, y);
		neighbors_unclipped[1] = mkpoint(x+1, y);
		neighbors_unclipped[2] = mkpoint(x, y+1);
		neighbors_unclipped[3] = mkpoint(x, y-1);

		std::vector<Point> neighbors;
		for (int i = 0; i < 4; i++) {
			Point p = neighbors_unclipped[i];
			if (p.x >= 0 && p.y >= 0 && p.x < (int)tiles[0].size() && p.y < (int)tiles.size()) {
				neighbors.push_back(p);
			}
		}

        std::vector<bool> spread(neighbors.size());
		//bool spread[neighbors.size()];
	
		if (check_all_layers) {
			for (int i = 0; i < (int)neighbors.size(); i++) {
				Point p = neighbors[i];
				bool go = true;
				for (int l = 0; l < (int)tiles[0][0].size(); l++) {
					_Tile t2 = tiles[p.y][p.x][l]; //layers[l].tiles[p.Y][p.X];
					if (l == layer) {
						if (t2.number != tile_num || t2.sheet != tile_sheet) {
							go = false;
							break;
						}
					}
					else {
						if ((t2.number != -1) && (t2.number != tile_num || t2.sheet != tile_sheet)) {
							go = false;
							break;
						}
					}
				}
				if (go) {
					spread[i] = true;
				}
				else {
					spread[i] = false;
				}
			}
		}
		else {
			for (int i = 0; i < (int)neighbors.size(); i++) {
				Point p = neighbors[i];
				_Tile t2 = tiles[p.y][p.x][layer]; //layers[layer].tiles[p.Y][p.X];
				if (t2.number != tile_num || t2.sheet != tile_sheet) {
					spread[i] = false;
				}
				else {
					spread[i] = true;
				}
			}
		}

		for (int i = 0; i < (int)neighbors.size(); i++) {
			if (spread[i] == false) {
				continue;
			}
			Point p = neighbors[i];
			/*
			if (layers[layer].tiles[p.Y][p.X].tile_num != new_tile ||
					 layers[layer].tiles[p.Y][p.X].tile_sheet != new_sheet) {
				stack.Push(p);
			}
			*/
			if (tiles[p.y][p.x][layer].number != new_tile || tiles[p.y][p.x][layer].sheet != new_sheet) {
				stack.push(p);
			}
		}
	
		_Tile &t = tiles[y][x][layer];
		t.number = new_tile;
		t.sheet = new_sheet;
	}

	int layers;
	bool down;
	int layer;
	int number;
	int sheet;
	std::vector< std::vector< std::vector<_Tile> > > tiles;
	std::vector<_TilePlusPlus> undo;
	std::vector< std::vector<_TilePlusPlus> > undoes;
	std::vector< std::vector<_TilePlusPlus> > redoes;
	ALLEGRO_PATH *loadSavePath;
	std::string lastSaveName;
	std::string operatingFilename;
	int statusX, statusY;
	int tool;
	bool recording;
	int macroStartX, macroStartY;
	std::vector<_TilePlusPlus> macro;
	int cloneStartX, cloneStartY;
	int cloneTileX, cloneTileY;
	A_Tileselector *ts;
	int mover_src_layer, mover_dest_layer;
	std::vector< std::pair<int, int> > already_moved;
	std::vector<bool> visible;
};

class A_Label : public tgui::TGUIWidget {
public:
	virtual void draw(int abs_x, int abs_y) {
		int yy = abs_y;

		for (unsigned int i = 0; i < lines.size(); i++) {
			al_draw_text(
				tgui::getFont(),
				color,
				abs_x, yy,
				0,
				lines[i].c_str()
			);
			yy += al_get_font_line_height(tgui::getFont());
		}
	}

	void setLines(std::string text) {
		lines.clear();
		std::string::size_type currPos = 0;
		std::string::size_type newlinePos;
		while ((newlinePos = text.find("\n", currPos)) != std::string::npos) {
			std::string s = text.substr(currPos, newlinePos-currPos);
			currPos = newlinePos+1;
			lines.push_back(s);
		}
		if (currPos < text.length()) {
			// no newline at end
			std::string s = text.substr(currPos, text.length()-currPos);
			lines.push_back(s);
		}
	}

	void setText(std::string text) {
		setLines(text);
	}

	A_Label(std::string text, ALLEGRO_COLOR color) :
		color(color)
	{
		setLines(text);
		x = y = 0;
	}

protected:
	std::vector<std::string> lines;
	ALLEGRO_COLOR color;
};

#endif // WIDGETS_H
