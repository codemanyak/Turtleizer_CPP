# Turtleizer_CPP
A Turtle graphic library in C++, functionally compatible with the [Turtleizer](https://help.structorizer.fisch.lu/index.php?menu=93) module of [Structorizer](https://github.com/fesch/Structorizer.Desktop).

This little project is just a makeshift static library based on gdiplus and was initiated to get turtle algorithms created in [Structorizer](https://structorizer.fisch.lu) and exported to C++ working with an equivalent behaviour. The source relies on WinAPI functions and Windows classes.

## How to make use of Turtleizer_CPP in an application project
There are several ways to come to terms with linking your application code against this library:
1. Just copy the source and header files into the project where the Structorizer export was directed to (i.e. rather than linking Turtleizer_CPP as library you may of course integrate the few sources into your application project, but then make sure the linker will be allowed to access to the gdiplus.lib library).
2. Put this project into your VisualStudio solution folder next to the projects with exported turtleizer programs (you might want to simplify the generated project folder name from "Turleizer_CPP-master" to just "Turtleizer"), register the project as existing project with your VisualStudio solution and make sure to update the project to your VisualStudio version. Then establish a reference (link) to the Turtleizer project in all projects needing it, configure "..\Turtleizer" (or how the project folder may be named) as additional include directory in these projects.
3. Compile this project separately as static library, and then (similar to the approach before) configure the Turtleizer.lib as additional linker input as well as the folder where Turtleizer.h and Turtle.h reside as additional include folder for your project.

It may be difficult to work with the original project file (in cases 2 and 3), in particular if you happen to use an older VisualStudio version. In this case simply set up a new static library project, insert the header and source files and get it compiled. With up-to-date VisualStudio versions, in contrast, you may have to update the project or at least to adapt the referred SDK version in the project preferences.
When you just import the sources (case 1 above or if you set up a new static library project around the Turtleizer_CPP sources) you must make sure to add gdiplus.lib to the dependencies of the linker and check that the folder where gdiplus.lib resides is registered among the searched library directories)

**It is important that the character encoding be set to "UTF-8" (Unicode). Otherwise some trouble with string type conversion will arise. It may also be necessary to adapt the platform of the using project to Win32 (x86) if Turtleizer_CPP can't be    switched to x64.**

## Turtleizer_CPP API
As mentioned above, Turtleizer_CPP implements all Turtleizer procedures and functions of Structorizer as global functions:

| Signature | Explanation |
| --- | --- |
| `void forward(double pixel)` | Make the turtle move some pixels forward, drawing a line segment if pen is down. |
| `void fd(int pixel)` | As before but with an integer coordinate model (see notes below). |
| `void forward(double pixel, Turtleizer::TurtleColour col)` | Draws the line segment with the given colour. |
| `void fd(int pixel, Turteizer::TurtleColour col)` | As before but with an integer coordinate model (see notes below). |
| `void backward(double pixel)` | Make the turtle move some pixels backward, drawing a line segment if pen is down. |
| `void bk(int pixel)` | As before but with an integer coordinate model (see notes below). |
| `void backward(double pixel, Turtleizer::TurtleColour col)` | Draws the line segment with the given colour. |
| `void bk(int pixel, Turteizer::TurtleColour col)` | As before but with an integer coordinate model (see notes below). |
| `void right(double degrees)` | Rotates the turtle to the right by the angle given in degrees. |
| `void rr(double degrees)` | As before. |
| `void left(double degrees)` | Rotates the turtle to the left by the angle given in degrees. |
| `void rl(double degrees)` | As before. |
| `void gotoXY(int X, int Y)` | Sets the turtle to the position (X,Y) - without drawing! |
| `void gotoX(int X)` | Sets the X coordintae of the turtle's position to the new value - without drawing! |
| `void gotoX(int Y)` | Sets the Y coordintae of the turtle's position to the new value - without drawing! |
| `void penUp()` | Symbolically lifts the pen from the canvas, such that subsequent moves won't draw. |
| `void penDown()` | Symbolically lowers the pen to the canvas, such that subsequent moves will draw. |
| `void hideTurtle()` | Hides the turtle image (lines may still be drawn). |
| `void showTurtle()` |	Show the turtle image again. |
| `void setPenColor(int red, int green, int blue)` | Set the default pen colour to the given RGB value (range 0...255 per argument). |
| `void setBackgroud(int red, int green, int blue)` | Set the background colour to the given RGB value (range 0...255 per argument). |
| `void clear()` | Wipes the canvas from all traces of the turtle. |
| `double getX()` | Returns the current horizontal position (may be between pixels). |
| `double getY()` | Returns the current vertical position (may be beztween pixels). |
| `double getOrientation()` | Returns the current turtle orientiation in degrees (range -180..+180). |

The codes for the standard set of ten predefined colours for the `forward()`, `backward()` etc. moves are defined as follows:
```c++
	enum Turtleizer::TurtleColour {
		TC_BLACK, TC_RED, TC_YELLOW, TC_GREEN,
		TC_CYAN, TC_BLUE, TC_MAGENTA, TC_GREY, TC_ORANGE, TC_VIOLET
	};
```

(Integral coordinate model (as used by the `fd` and `bk` routines) means that end positions of moves are rounded to the next pixels. This seems logical as the screen hasn't pixel fractions but leads to accumulating biases on sequences of traversal moves.)

The only additional instruction that should be inserted in your C++ main function (e.g. exported from [Structorizer](https://structorizer.fisch.lu)) is
```c++
  Turtleizer::awaitClose(); // Put this at the end of the main function
```

The only purpose of `Turtleizer::awaitClose()` is to let the main thread wait for someone closing the Turtleizer window, which will automatically open with the first called Turtleizer instruction.

## Additional "turtles"
In addition to the standard Turtleizer functionality of [Structorizer](https://structorizer.fisch.lu) this library offers to add further "turtles" to the canvas.

To do this, you need the Turtleizer singleton instance first. Use method `Turtleizer::getInstance()` to obtain a pointer to it. With this instance you may create further turtles by means of method
`Turtle* addNewTurtle(int x, int y, LPCWSTR imagePath = NULL);`
You simply specify the start position via arguments `x` and `y` and provide the file path to an image file (recommended: PNG format). The resulting pointer references the new "turtle" instance. (You may sensibly derive a shared_ptr or unique_ptr from it since the `Turtle` instance is dynamically created.)

You may create as many `Turtle` instances as you like (performance may become a limiting factor, of course).
Now you can apply Turtleizer commands as method calls to these specific "turtle" instances independently. Example:

```c++
Turtle* pMyCar = Turtleizer::getInstance()->addNewTurtle(200, 150, L"C:\\Users\\Public\\Pictures\\redCar.png");
unique_ptr<Turtle> upMyCar(pMyCar);
upMyCar->right(45);  // Turns the red car by 45 °
upMyCar->forward(100); // Moves the red car
forward(75);  // This moves the standard turtle independently
```

With this respect, there is an enhanced standard procedure:
`void clear(bool allTurtles = false);`
This wipes the traces of the turtles, including the additional ones if the argument is true, from the canvas. (With the argument being false or omitted, only the standard turtle traces will be cleared.)

## GUI functions
Since version 11.0.0, the Turtleizer window offers enhanced GUI functionality in analogy to [Structorizer](https://structorizer.fisch.lu) versions ≥ 3.31. They comprise scrollbars, zooming support, mouse measuring, a status bar, a tooltip, and a context menu.

![grafik](https://user-images.githubusercontent.com/15326471/115716180-34f87300-a379-11eb-8456-f1ae80ef8d32.png)

### Status bar (all coordinate values in turtle units)
The status bar information consists of (from left to right):
- Home position (x, y) of the standard turtle;
- Current position (x, y) and orientation (in degrees from North) of the standard turtle;
- Extension (maxX x maxY) of (the reachable part of) the drawings of all turtles (the reachable part is all that is positioned beneath and right of the top-left window pixel);
- Current coordinate ranges of the visible scroll area (xLeft .. xRight : yTop .. yBottom);
- The current zoom factor in percent;
- The snap mode (either "+ → /" on snapping to lines or "+ → ▪" on snapping to points only).

### Zooming support
- Numpad-`+` or Ctrl-Numpad-`+` zoom in;
- Numpad-`-` or Ctrl-Numpad-`-` zoom out;
- `1` resets the zoom factor to 100 %;
- `Z` zooms to the united drawing bounds of all turtles.

### Measuring support
- On moving the mouse over the turtle canvas, the current coordinates (in turtle units) are displayed in a tooltip (unless disabled);
- `C` switches the coordinate tooltip on or off;
- On dragging the mouse with left key pressed down, a dashed measuring line from the starting point will follow the mouse and a tooltip continuously shows the length, the coordinate differences (deltaX, deltaY), and the orientation of the line between starting and current point (in turtle units);
- According to the snap configuration the end point of the measuring line will snap to (i.e. caught by) the nearest point on or at the start or end of a line within the snap radius;
- `L` toggles the snap mode between nearest point on a line (+ → /) and nearest start/end/bend point (+ → ▪);
- `R` allows to adapt the snapping radius (default is 5 turtle units).

### Context menu (and accelerator keys)
The context menu offers several navigation (scolling), zooming, visibility, and export functions or options:
- Scrolling
  - `G`:  **Scroll to coordinate** ... → allows to input a turtle coordinate and scrolls to that position (or the nearest position within he canvas range if outside);
  - `End`:  **Scroll to turtle position** → scrolls to the current position of the standard turtle;
  - `Pos1`: **Scroll to home position** → scrolls to the initial (home) position of the standard turtle;
  - `0`:  **Scroll to origin (0,0)** → scrolls to the turtle coordinate origin, i.e. coordinate (0,0);
- Zooming
  - `1`:  **Reset zoom to 100%** → Resets the zoom factor such that a turtle unit equals one screen pixel again;
  - `Z`:  **Zoom to the bounds** → Zooms out (or in) such that the entire reachable drawing fills the Turtleizer canvas (unless zoom factor limits would be exceded);
- Visibility
  - `A`:  **Make all drawing visible** → Transforms the drawing such that parts of the drawing with negative turtle coordinates get visible, i.e. become reachable for zooming and scrolling;
  - `O`:  **Show axes of coordinates** → If enabled, a cross of dashed lines will be drawn through the turtle coordinate origin (only visible if drawing parts in negative quadrants were made visible, see `A` above);
  - `T`:  **Show turtle** → Toggles the visibility of the standard turtle (corresponds to turtle commands `hideTurtle()` and `showTurtle()`, respectively);
  - `B`:  **Set background colour ...** → Opens a colour dialog allowing to change the turtle canvas background (corresponds to turtle command `setBackground(r,g,b)`);
  - `S`:  **Show statusbar** → Shows/hides the statusbar (to hide it may accelerate drawing and enlarges the scroll viewport);
- Measuring
  - `C`:  **Pop up coordinates** → Enables or disables the tooltip that displays the turtle coordnate at the current mouse position (while dragging, the measuring tooltip will be shown no matter whether this option is on or off);
  - `L`:  **Snap to lines (else: points only)** → Toggles between the two snapping modes (either to any point along the nearest line or to start and end points only);
  - `R`:  **Set measuring snap radius** → Opens an input dialog with spinner to modify the snapping radius for measuring;
- Graphics export
  - `X`:  **Export drawing items as CSV ...** → Saves the triples of start point, end point, and colour for all drawn lines of all turtles into a comma-separated values files (the column separator can be chosen);
  - Ctrl-`S`: **Export drawing as PNG ...** → Saves the drawing as PNG file;
  - `V`:  **Export drawing as SVG ...** → Saves the drawing as SVG vecor graphics file.

## License remarks
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.
