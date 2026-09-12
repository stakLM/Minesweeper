# Minesweeper
Recreation of the famous game Minesweeper in C++ using OpenGL for graphics rendering.

Left-click: reveal tile

Right-click: flag tile

Note:


My very first game made with c++ and opengl. It isn't great; it's fairly small and lacks some features from the actual game. In the future, maybe, my programs will be bigger.

Other Notes:

I did not include a way for the display to show negative numbers, so it does stop at zero.
# Building exe
GNU compiler

Enter the command:
```g++ -I ./include -L ./lib src/minesweeper.cpp src/glad.c -lglfw3dll -o minesweeper ```

or download the <a href = "/minesweeper.exe" download>executable</a>
