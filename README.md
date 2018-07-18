# Turtleizer_CPP
A Turtle graphic library in C++, functionally compatible with the Turtleizer module of Structorizer

This little project is just a makeshift static library based on gdiplus and was initiated to get turtle algorithms created in Structorizer (http://structorizer.fisch.lu) and exported to C++ working with an equivalent behaviour. The source relies on WinAPI functions and Windows classes.

There are several ways to come to terms with it:
1. Just copy the source and header files into the project were the Structorizer export is worked on.
2. Put this project into your VisualStudio solution folder next to the projects with exported turtleizer programs, establish a reference (link) to the Turtleizer project in all projects needing it, configure "..\Turtleizer" as additional include directory in these projects.
3. Compile this project as static library, and then again configure the Turtleizer.lib as additional linker input as well as the folder where Turleizer.h and Turtle.h reside as additional include folder.

It may be diffcult to work with the original project file. In this case simply set up a new static library project, insert the header and source files and get it compiled.

It is important that the character encoding be set to "UTF-8". Otherwise some trouble with string type conversion will arise.

The only additional instruction that should be inserted in your C++ main function (e.g. exported from Structrizer) is
```c++
  Turtleizer::awaitClose(); // Put this at the end of the main function
```

The only purpose of `Turtleizer::awaitClose()` is to let the main thread wait for someone closing the Turtleizer window, which automatically opens with the first called Turtleizer instruction.

In addition to the standard Turtleizer functionality of Structorizer (http://structorizer.fisch.lu) this library offers to add further "turtles" to the canvas.

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

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.
