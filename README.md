# Turtleizer_CPP
A Turtle graphic library in C++, functionally compatible with the Turtleizer module of [Structorizer](https://github.com/fesch/Structorizer.Desktop).

This little project is just a makeshift static library based on gdiplus and was initiated to get turtle algorithms created in [Structorizer](https://structorizer.fisch.lu) and exported to C++ working with an equivalent behaviour. The source relies on WinAPI functions and Windows classes.

## How to make use of Turtleizer_CPP in an application project
There are several ways to come to terms with linking your application code against this library:
1. Just copy the source and header files into the project where the Structorizer export was directed to (i.e. rather than linking Turtleizer_CPP as library you may of course integrate the few sources into your application project).
2. Put this project into your VisualStudio solution folder next to the projects with exported turtleizer programs and make sure to update the project to your VisualStudio version. Then establish a reference (link) to the Turtleizer project in all projects needing it, configure "..\Turtleizer" as additional include directory in these projects.
3. Compile this project separately as static library, and then (similar to the approach before) configure the Turtleizer.lib as additional linker input as well as the folder where Turtleizer.h and Turtle.h reside as additional include folder for your project.

It may be difficult to work with the original project file (in cases 2 and 3), in particular if you happen to use an older VisualStudio version. In this case simply set up a new static library project, insert the header and source files and get it compiled. With up-to-date VisualStudio versions, in contrast, you may have to update the project.
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

The only purpose of `Turtleizer::awaitClose()` is to let the main thread wait for someone closing the Turtleizer window, which automatically opens with the first called Turtleizer instruction.

## Additional "turtles"
In addition to the standard Turtleizer functionality of [Structorizer](https://structorizer.fisch.lu) this library offers to add further "turtles" to the canvas.

To do this, you need the Turtleizer singleton instance first. Use method `Turtleizer::getInstance()` to obtain a pointer to it. With this instance you may create further turtles by means of method
`Turtle* addNewTurtle(int x, int y, LPCWSTR imagePath = NULL);`
You simply specify the start position via arguments `x` and `y` and provide the file path to an image file (recommended: PNG format). The resulting pointer references the new "turtle" instance. (You may sensibly derive a shared_ptr or unique_ptr from it since the `Turtle` instance is dynamically created.)

You may create as many `Turtle` instances as you like (performance may become a limiting factor, of course).
Now you can apply Turtleizer commands as method calls to these specific "turtle" instances  independently. Example:

```c++
Turtle* pMyCar = Turtleizer::getInstance()->addNewTurtle(200, 150, L"C:\\Users\\Public\\Pictures\\redCar.png");
unique_ptr<Turtle> upMyCar(pMyCar);
upMyCar->right(45);  // Turns the red car by 45 °
upMyCar->forward(100); // Moves the red car
forward(75);  // This moves the standard turtle independently
```

With this respect, there is an enhanced standard procedure:
`void clear(bool allTurtles = false);`
This wipes the canvas from the traces of all turtles, including the additional ones, if the argument is true.

## License remarks
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.
