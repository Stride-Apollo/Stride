Checkpointing
=============

Configuring checkpointing
-------------------------

*Checkpointing is configured using command line options and/or
specifying certain parameters in the configuration file. These options
are specified in the chapters above.* *For more detailed information on
how to configure checkpointing, read the Simulator chapter (Run the
simulator part).*

| Checkpointing enables the ability to save the state of the simulator
  multiple times during the simulation itself. The simulator state is
  saved in a binary format, based on a HDF5 storage format. The format
  of this file is described below.
| Checkpointing is configured by 3 parameters: the checkpointing
  frequency, the checkpointing file and the simulator run mode.

Checkpointing frequency
~~~~~~~~~~~~~~~~~~~~~~~

| The amount of times the simulator will be saved, can be set by the
  checkpointing frequency parameter. This parameter can be set by using
  a commandline argument or specifying the parameter in the xml
  configuration file. This are the possible values for the parameter:

-  0 - Only save the last timestep of the simulation

-  x - Save the simulator state every x timesteps

Checkpointing file
~~~~~~~~~~~~~~~~~~

This paramater specifies the name of the checkpointing file. The use for
the file depends on the simulator run mode parameter.

Simulator run mode
~~~~~~~~~~~~~~~~~~

| The simulator can be run in different modes. Currently, these run
  modes are supported:

-  Initial - The simulator is built from scratch using the configuration
   file, and saved every x timesteps according to the checkpointing
   frequency. If no configuration file is present, the initial saved
   state in the checkpointing file is used to start the simulation.

-  Extend - The simulation is extended from the last saved checkpoint in
   the checkpointing file.

-  Extract - A configuration file is extracted from the checkpointing
   file. This mode will not actually run the simulator itself.

Table structure
---------------

+-------------------------+---------+---------------------+
| /configuration          | rank    | 1                   |
+-------------------------+---------+---------------------+
|                         | dims    | 1                   |
+-------------------------+---------+---------------------+
|                         | dtype   | ConfDatatype        |
+-------------------------+---------+---------------------+
| /track\_index\_case     | rank    | 1                   |
+-------------------------+---------+---------------------+
|                         | dims    | 1                   |
+-------------------------+---------+---------------------+
|                         | dtype   | H5T\_NATIVE\_INT    |
+-------------------------+---------+---------------------+
| /amt\_timesteps         | rank    | 1                   |
+-------------------------+---------+---------------------+
|                         | dims    | n\_steps            |
+-------------------------+---------+---------------------+
|                         | dtype   | H5T\_NATIVE\_UINT   |
+-------------------------+---------+---------------------+
| /personsTI              | rank    | 1                   |
+-------------------------+---------+---------------------+
|                         | dims    | amt\_persons        |
+-------------------------+---------+---------------------+
|                         | dtype   | PersonTIDatatype    |
+-------------------------+---------+---------------------+
| /Timestep\_n/randomgen  | rank    | 1                   |
+-------------------------+---------+---------------------+
|                         | dims    | amt\_threads        |
+-------------------------+---------+---------------------+
|                         | dtype   | RNGDatatype         |
+-------------------------+---------+---------------------+
| /Timestep\_n/PersonTD   | rank    | 1                   |
+-------------------------+---------+---------------------+
|                         | dims    | amt\_persons        |
+-------------------------+---------+---------------------+
|                         | dtype   | PersonTDDatatype    |
+-------------------------+---------+---------------------+
| /Timestep\_n/Calendar   | rank    | 1                   |
+-------------------------+---------+---------------------+
|                         | dims    | 1                   |
+-------------------------+---------+---------------------+
|                         | dtype   | CalendarDatatype    |
+-------------------------+---------+---------------------+

Custom datatypes
~~~~~~~~~~~~~~~~

ConfDatatype
^^^^^^^^^^^^

-  StrType (variable length) - conf\_content

-  StrType (variable length) - disease\_content

-  StrType (variable length) - holidays\_content

-  StrType (variable length) - age\_contact\_content

RNGDatatype
^^^^^^^^^^^

-  H5T\_NATIVE\_ULONG - seed

-  StrType (variable length) - rng\_state

PersonTIDatatype (time independent)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  H5T\_NATIVE\_UINT - ID

-  H5T\_NATIVE\_DOUBLE - age

-  H5T\_NATIVE\_CHAR - gender

-  H5T\_NATIVE\_UINT - household\_ID

-  H5T\_NATIVE\_UINT - school\_ID

-  H5T\_NATIVE\_UINT - work\_ID

-  H5T\_NATIVE\_UINT - prim\_comm\_ID

-  H5T\_NATIVE\_UINT - sec\_comm\_ID

-  H5T\_NATIVE\_UINT - start\_infectiousness

-  H5T\_NATIVE\_UINT - time\_infectiousness

-  H5T\_NATIVE\_UINT - start\_symptomatic

-  H5T\_NATIVE\_UINT - time\_symptomatic

PersonTDDatatype (time dependent)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  H5T\_NATIVE\_HBOOL - at\_household

-  H5T\_NATIVE\_HBOOL - at\_school

-  H5T\_NATIVE\_HBOOL - at\_work

-  H5T\_NATIVE\_HBOOL - at\_prim\_comm

-  H5T\_NATIVE\_HBOOL - at\_sec\_comm

-  H5T\_NATIVE\_HBOOL - participant

-  H5T\_NATIVE\_UINT - health\_status

CalendarDatatype
^^^^^^^^^^^^^^^^

-  H5T\_NATIVE\_HSIZE - day

-  StrType (variable length) - date
