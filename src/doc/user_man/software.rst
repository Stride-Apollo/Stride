Software
========

System Requirements
-------------------

Stride is written in C++ and portable over all platforms that have the
GNU C++ compiler. De software has no dependencies on external libraries.
The following tools needs to be installed:

  * g++

  * make

  * CMake

  * Boost

  * Python (optional, for automatization)

  * Doxygen (optional, for documentation)

  * LaTeX (optional, for documentation)

  * Sphinx (optional, for documentation)

  * Nodejs, npm, electron and electron-packager (optional, for visualization)

Installation
------------

To install the project, first obtain the source code by cloning the
repository to a directory or download a zip file with all project
material from the Bitbucket website and de-compress the archive. The
build system for Stride uses the CMake tool. This is used to build and
install the software at a high level of abstraction and almost platform
independent (see http://www.cmake.org/). The project includes the
conventional make targets to “build”, “install”, “test” and “clean” the
project. There is one additional target “configure” to set up the
CMake/make structure that will actually do all the work. For those users
that do not have a working knowledge of CMake, a front end Makefile has
been provided that invokes the appropriate CMake commands. More details
on building the software can be found in “INSTALL.txt” in the source
folder.

Documentation
-------------

The Application Programmer Interface (API) documentation is generated
automatically using the Doxygen tool (see
`www.doxygen.org <www.doxygen.org>`_) from documentation instructions
embedded in the code .

The user manual distributed with the source code has been written in
LaTeX (see `www.latex-project.org <www.latex-project.org>`_).

Directory layout
----------------

The project directory structure is very systematic. Everything used to
build the software is stored in the directory ``./src``:

  * ``src/main``: Code related files (sources, third party libraries and headers, ...)

    -  ``src/main/<language>``: source code, per coding language: cpp (for C++), python, R, ...

    -  ``src/main/resources``: third party resources included in the project

  * ``src/doc``: documentation files (API, manual, ...)

    -  ``src/doc/doxygen_ref_man``: files needed to generate the reference documentation with Doxygen

    -  ``src/doc/user_man``: files needed to generate the user manual with Sphinx

  * ``src/test``: test related files (scripts, regression files, ...)


File formats
------------

The Stride software supports different file formats:

CSV
    | Comma separated values, used for population input data and
      simulator output.

HDF5
    | Hierarchical Data Format 5, designed to store and organize large
      amounts of data.

JSON
    | JavaScript Object Notation, an open standard format that uses
      human-readable text to transmit objects consisting of
      attribute-value pairs.

TXT
    | Text files, for the logger.

XML
    | Extensible Markup Language, a markup language (both human-readable
      and machine-readable) that defines a set of rules for encoding
      documents.

Testing
-------

Unit tests and install checks are added to Stride based on Google’s
*gtest* framework. In addition, the code base
contains assertions to verify the simulator logic. They are activated
when the application is built in debug mode and can be used to catch
errors at run time.

Results
-------

The software can generates different output files:

cases.csv
    | Cumulative number of cases per day.

person.csv
    | Individual details on infection characteristics.

logfile.txt
    | Details on transmission and/or social contacts events.
