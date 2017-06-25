Simulator
=========

Workspace
---------

By default, Stride is installed in ``./target/installed/`` inside the
project directory though this can be modified using the
CMakeLocalConfig.txt file (example is given in
``./src/main/resources/make``). Compilation and installation of the
software will create the following files and directories:

Binaries in directory ``<project_dir>/bin``

-  ``stride``: executable.

-  ``gtester``: regression tests for the sequential code.

-  ``wrapper_sim.py``: Python simulation wrapper

Configuration files (xml and json) in directory ``<project_dir>/config``

-  ``run_default.xml``: default configuration file for Stride to
   perform a Nassau simulation.

-  ``run_miami_weekend.xml``: configuration file for Stride to
   perform Miami simulations with uniform social contact rates in the
   community clusters.

-  ``wrapper_miami.json``: default configuration file for the
   wrapper_sim binary to perform Miami simulations with different
   attack rates.

-  ...

Data files (csv) in directory ``<project_dir>/data``

-  ``belgium_commuting``: Belgian commuting data for the active
   populations. The fraction of residents from “city_depart” that are
   employed in “city_arrival”. Details are provided for all cities and
   for 13 major cities.

-  ``belgium_population``: Relative Belgian population per city.
   Details are provided for all cities and for 13 major cities.

-  ``contact_matrix_average``: Social contact rates, given the
   cluster type. Community clusters have average (week/weekend) rates.

-  ``contact_matrix_week``: Social contact rates, given the cluster
   type. Community clusters have week rates.

-  ``contact_matrix_week``: Social contact rates, given the cluster
   type. Primary Community cluster has weekend rates, Secondary
   Community has week rates.

-  ``disease_xxx``: Disease characteristics (incubation and
   infectious period) for xxx.

-  ``holidays_xxx``: Holiday characteristics for xxx.

-  ``pop_xxx``: Synthetic population data extracted from the 2010
   U.S. Synthetic Population Database (Version 1) from RTI International
   for xxx :cite:`wheaton2014a`, :cite:`wheaton2014b`.

-  ``ref_2011``: Reference data from EUROSTAT on the Belgian
   population of 2011. Population ages and household sizes.

-  ``ref_fl2010_xxx``: Reference data on social contacts for
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

The usage of a configuration file is necessary unless you choose the extract mode. From this configuration file all the necessary files will be read.

Overrides can be used to quickly override a certain value in the configuration. Multiple ``-o`` flags can be given. This is based on the API of the Boost property tree, with the following exceptions:

  * The first element is always ``run``, so this is implicit.
  
  * To get an XML attribute, you can use ``@`` instead of ``<xmlattr>.``.
  
Some typical examples:

  * ``-o @name=other_name`` to change the name of the current run (and therefore the output directory)
  
  * ``-o disease.config=other_disease.xml`` to try another disease.

Output can be found in ``output/<name>``. Mind the fact that you will overwrite your previous run if you don't change the name.

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
        <checkpointing frequency="1"/>
    </outputs>
    <disease>
        <seeding_rate>0.002</seeding_rate>
        <immunity_rate>0.8</immunity_rate>
        <config>disease_measles.xml</config>
    </disease>
    <regions>
        <region name="Belgium">
            <rng_seed>1</rng_seed>
            <raw_population>pop_nassau.csv</raw_population>
        </region>
        <region name="">
            <rng_seed>1</rng_seed>
            <population>pop.xml</population>
        </region>
    </regions>
  </run>

The population, as referenced in a ``<region>`` can be either a ``<raw_population>`` or a ``<population>``. The first option is a simple csv, the second one an XML file with the following format:

.. code-block:: xml

  <?xml version="1.0" encoding="utf-8"?>
  <population>
      <people>people.csv</people>
      <districts>cities.csv</districts>
      <sphere_of_influence speed="100" size="20" min="5"/>
      <clusters>clusters.csv</clusters>
      <households>households.csv</households>
      <cities>
          <city name="Antwerp" pop="5000" lat="51.123" lon="4.567">
              <airport name="ANR"/>
          </city>
          <city name="Brussels" pop="10000" lat="50.850" lon="4.348">
              <airport name="BRU"/>
          </city>
      </cities>
  </population>

Here, the ``<people>`` tag refers to the same kind of file as a ``<raw_population>``.
  
You can use multiple regions for the multi region feature.
The output tags ``<visualization/>`` and ``<checkpointing_frequency/>`` enable the saving of hdf5 or visualization files.
