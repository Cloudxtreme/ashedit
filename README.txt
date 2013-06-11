Building
========

Set CMake variables USER_INCLUDE_PATH and USER_LIBRARY_PATH
as necessary (for Allegro etc.)

mkdir build
cd build
cmake ..
make
cp ../data/* .
<run ashedit2>

See FORMAT.txt for the file format of levels. Tile sheets are named tiles0.png,
tiles1.png, etc. For Monster RPG 2, only one tile sheet is used.


Using
=====

FIXME: This is incomplete.

- Using Copy and Paste
	1) Press 'q' for the marquee tool
	2) Drag to create a rectangular selection
	3) Press COMMA to copy, PERIOD to cut
	   Hold control while doing to to copy/cut all layers
	4) Press SLASH to paste
	5) Drag selection around with mouse
	6) Press SPACE to anchor floating selection

- Using Clone
	1) Hilight a corner of the section of the tilemap you want to clone
	2) Press 'k'
	3) Press and hold mouse in editor where you want the first tile to appear
	4) Drag in the direction of the next tile in the tilemap
	5) Repeat step 4 until you've clone all tiles you want

- Using Layer Move
	1) Select the source layer
	2) Press 'v'
	3) Select the destination layer
	4) Click/paint over the tiles you want to move.
	   Clicked tiles in source layer are moved to
	   destination layer.

