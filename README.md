Introduction
============

AshEdit is a tile-based level editor used for Crystal Picnic and Monster RPG 2. It was originally written for an unreleased game called Ashes Fall, hence the name.

The levels it produces are in a very simple binary format that is easy to load. See FORMAT.txt.


Building
========

AshEdit requires tgui2 to build. See http://github.com/Nooskewl/tgui2. Allegro 5.1 is also needed.

Set CMake variables USER_INCLUDE_PATH and USER_LIBRARY_PATH to where you've put Allegro and TGUI2 if necessary.

	mkdir build
	cd build
	cmake ..
	msbuild /p:Configuration=Release AshEdit.sln # or use make
	<run AshEdit.exe>

On Windows it loads arial.ttf from C:\Windows\Fonts. On Linux, it looks for font.ttf in the CWD.


Using
=====

Tile sheets are named tiles0.png, tiles1.png, etc. (or .tga if you ask CMake to build that way...) For Monster RPG 2 builds, only one tile sheet is used.

FIXME: This is incomplete.

	Using Copy and Paste
		1) Press 'q' for the marquee tool
		2) Drag to create a rectangular selection
		3) Press COMMA to copy, PERIOD to cut
		   Hold control while doing to to copy/cut all layers
		4) Press SLASH to paste
		5) Drag selection around with mouse
		6) Press SPACE to anchor floating selection

	Using Clone
		1) Hilight a corner of the section of the tilemap you want to clone
		2) Press 'k'
		3) Press and hold mouse in editor where you want the first tile to appear
		4) Drag in the direction of the next tile in the tilemap
		5) Repeat step 4 until you've clone all tiles you want

	Using Layer Move
		1) Select the source layer
		2) Press 'v'
		3) Select the destination layer
		4) Click/paint over the tiles you want to move.
		   Clicked tiles in source layer are moved to
		   destination layer.

See the built-in help for keyboard shortcuts.