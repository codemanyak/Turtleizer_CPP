# Turtleizer_CPP
A Turtle graphic library in C++, functionally compatible with the Turtleizer module of Structorizer

This little project is just a makeshift static library based on gdiplus and was initiated to get turtle algorithms created in Structorizer (http://structorizer.fisch.lu) and exported to C++ working with an equivalent behaviour. The source relies on WinAPI functions and Windows classes.

There are several ways to come to terms with it:
1. Just copy the source and header files into the project were the Structorizer export is worked on.
2. Put this project into your solution folder next to the projects with exported turtleizer programs, establish a reference (link) to the Turtleizer project, configure "..\Turtleizer" as additional include directory.
3. Compile this project as static library, and then configure the Turtleizer.lib as additional linker input as well as the folder wheer Turleizer.h and Turtle.h reside as additional include folder again.

It may be diffcult to work with the original project file. In this case simply set up a new static librarry project, insert the header and source files and get it compiled.

It is important that the character encoding be set to "UTF-8". Otherwise some trouble with string type conversion will arise.

The only additional instructions that must be inserted in your C++ main function (e.g. exported from Structrizer) are

  Turtleizer::startUp();  // Put this at the beginning of your main program

  Turtleizer::awaitClose(); // Put this at the end of the main function

With the next version, the startUp() won't be necessary anymore, it is planned to do it automatically on the first call of any Turtleizer function.
The only purpose of Turtleizer::awaitClose() is to let the main hread wait for someone closing the Turtkeizer window, which opens with Turtleizer::StartUp().

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.
