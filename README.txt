Building:

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

