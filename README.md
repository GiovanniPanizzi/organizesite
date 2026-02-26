# organizesite
A command line tool to organize files in static websites.

## 1. Requirements analysis
The tool idea is to help organize styles in a static HTML/CSS website. During development, it's common to add styles directly in the HTML or in a disorganized way, which makes it hard to understand which rules affect a given element. With this tool, you can write styles wherever you prefer, and it will automatically separate them in order, grouping them by main sections and following the DOM structure, so it's always clear what affects each element.

It also separates resources and files into folders, using a standard structure.

## 2. Tech stack
* **Main Language:** C++
* **Compilation tool:** CMake 
* **HTML Parser:** gumbo-parser
