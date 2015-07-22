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
extern ALLEGRO_EVENT_QUEUE *queue;
extern std::vector<ALLEGRO_BITMAP *> tileSheets;
void setTitle();

struct Point {
	float x, y;
};

class A_Splitter_Resizable : public tgui::TGUIWidget {
public:
	virtual void paneResize(tgui::TGUIWidget *pane, float *xx, float *yy, float *w, float *h) = 0;
};

class A_Splitter_Panel : public tgui::TGUIWidget {
public:
	bool getAbsoluteChildPosition(tgui::TGUIWidget *widget, int *x, int *y)
	{
		if (child == widget) {
			int xx, yy;
			tgui::determineAbsolutePosition(this, &xx, &yy);
			*x = xx;
			*y = yy;
			return true;
		}

		int xx, yy;

		if (child->getAbsoluteChildPosition(widget, x, y)) {
			return true;
		}

		return false;
	}

	virtual void translate(int xx, int yy) {
	}

	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (!child) return;
		child->mouseDown(rel_x, rel_y, abs_x, abs_y, mb);
	}

	virtual void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y) {
		if (!child) return;
		child->mouseMove(rel_x, rel_y, abs_x, abs_y);
	}

	virtual void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (!child) return;
		child->mouseUp(rel_x, rel_y, abs_x, abs_y, mb);
	}

	virtual void draw(int abs_x, int abs_y) {
		int prev_x, prev_y, prev_w, prev_h;
		tgui::getClip(&prev_x, &prev_y, &prev_w, &prev_h);
		tgui::setClip(abs_x, abs_y, width, height);
		if (child) {
			child->draw(abs_x, abs_y);
		}
		tgui::setClip(prev_x, prev_y, prev_w, prev_h);
	}

	virtual void postDraw(int abs_x, int abs_y) {
		int prev_x, prev_y, prev_w, prev_h;
		tgui::getClip(&prev_x, &prev_y, &prev_w, &prev_h);
		tgui::setClip(abs_x, abs_y, width, height);
		if (child) {
			child->postDraw(abs_x, abs_y);
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

	bool getAbsoluteChildPosition(tgui::TGUIWidget *widget, int *x, int *y)
	{
		if (first_pane == widget || second_pane == widget) {
			int own_x, own_y;
			tgui::determineAbsolutePosition(this, &own_x, &own_y);

			int xx, yy;

			if (first_pane == widget) {
				*x = own_x + first_pane->getX();
				*y = own_y + first_pane->getY();
			}
			else {
				if (split_type == SPLIT_HORIZONTAL) {
					xx = 0;
					yy = first_pixels+4;
				}
				else {
					xx = first_pixels+4;
					yy = 0;
				}
				*x = own_x + xx + second_pane->getX();
				*y = own_y + yy + second_pane->getY();
			}

			return true;
		}

		int xx, yy;

		if (first_pane->getAbsoluteChildPosition(widget, x, y)) {
			return true;
		}
		if (second_pane->getAbsoluteChildPosition(widget, x, y)) {
			return true;
		}

		return false;
	}

	virtual void chainKeyDown(int chainKeycode) {
		if (first_pane) {
			first_pane->chainKeyDown(chainKeycode);
		}
		if (second_pane) {
			second_pane->chainKeyDown(chainKeycode);
		}
	}

	virtual void chainKeyUp(int chainKeycode) {
		if (first_pane) {
			first_pane->chainKeyUp(chainKeycode);
		}
		if (second_pane) {
			second_pane->chainKeyUp(chainKeycode);
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
		resize();
	}

	virtual void raise(void) {
		TGUIWidget::raise();
	
		if (first_pane) {
			first_pane->raise();
		}
		if (second_pane) {
			second_pane->raise();
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
		int xx, yy;
		if (first_pane) {
			determineAbsolutePosition(first_pane, &xx, &yy);
			first_pane->draw(xx, yy);
		}
		if (second_pane) {
			determineAbsolutePosition(second_pane, &xx, &yy);
			second_pane->draw(xx, yy);
		}

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

	virtual void chainDraw()
	{
		int abs_x, abs_y;
		determineAbsolutePosition(this, &abs_x, &abs_y);
		draw(abs_x, abs_y);
	}

	virtual void postDraw(int abs_x, int abs_y)
	{
		if (first_pane) {
			first_pane->postDraw(abs_x, abs_y);
		}
		if (second_pane) {
			if (split_type == SPLIT_HORIZONTAL) {
				second_pane->postDraw(abs_x, abs_y+first_pixels+4);
			}
			else {
				second_pane->postDraw(abs_x+first_pixels+4, abs_y);
			}
		}
	}


	void passMouseOn(int type, int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		int rx1, rx2, rx3;
		int ry1, ry2, ry3;
		TGUIWidget *target;

		if (down_on_slider) {
			target = this;
		}
		else {
			if (split_type == SPLIT_HORIZONTAL) {
				if (rel_y < first_pixels) {
					target = first_pane;
				}
				else if (rel_y >= first_pixels+4) {
					target = second_pane;
				}
				else {
					target = this;
				}
			}
			else {
				if (rel_x < first_pixels) {
					target = first_pane;
				}
				else if (rel_x >= first_pixels+4) {
					target = second_pane;
				}
				else {
					target = this;
				}
			}
		}

		if (target == this) {
			rx1 = rel_x;
			ry1 = rel_y;
			rx2 = ry2 = rx3 = ry3 = -1;
		}
		else if (target == first_pane) {
			rx2 = rel_x;
			ry2 = rel_y;
			rx1 = ry1 = rx3 = ry3 = -1;
		}
		else {
			if (split_type == SPLIT_HORIZONTAL) {
				rx3 = rel_x;
				ry3 = rel_y - first_pixels - 4;
			}
			else {
				rx3 = rel_x - first_pixels - 4;
				ry3 = rel_y;
			}
			rx1 = ry1 = rx2 = ry2 = -1;
		}

		switch (type) {
			case MOUSE_MOVE:
				_mouseMove(rx1, ry1, abs_x, abs_y);
				break;
			case MOUSE_DOWN:
				_mouseDown(rx1, ry1, abs_x, abs_y, mb);
				break;
			default:
				_mouseUp(rx1, ry1, abs_x, abs_y, mb);
				break;
		}

		if (type == MOUSE_MOVE) {
			first_pane->mouseMove(rx2, ry2, abs_x, abs_y);
		}
		else if (type == MOUSE_DOWN) {
			first_pane->mouseDown(rx2, ry2, abs_x, abs_y, mb);
		}
		else {
			first_pane->mouseUp(rx2, ry2, abs_x, abs_y, mb);
		}

		if (type == MOUSE_MOVE) {
			second_pane->mouseMove(rx3, ry3, abs_x, abs_y);
		}
		else if (type == MOUSE_DOWN) {
			second_pane->mouseDown(rx3, ry3, abs_x, abs_y, mb);
		}
		else {
			second_pane->mouseUp(rx3, ry3, abs_x, abs_y, mb);
		}
	}

	virtual void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y)
	{
		passMouseOn(MOUSE_MOVE, rel_x, rel_y, abs_x, abs_y, 0);
	}

	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
	{
		passMouseOn(MOUSE_DOWN, rel_x, rel_y, abs_x, abs_y, mb);
	}

	virtual void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
	{
		passMouseOn(MOUSE_UP, rel_x, rel_y, abs_x, abs_y, mb);
	}

	void _mouseMove(int rel_x, int rel_y, int abs_x, int abs_y) {
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

	void _mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
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
			if (abs_y >= my_y && abs_y < my_y+height) {
				on = (abs_x >= first_pixels && abs_x < first_pixels+4);
			}
			else
				on = false;
		}
		if (resizable && on) {
			down_on_slider = true;
			last_slider_pos = (split_type == SPLIT_HORIZONTAL) ? abs_y : abs_x;
		}
	}
	
	void _mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
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
		else {
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
		tgui::setNewWidgetParent(this);
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
			*xx = 0;
			*yy = 0;

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
			*xx = 0;
			*yy = 0;

			if (split_type == SPLIT_HORIZONTAL) {
				//*xx = 0;
				//*yy = first_pixels+4;
				*w = width;
				*h = second_pixels;
			}
			else {
				//*xx = first_pixels+4;
				//*yy = 0;
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

	bool getAbsoluteChildPosition(tgui::TGUIWidget *widget, int *x, int *y)
	{
		if (child == widget) {
			int xx, yy;
			tgui::determineAbsolutePosition(this, &xx, &yy);
			*x = xx;
			*y = yy;
			return true;
		}

		int xx, yy;

		if (child->getAbsoluteChildPosition(widget, x, y)) {
			return true;
		}

		return false;
	}

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
				pushEvent(TGUI_EVENT_OBJECT, (void *)this);
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

		A_Splitter_Panel *panel = (A_Splitter_Panel *)parent;
		A_Splitter *splitter = (A_Splitter *)(panel->splitter);

		int show = getShow();

		if (rel_x >= 0 && !opened) {
			tgui::getTopLevelParent(this)->raise();
			height = (getShow()+2)*HEIGHT;
			splitter->setSplitSize(height, -1);
			tgui::resize(NULL);

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
			splitter->setSplitSize(height, -1);
			tgui::resize(NULL);
		}
		else {
			opened = false;
			hover = -4;
			height = HEIGHT;
			splitter->setSplitSize(height, -1);
			tgui::resize(NULL);
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

			if (values.size()) {
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

			if (values.size()) {
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

	bool getAbsoluteChildPosition(tgui::TGUIWidget *widget, int *x, int *y)
	{
		if (child == widget) {
			int xx, yy;
			tgui::determineAbsolutePosition(this, &xx, &yy);
			*x = xx;
			*y = yy;
			return true;
		}

		int xx, yy;

		if (child->getAbsoluteChildPosition(widget, x, y)) {
			return true;
		}

		return false;
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
		return len;
	}
	
	int getTotalLength(int widgetSize, int barSize) {
		return widgetSize-barSize-THICKNESS-1;
	}

	void draw_self(int abs_x, int abs_y) {
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
		if (p1 < 0) p1 = 0;
		else if (p1 > 1) p1 = 1;
		if (p2 < 0) p2 = 0;
		else if (p2 > 1) p2 = 1;
	}
   
	virtual void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y) {
		if (downOnSide) {
			int barSize = getBarLength(height-THICKNESS, size_y);
			int totalLength = getTotalLength(height, barSize);
			p1 = downP + ((float)(abs_y - downAt) / totalLength);
			if (p1 < 0) p1 = 0;
			else if (p1 > 1) p1 = 1;
		}
		else if (downOnBottom) {
			int barSize = getBarLength(width-THICKNESS, size_x);
			int totalLength = getTotalLength(width, barSize);
			p2 = downP + ((float)(abs_x - downAt) / totalLength);
			if (p2 < 0) p2 = 0;
			else if (p2 > 1) p2 = 1;
		}
		else if (rel_x >= 0 && child) {
			if (tgui::isKeyDown(ALLEGRO_KEY_LCTRL) && down) {
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
			}
			else {
				int cx = rel_x + getOffsetX();
				int cy = rel_y + getOffsetY();
				child->mouseMove(cx, cy, abs_x, abs_y);
			}
		}
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
			else if (rel_y >= bpos) {
				p1 += (float)barSize/totalLength;
				if (p1 > 1) p1 = 1;
			}
			else {
				p1 -= (float)barSize/totalLength;
				if (p1 < 0) p1 = 0;
			}
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
			else if (rel_x >= bpos) {
				p2 += (float)barSize/totalLength;
				if (p2 > 1) p2 = 1;
			}
			else {
				p2 -= (float)barSize/totalLength;
				if (p2 < 0) p2 = 0;
			}
		}
		else if (rel_x >= 0 && child) {
			if (tgui::isKeyDown(ALLEGRO_KEY_LCTRL)) {
				down = true;
				start_x = rel_x;
				start_y = rel_y;
			}
			else {
				int cx = rel_x + getOffsetX();
				int cy = rel_y + getOffsetY();
				child->mouseDown(cx, cy, abs_x, abs_y, mb);
			}
		}
	}

	virtual void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		down = false;
		if (downOnSide || downOnBottom) {
			downOnSide = downOnBottom = false;
		}
		else {
			int cx = rel_x + getOffsetX();
			int cy = rel_y + getOffsetY();
			child->mouseUp(cx, cy, abs_x, abs_y, mb);
		}
	}

	virtual void draw(int abs_x, int abs_y)
	{
		draw_self(abs_x, abs_y);

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
		downOnSide(false),
		down(false)
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
	bool down;
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
			down = true;
			selected_x = rel_x / General::tileSize / General::scale;
			selected_y = rel_y / General::tileSize / General::scale;
			selected_w = 1;
			selected_h = 1;
		}
	}
	virtual void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y) {
		if (down) {
			int x = rel_x / General::tileSize / General::scale;
			int y = rel_y / General::tileSize / General::scale;
			selected_w = x - selected_x;
			selected_h = y - selected_y;
			if (x < selected_x) {
				selected_w--;
			}
			else {
				selected_w++;
			}
			if (y < selected_y) {
				selected_h--;
			}
			else {
				selected_h++;
			}
		}
	}
	virtual void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
	{
		down = false;
	}

	void getSelected(int *x, int *y, int *w, int *h) {
		if (selected_w < 0) {
			if (x)
				*x = selected_x + selected_w + 1;
			if (w)
				*w = -selected_w;
		}
		else {
			if (x)
				*x = selected_x;
			if (w)
				*w = selected_w;\
		}

		if (selected_h < 0) {	
			if (y)
				*y = selected_y + selected_h + 1;
			if (h)
				*h = -selected_h;
		}
		else {
			if (y)
				*y = selected_y;
			if (h)
				*h = selected_h;
		}
	}

	void setSelected(int x, int y, int w, int h)
	{
		selected_x = x;
		selected_y = y;
		selected_w = w;
		selected_h = h;
	}

	A_Tileselector(drawCallback callback) :
		A_Canvas(callback),
		selected_x(0),
		selected_y(0),
		selected_w(1),
		selected_h(1),
		down(false)
	{
	}

protected:
	int selected_x;
	int selected_y;
	int selected_w;
	int selected_h;
	bool down;
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
	static const int TOOL_MARQUEE = 9;

	struct _TilePlusPlus {
		int x, y, layer, number, sheet;
		bool solid;
		int tool;
	};

	struct _Tile {
		int number;
		int sheet;
		bool solid;
		int x, y;
	};

	struct Group {
		int type, layer, x, y, w, h;
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

	std::vector<Group> &getGroups() {
		return groups;
	}

	_Tile createEmpty_Tile(void) {
		_Tile t;
		t.number = -1;
		t.sheet = -1;
		t.solid = false;
		t.x = -1;
		t.y = -1;
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

	void deleteLayer(int i) {
		if (layers < 2) return;

		push_undo();

		for (unsigned int y = 0; y < tiles.size(); y++) {
			for (unsigned int x = 0; x < tiles[y].size(); x++) {
				std::vector<_Tile>::iterator it = tiles[y][x].begin() + i;
				tiles[y][x].erase(it);
			}
		}
		visible.erase(visible.begin() + i);
		layers--;

		for (size_t j = 0; j < groups.size();) {
			Group &g = groups[j];
			if (g.layer == i) {
				groups.erase(groups.begin() + j);
			}
			else {
				if (g.layer > i) {
					g.layer--;
				}
				j++;
			}
		}

		changed = true;
	}

	void insertLayer(int i) {
		push_undo();

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

		if (i >= 0) {
			for (size_t j = 0; j < groups.size(); j++) {
				Group &g = groups[j];
				if (g.layer >= i) {
					g.layer++;
				}
			}
		}

		changed = true;
	}

	void deleteRow(int i) {
		if (tiles.size() <= i) return;
		push_undo();
		tiles.erase(tiles.begin() + i);
		resizeScrollpane();

		for (size_t j = 0; j < groups.size(); j++) {
			Group &g = groups[j];
			if (g.y > i) {
				g.y--;
			}
			else if (i >= g.y && i < g.y+g.h) {
				g.h--;
			}
		}

		changed = true;
	}

	void deleteColumn(int i) {
		if (tiles[0].size() <= i) return;
		push_undo();
		for (unsigned int y = 0; y < tiles.size(); y++) {
			tiles[y].erase(tiles[y].begin() + i);
		}
		resizeScrollpane();

		for (size_t j = 0; j < groups.size(); j++) {
			Group &g = groups[j];
			if (g.x > i) {
				g.x--;
			}
			else if (i >= g.x && i < g.x+g.w) {
				g.w--;
			}
		}

		changed = true;
	}

	void insertRow(int i) {
		push_undo();
		std::vector< std::vector<_Tile> > row = createRow(tiles[0].size());
		std::vector< std::vector< std::vector<_Tile> > >::iterator it;
		if (i < 0)
			it = tiles.end();
		else
			it = tiles.begin() + i;
		tiles.insert(it, row);
		resizeScrollpane();

		for (size_t j = 0; j < groups.size(); j++) {
			Group &g = groups[j];
			if (g.y >= i) {
				g.y++;
			}
			else if (i > g.y && i < g.y+g.h) {
				g.h++;
			}
		}

		changed = true;
	}

	void insertColumn(int i) {
		push_undo();
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

		for (size_t j = 0; j < groups.size(); j++) {
			Group &g = groups[j];
			if (g.x >= i) {
				g.x++;
			}
			else if (i > g.x && i < g.x+g.w) {
				g.w++;
			}
		}

		changed = true;
	}

	virtual void keyDown(int keycode) {
		bool using_mover = (tool == TOOL_MOVER);
		bool using_raiser = (tool == TOOL_RAISER);

		if (marquee_floating && keycode == ALLEGRO_KEY_SPACE) {
			push_undo();
			for (int yy = 0; yy < marquee_buffer.size(); yy++) {
				for (int xx = 0; xx < marquee_buffer[0].size(); xx++) {
					int layer_start;
					int layer_end;
					if (marquee_layer == -1) {
						layer_start = 0;
						layer_end = marquee_buffer[0][0].size();
					}
					else {
						layer_start = marquee_layer;
						layer_end = layer_start+1;
					}
					for (int l = layer_start; l < layer_end; l++) {
						tiles[yy+marquee_float_y][xx+marquee_float_x][l].number = marquee_buffer[yy][xx][marquee_layer == -1 ? l : 0].number;
						tiles[yy+marquee_float_y][xx+marquee_float_x][l].sheet = marquee_buffer[yy][xx][marquee_layer == -1 ? l : 0].sheet;
						tiles[yy+marquee_float_y][xx+marquee_float_x][l].solid = marquee_buffer[yy][xx][marquee_layer == -1 ? l : 0].solid;
						changed = true;
					}
				}
			}
			marquee_marked = false;
		}
		marquee_floating = false;

		if (keycode == ALLEGRO_KEY_S) {
			if (tgui::isKeyDown(ALLEGRO_KEY_LCTRL) || tgui::isKeyDown(ALLEGRO_KEY_RCTRL)) {
				save(false);
				setTitle();
				al_flush_event_queue(queue);
			}
			else {
				tool = TOOL_SOLID;
			}
		}
		else if (keycode == ALLEGRO_KEY_Z) {
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
		else if (keycode == ALLEGRO_KEY_Q) {
			tool = TOOL_MARQUEE;
			marquee_marked = false;
		}
		else if (keycode == ALLEGRO_KEY_M) {
			tool = TOOL_MACRO;
		}
		else if (keycode == ALLEGRO_KEY_COMMA || keycode == ALLEGRO_KEY_FULLSTOP) {
			if (tool != TOOL_MARQUEE || marquee_floating || !marquee_marked)
				return;
			int layer_start;
			int layer_end;
			if (tgui::isKeyDown(ALLEGRO_KEY_LCTRL) || tgui::isKeyDown(ALLEGRO_KEY_RCTRL)) {
				layer_start = 0;
				layer_end = tiles[0][0].size();
			}
			else {
				layer_start = layer;
				layer_end = layer+1;
			}
			marquee_buffer_filled = true;
			marquee_buffer.clear();
			int x1, y1, x2, y2;
			get_marquee(&x1, &y1, &x2, &y2);
			for (int yy = y1; yy < y2; yy++) {
				std::vector< std::vector<_Tile> > r;
				for (int xx = x1; xx < x2; xx++) {
					std::vector<_Tile> g;
					for (int i = layer_start; i < layer_end; i++) {
						_Tile t;
						t.number = tiles[yy][xx][i].number;
						t.x = tiles[yy][xx][i].x;
						t.y = tiles[yy][xx][i].y;
						t.sheet = tiles[yy][xx][i].sheet;
						t.solid = tiles[yy][xx][i].solid;
						changed = true;
						g.push_back(t);
					}
					r.push_back(g);
				}
				marquee_buffer.push_back(r);
			}
			if (keycode == ALLEGRO_KEY_FULLSTOP) {
				// delete stuff from tiles
				push_undo();
				for (int yy = y1; yy < y2; yy++) {
					for (int xx = x1; xx < x2; xx++) {
						for (int i = layer_start; i < layer_end; i++) {
							tiles[yy][xx][i].number = -1;
							tiles[yy][xx][i].x = -1;
							tiles[yy][xx][i].y = -1;
							tiles[yy][xx][i].sheet = -1;
							tiles[yy][xx][i].solid = false;
							changed = true;
						}
					}
				}
			}
		}
		else if (keycode == ALLEGRO_KEY_SLASH) {
			if (marquee_buffer_filled) {
				marquee_layer = marquee_buffer[0][0].size() > 1 ? -1 : layer;
				int offsx = ((A_Scrollpane *)parent)->getOffsetX();
				int offsy = ((A_Scrollpane *)parent)->getOffsetY();
				int spw = ((A_Scrollpane *)parent)->getWidth();
				int sph = ((A_Scrollpane *)parent)->getHeight();
				int rows = marquee_buffer.size();
				int cols = marquee_buffer[0].size();
				int pixw = rows * General::tileSize * General::scale;
				int pixh = cols * General::tileSize * General::scale;
				int topx = (offsx+spw/2) - pixw/2;
				int topy = (offsy+sph/2) - pixh/2;
				marquee_float_x = topx / General::tileSize / General::scale;
				marquee_float_y = topy / General::tileSize / General::scale;
				marquee_floating = true;
				marquee_marked = false;
			}

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
				marquee_buffer_filled = false;
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
		else if (keycode == ALLEGRO_KEY_G) {
			if (is_marquee_marked()) {
				if (tgui::isKeyDown(ALLEGRO_KEY_ALT) || tgui::isKeyDown(ALLEGRO_KEY_ALTGR)) {
					Group g = { group_type, layer, marquee_x1, marquee_y1, marquee_x2 - marquee_x1 + 1, marquee_y2 - marquee_y1 + 1};
					for (size_t i = 0; i < groups.size(); i++) {
						if (groups[i].layer == g.layer && groups[i].x == g.x && groups[i].y == g.y && groups[i].w == g.w && groups[i].h == g.h) {
							groups.erase(groups.begin() + i);

							break;
						}
					}
				}
				else {
					bool found = false;
					Group g = { group_type, layer, marquee_x1, marquee_y1, marquee_x2 - marquee_x1 + 1, marquee_y2 - marquee_y1 + 1};
					for (size_t i = 0; i < groups.size(); i++) {
						if (groups[i].layer == g.layer && groups[i].x == g.x && groups[i].y == g.y && groups[i].w == g.w && groups[i].h == g.h) {
							found = true;
							groups[i].type = group_type;
							break;
						}
					}
					if (!found) {
						groups.push_back(g);
					}
				}
				changed = true;
			}
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
			case TOOL_MARQUEE:
				return "Marquee";
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

	void doUndo(void) {
		if (undoes.size() <= 0) return;

		push_redo();

		std::vector<Lvl>::iterator it = undoes.end() - 1;
		tiles = *it;
		undoes.erase(it);
	}
	
	void doRedo(void) {
		if (redoes.size() <= 0) return;
		
		push_undo();
		tiles = redoes[redoes.size()-1];
		redoes.erase(redoes.end()-1);
	}
	
	void use_tool(int t, int x, int y, int l, int number, int sheet) {
		bool fill_all = false;
		std::pair<int, int> pr(x, y);

		switch (t) {
			case TOOL_MOVER: {
				if (mover_dest_layer >= -1 && std::find(already_moved.begin(), already_moved.end(), pr) == already_moved.end() && tiles[y][x][mover_src_layer].number != -1 && tiles[y][x][mover_src_layer].sheet != -1) {
					already_moved.push_back(pr);
					tiles[y][x][mover_dest_layer].number = tiles[y][x][mover_src_layer].number;
					tiles[y][x][mover_dest_layer].x = tiles[y][x][mover_src_layer].x;
					tiles[y][x][mover_dest_layer].y = tiles[y][x][mover_src_layer].y;
					tiles[y][x][mover_dest_layer].sheet = tiles[y][x][mover_src_layer].sheet;
					tiles[y][x][mover_src_layer].number = -1;
					tiles[y][x][mover_src_layer].sheet = -1;
					changed = true;
				}
				break;
			}
			case TOOL_CLONE:
			case TOOL_BRUSH: {
				tiles[y][x][l].number = number;
				int tw = al_get_bitmap_width(tileSheets[0]) / (General::tileSize*General::scale);
				tiles[y][x][l].x = number % tw;
				tiles[y][x][l].y = number / tw;
				tiles[y][x][l].sheet = sheet;
				changed = true;
				break;
			}
			case TOOL_CLEAR:
				tiles[y][x][l].number = -1;
				tiles[y][x][l].x = -1;
				tiles[y][x][l].y = -1;
				tiles[y][x][l].sheet = -1;
				changed = true;
				break;
			case TOOL_SOLID:
				if ((last_solid_x == -1 && last_solid_y == -1) || (last_solid_x != x || last_solid_y != y)) {
					tiles[y][x][l].solid = !tiles[y][x][l].solid;
					last_solid_x = x;
					last_solid_y = y;
					changed = true;
				}
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

					fill(x, y, l, p.x, p.y, tile_num, tile_sheet, stack, fill_all);
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
					tiles[y][x][i].x = -1;
					tiles[y][x][i].y = -1;
					tiles[y][x][i].sheet = -1;

					tiles[y][x][l].number = n;
					int tw = al_get_bitmap_width(tileSheets[0]) / (General::tileSize*General::scale);
					tiles[y][x][l].x = n % tw;
					tiles[y][x][l].y = n / tw;
					tiles[y][x][l].sheet = s;

					changed = true;
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
		if (x < 0 || y < 0 || x >= (int)tiles[0].size() || y >= (int)tiles.size()) {
			return;
		}
		if (tool == TOOL_MACRO) {
			doMacro(x, y);
			return;
		}
		_TilePlusPlus mt;
		mt.x = x;
		mt.y = y;
		mt.layer = layer;
		mt.tool = tool;
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
		_Tile t;
		if (marquee_floating && (l == marquee_layer || marquee_layer == -1) && (xx >= marquee_float_x && xx < (marquee_float_x+marquee_buffer[0].size()) && yy >= marquee_float_y && yy < (marquee_float_y+marquee_buffer.size()))) {
			t = marquee_buffer[yy-marquee_float_y][xx-marquee_float_x][marquee_layer == -1 ? l : 0];
		}
		else {
			t = tiles[yy][xx][l];
		}
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
			if (down && tool == TOOL_MARQUEE) {
				if (dragging_marquee) {
					int dx = statusX - marquee_drag_x;
					int dy = statusY - marquee_drag_y;
					marquee_drag_x = statusX;
					marquee_drag_y = statusY;
					marquee_float_x += dx;
					marquee_float_y += dy;
				}
				else {
					marquee_x2 = statusX;
					marquee_y2 = statusY;
				}
			}
			else {
				if (down) {
					if (statusX >= 0 && tool == TOOL_CLONE) {
						int tx = cloneTileX + (statusX - cloneStartX);
						int ty = cloneTileY + (statusY - cloneStartY);
						int tw = al_get_bitmap_width(tileSheets[0]) / (General::tileSize*General::scale);
						int th = al_get_bitmap_height(tileSheets[0]) / (General::tileSize*General::scale);
						tx %= tw;
						ty %= th;
						ts->setSelected(tx, ty, 1, 1);
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
		}
		else {
			statusX = -1;
			statusY = -1;
		}
	}

	void push_undo(void) {
		undoes.push_back(tiles);
		if (undoes.size() >= MAX_UNDO) {
			undoes.erase(undoes.begin());
		}
	}

	void push_redo(void) {
		redoes.push_back(tiles);
		if (redoes.size() >= MAX_UNDO) {
			redoes.erase(redoes.begin());
		}
	}

	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (mb != 1) return;

		if (rel_x >= 0) {
			int xx = rel_x / General::tileSize / General::scale;
			int yy = rel_y / General::tileSize / General::scale;

			if (tool == TOOL_MARQUEE) {
				if (marquee_floating) {
					dragging_marquee = true;
					marquee_drag_x = xx;
					marquee_drag_y = yy;
				}
				else {
					marquee_x1 = xx;
					marquee_y1 = yy;
					marquee_x2 = xx;
					marquee_y2 = yy;
				}
				down = true;
				marquee_marked = true;
			}
			else {
				push_undo();

				if (tool == TOOL_CLONE) {
					cloneStartX = statusX;
					cloneStartY = statusY;
					ts->getSelected(&cloneTileX, &cloneTileY, NULL, NULL);
				}

				if (tool == TOOL_SOLID) {
					last_solid_x = -1;
					last_solid_y = -1;
				}

				int sel_x, sel_y, sel_w, sel_h;
				ts->getSelected(&sel_x, &sel_y, &sel_w, &sel_h);

				if (tool == TOOL_BRUSH && (sel_w > 1 || sel_h > 1)) {
					for (int _y = 0; _y < sel_h; _y++) {
						for (int _x = 0; _x < sel_w; _x++) {
							if ((xx+_x) >= tiles[0].size() || (yy+_y) >= tiles.size()) {
								continue;
							}
							int tw = al_get_bitmap_width(tileSheets[0]) / (General::tileSize*General::scale);
							int number = (sel_x+_x) + ((sel_y+_y)*tw);
							use_tool(tool, xx+_x, yy+_y, layer, number, sheet);
						}
					}
					down = false;
				}
				else {
					placeTile(xx, yy);
					down = true;
				}
			}
		}
	}

	virtual void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (mb != 1 || !down) return;
      
		down = false;
		dragging_marquee = false;

		redoes.clear();

		cloneStartX = -1;
	}

	void size(int w, int h) {
		tiles.clear();

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
		groups.clear();

		const char *cFilename = filename.c_str();

		ALLEGRO_FILE *f = al_fopen(cFilename, "rb");

#ifdef MO2
		int w = al_fread32le(f);
		int h = al_fread32le(f);

		layers = General::startLayers;

		size(w, h);

		resizeScrollpane();

		int nTileTypes = al_fread32le(f);

		std::vector<int> tileTypes;

		for (int i = 0; i < nTileTypes; i++) {
			tileTypes.push_back(al_fread32le(f));
		}

		for (unsigned int y = 0; y < tiles.size(); y++) {
			for (unsigned int x = 0; x < tiles[0].size(); x++) {
				for (int l = 0; l < General::startLayers; l++) {
					_Tile &t = tiles[y][x][l];
					int n = al_fread32le(f);
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
#elif defined MO3
		int w = al_fread16le(f);
		int h = al_fread16le(f);

		layers = al_fgetc(f);

		size(w, h);

		resizeScrollpane();
		
		// for each layer
		for (int l = 0; l < layers; l++) {
			// read each tile: tile number and sheet
			for (unsigned int y = 0; y < tiles.size(); y++) {
				for (unsigned int x = 0; x < tiles[0].size(); x++) {
					_Tile &t = tiles[y][x][l];
					t.x = (char)al_fgetc(f);
					t.y = (char)al_fgetc(f);
					if (t.x < 0) {
						t.number = -1;
					}
					else if (tileSheets.size() > 0) {
						int tw = al_get_bitmap_width(tileSheets[0]) / (General::tileSize*General::scale);
						t.number = t.x + t.y * tw;
					}
					else {
						t.number = -777;
					}
					t.sheet = (char)al_fgetc(f);
					t.solid = al_fgetc(f);
				}
			}
		}

		int num_groups = al_fread16le(f);

		for (int i = 0; i < num_groups; i++) {
			int t = al_fgetc(f);
			int l = al_fgetc(f);
			int x = al_fread16le(f);
			int y = al_fread16le(f);
			int w = al_fread16le(f);
			int h = al_fread16le(f);
			Group g = { t, l, x, y, w, h };
			groups.push_back(g);
		}
#else // Crystal Picnic format
		int w = al_fread32le(f);
		int h = al_fread32le(f);

		layers = al_fread32le(f);

		size(w, h);

		resizeScrollpane();
		
		// for each layer
		for (int l = 0; l < layers; l++) {
			// read each tile: tile number and sheet
			for (unsigned int y = 0; y < tiles.size(); y++) {
				for (unsigned int x = 0; x < tiles[0].size(); x++) {
					_Tile &t = tiles[y][x][l];
					t.number = al_fread32le(f);
					t.sheet = al_fgetc(f);
					t.solid = al_fgetc(f);
				}
			}
		}
#endif

		al_fclose(f);

		undoes.clear();
		redoes.clear();
	}

	void setLastSaveName(std::string name)
	{
		lastSaveName = name;
	}

	void save(std::string filename) {
		changed = false;

#ifdef MO2
		ALLEGRO_FILE *f = al_fopen(filename.c_str(), "wb");
		
		al_fwrite32le(f, tiles[0].size()); // width
		al_fwrite32le(f, tiles.size()); // height

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

		al_fwrite32le(f, used_tiles.size());

		for (size_t i = 0; i < used_tiles.size(); i++) {
			al_fwrite32le(f, used_tiles[i]);
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
					al_fwrite32le(f, t.number < 0 ? -1 : idx);
					if (l == 0 && t.solid) {
						solid = true;
					}
				}
				al_fputc(f, solid);
			}
		}

		al_fclose(f);
#elif defined MO3
		const char *cFilename = filename.c_str();

		ALLEGRO_FILE *f = al_fopen(cFilename, "wb");

		al_fwrite16le(f, tiles[0].size()); // width
		al_fwrite16le(f, tiles.size()); // height
		al_fputc(f, layers);

		// for each layer
		for (int l = 0; l < layers; l++) {
			// write each tile: tile number and sheet
			for (unsigned int y = 0; y < tiles.size(); y++) {
				for (unsigned int x = 0; x < tiles[0].size(); x++) {
					_Tile t = tiles[y][x][l];
					int tx, ty;
					if (t.sheet < 0 || t.number < 0) {
						tx = ty = -1;
					}
					else {
						int tw = al_get_bitmap_width(tileSheets[0]) / (General::tileSize*General::scale);
						tx = t.number % tw;
						ty = t.number / tw;
					}
					al_fputc(f, tx);
					al_fputc(f, ty);
					al_fputc(f, t.sheet);
					al_fputc(f, t.solid);
				}
			}
		}

		al_fwrite16le(f, groups.size());

		for (size_t i = 0; i < groups.size(); i++) {
			Group &g = groups[i];
			al_fputc(f, g.type);
			al_fputc(f, g.layer);
			al_fwrite16le(f, g.x);
			al_fwrite16le(f, g.y);
			al_fwrite16le(f, g.w);
			al_fwrite16le(f, g.h);
		}

		al_fclose(f);
#else // Crystal Picnic format
		const char *cFilename = filename.c_str();

		ALLEGRO_FILE *f = al_fopen(cFilename, "wb");

		al_fwrite32le(f, tiles[0].size()); // width
		al_fwrite32le(f, tiles.size()); // height
		al_fwrite32le(f, layers);

		// for each layer
		for (int l = 0; l < layers; l++) {
			// write each tile: tile number and sheet
			for (unsigned int y = 0; y < tiles.size(); y++) {
				for (unsigned int x = 0; x < tiles[0].size(); x++) {
					_Tile t = tiles[y][x][l];
					al_fwrite32le(f, t.number);
					al_fputc(f, t.sheet);
					al_fputc(f, t.solid);
				}
			}
		}

		al_fclose(f);
#endif

		al_show_native_message_box(
			display,
			"Saved",
			"",
			"The level has been saved.",
			"OK",
			0
		);
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
		undoes.clear();
		redoes.clear();
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

	void get_marquee(int *x1, int *y1, int *x2, int *y2) {
		if (x1)
			*x1 = MAX(0, MIN(marquee_x1, marquee_x2));
		if (y1)
			*y1 = MAX(0, MIN(marquee_y1, marquee_y2));
		if (x2)
			*x2 = MIN(tiles[0].size(), MAX(marquee_x1, marquee_x2)+1);
		if (y2)
			*y2 = MIN(tiles.size(), MAX(marquee_y1, marquee_y2)+1);
	}

	std::vector< std::vector< std::vector<_Tile> > > get_marquee_buffer() {
		return marquee_buffer;
	}

	bool is_marquee_floating() {
		return marquee_floating;
	}

	void get_marquee_float_xy(int *x, int *y)
	{
		if (x)
			*x = marquee_float_x;
		if (y)
			*y = marquee_float_y;
	}

	int get_marquee_layer() {
		return marquee_layer;
	}

	bool is_marquee_buffer_filled()
	{
		return marquee_buffer_filled;
	}

	bool is_marquee_marked()
	{
		return marquee_marked;
	}

	void toggleLayerVisibility(int layer) {
		visible[layer] = !visible[layer];
	}

	int getCurrentLayer() {
		return layer;
	}

	bool getChanged() {
		return changed;
	}

	void tilesLoaded() {
#ifdef MO3
		if (tileSheets.size() == 0) {
			return;
		}
		int tw = al_get_bitmap_width(tileSheets[0]) / (General::tileSize*General::scale);
		for (int l = 0; l < layers; l++) {
			for (unsigned int y = 0; y < tiles.size(); y++) {
				for (unsigned int x = 0; x < tiles[0].size(); x++) {
					tiles[y][x][l].number = tiles[y][x][l].x + tiles[y][x][l].y * tw;
				}
			}
		}
#endif
	}

	void set_group_type(int type) {
		group_type = type;
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
		ts(ts),
		marquee_marked(false),
		marquee_buffer_filled(false),
		marquee_floating(false),
		dragging_marquee(false),
		changed(false),
		group_type(0)
	{
		size(General::areaSize, General::areaSize);
		loadSavePath = al_create_path("");
	}

	~A_Leveleditor(void) {
		al_destroy_path(loadSavePath);
	}

protected:
	static const unsigned int MAX_UNDO = 100;

	Point mkpoint(int x, int y) {
		Point p;
		p.x = x;
		p.y = y;
		return p;
	}
	
	void fill(int firstx, int firsty, int layer, int x, int y, int tile_num, int tile_sheet, std::stack<Point> &stack, bool check_all_layers) {
		int sel_x, sel_y, sel_w, sel_h;
		ts->getSelected(&sel_x, &sel_y, &sel_w, &sel_h);
		int dx = x - firstx;
		int dy = y - firsty;
		dx %= sel_w;
		dy %= sel_h;
		if (dx < 0) dx = sel_w + dx;
		if (dy < 0) dy = sel_h + dy;

		int tw = al_get_bitmap_width(tileSheets[0]) / (General::tileSize*General::scale);
		int new_tile = (sel_x+dx) + ((sel_y+dy)*tw);
		int new_sheet = sheet;

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
			if (tiles[p.y][p.x][layer].number != new_tile || tiles[p.y][p.x][layer].sheet != new_sheet) {
				stack.push(p);
			}
		}
		
		_Tile &t = tiles[y][x][layer];
		t.number = new_tile;
		t.x = new_tile % tw;
		t.y = new_tile / tw;
		t.sheet = new_sheet;

		changed = true;
	}

	int layers;
	bool down;
	int layer;
	int number;
	int sheet;

	typedef std::vector< std::vector< std::vector<_Tile> > > Lvl;
	Lvl tiles;
	std::vector<Lvl> undoes;
	std::vector<Lvl> redoes;
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

	int marquee_x1;
	int marquee_y1;
	int marquee_x2;
	int marquee_y2;
	bool marquee_marked;
	bool marquee_buffer_filled;
	std::vector< std::vector< std::vector<_Tile> > > marquee_buffer;
	int marquee_layer;
	bool marquee_floating;
	int marquee_float_x;
	int marquee_float_y;
	bool dragging_marquee;
	int marquee_drag_x;
	int marquee_drag_y;
	int last_solid_x;
	int last_solid_y;

	bool changed;

	std::vector<Group> groups;
	int group_type;
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
