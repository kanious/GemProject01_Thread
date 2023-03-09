1. How to build the project

- Please set "GemThreadProject" project as a starter project and build with x64 configuration / Debug or Release mode. Or you can execute with "GemThreadProject.exe" file in x64\Debug(or Release) folder.

- GitHub Link:
https://github.com/kanious/GemProject01_Thread

- Video Link:
https://youtu.be/fMN2q1JnsAw



2. User Input Option

* WASD : Move Camera (look/right direction)
* Mouse Wheel : Change Camera Zoom Level
* ESC : Close the program



3. Description

1) The map size is 1001x1001.

2) The map is so large that it was not easy for 100 beholders to meet each other.
So I restricted the beholder spawn area. Beholders are created only within a 300x300 rectangular area based on the middle tile.

3) Tiles are displayed only when they are within 20 squares of the tile where the camera is looking. (total rendering tiles are 40x40 = 1600 squares)

4) Rolling the mouse wheel changes the zoom level. If you make the zoom level max, you can see the tiles and beholders being culled.


