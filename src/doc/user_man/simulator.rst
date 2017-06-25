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
configuration using the command ``./bin/stride``. Settings can be passed to the
simulator using one or more command line arguments:

  * ``-c`` or ``--config``: The configuration file.
  
  * ``-m`` or ``--mode``: The simulation run mode (defaults to ``Extend`` mode).

  * ``-f`` or ``--hdf5_file``: The HDF5 file (only used for mode 'extract')

  * ``-o`` or ``--override``: Override configuration file options in the command line.

  * ``-t`` or ``--timestamp``: The timestep from which ``Replay`` mode is replayed.

* The usage of a configuration file is necessary unless you choose the extract mode * from this configuration file all the necessary files will be read.

Configuration File
------------------


.. code-block:: xml

  <?xml version="1.0" encoding="utf-8"?>
    <run name="default">
    <r0>11</r0>
    <start_date>2017-01-01</start_date>
    <num_days>50</num_days>
    <holidays>holidays_none.json</holidays>
    <age_contact_matrix_file>contact_matrix_average.xml</age_contact_matrix_file>
    <track_index_case>1</track_index_case>
    <num_threads>1</num_threads>
    <information_policy>Global</information_policy>
    <outputs>
        <log level="Transmissions"/>
        <person_file/>
        <participants_survey num="10"/>
        <visualization/>
        <!-- <checkpointing frequency="1"/> -->
    </outputs>
    <disease>
        <seeding_rate>0.002</seeding_rate>
        <immunity_rate>0.8</immunity_rate>
        <config>disease_measles.xml</config>
    </disease>
    <regions>
        <region name="Belgium">
            <rng_seed>1</rng_seed>
            <raw_population>pop_nassau.csv</population>
        </region>
    </regions>
  </run>


use multiple regions for the multi region feature.
The output tags ``<visualization/>`` and ``<checkpointing_frequency/>`` enable the saving of hdf5 or visualization files.
