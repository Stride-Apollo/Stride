Simulator
=========

Workspace
---------

By default, Stride is installed in ``./target/installed/`` inside the
project directory though this can be modified using the
CMakeLocalConfig.txt file (example is given in
``./src/main/resources/make``). Compilation and installation of the
software will create the following files and directories: (illustrated
in Figure [fig:workspace]):

Binaries in directory ``<project_dir>/bin``

-  ``stride``: executable.

-  ``gtester``: regression tests for the sequential code.

-  ``wrapper\_sim.py``: Python simulation wrapper

Configuration files (xml and json) in directory ``<project_dir>/config``

-  ``run\_default.xml``: default configuration file for Stride to
   perform a Nassau simulation.

-  ``run\_miami\_weekend.xml``: configuration file for Stride to
   perform Miami simulations with uniform social contact rates in the
   community clusters.

-  ``wrapper_miami.json``: default configuration file for the
   wrapper\_sim binary to perform Miami simulations with different
   attack rates.

-  ...

Data files (csv) in directory ``<project_dir>/data``

-  ``belgium\_commuting``: Belgian commuting data for the active
   populations. The fraction of residents from “city\_depart” that are
   employed in “city\_arrival”. Details are provided for all cities and
   for 13 major cities.

-  ``belgium\_population``: Relative Belgian population per city.
   Details are provided for all cities and for 13 major cities.

-  ``contact\_matrix_average``: Social contact rates, given the
   cluster type. Community clusters have average (week/weekend) rates.

-  ``contact\_matrix_week``: Social contact rates, given the cluster
   type. Community clusters have week rates.

-  ``contact\_matrix_week``: Social contact rates, given the cluster
   type. Primary Community cluster has weekend rates, Secondary
   Community has week rates.

-  ``disease\_xxx``: Disease characteristics (incubation and
   infectious period) for xxx.

-  ``holidays\_xxx``: Holiday characteristics for xxx.

-  ``pop\_xxx``: Synthetic population data extracted from the 2010
   U.S. Synthetic Population Database (Version 1) from RTI International
   for xxx :cite:`wheaton2014a`, :cite:`wheaton2014b`.

-  ``ref\_2011``: Reference data from EUROSTAT on the Belgian
   population of 2011. Population ages and household sizes.

-  ``ref\_fl2010\_xxx``: Reference data on social contacts for
   Belgium, 2011.

Documentation files in directory ``./target/installed/doc``

-  Reference manual

-  User manual

Run the simulator
-----------------

From the workspace directory, the simulator can be started with default
configuration using the command . Settings can be passed to the
simulator using one or more command line arguments:

  * ``-c`` or ``--config``: The configuration file.

  * ``-r`` or ``--r0``: To obtain the basic reproduction number, no tertiary infections.

Python Wrapper
--------------

A Python wrapper is provided to perform multiple runs with the C++
executable. The wrapper is designed to be used with .json
configuration files and examples are provided with the source code.
For example::

  ./bin/wrapper_sim –config ./config/wrapper_default.json

will start the simulator with each configuration in the file. It is
important to note the input notation: values given inside brackets can
be extended (e.g., ``"rng_seeds"=[1,2,3]``) but single values can only be
replaced by one other value (e.g., ``"days": 100``).
