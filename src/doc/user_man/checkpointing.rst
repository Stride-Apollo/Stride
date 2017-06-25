Checkpointing
=============

Configuring checkpointing
-------------------------

*Checkpointing is configured using command line options and/or
specifying certain parameters in the configuration file. These options
are specified in the chapters above.* *For more detailed information on
how to configure checkpointing, read the Simulator chapter (Run the
simulator part).*

Checkpointing enables the ability to save the state of the simulator
multiple times during the simulation itself. The simulator state is
saved in a binary format, based on a HDF5 storage format. The format
of this file is specified below.
Checkpointing is configured using 3 parameters: the checkpointing
frequency, the checkpointing file and the simulator run mode.

Checkpointing frequency
~~~~~~~~~~~~~~~~~~~~~~~

How frequently the simulator will be saved, can be set by the
checkpointing frequency parameter. This parameter can be set by using
a commandline argument or specifying the parameter in the xml
configuration file. This are the possible values for the parameter:

-  0 - Only save the last timestep of the simulation

-  x - Save the simulator state every x timesteps

Checkpointing file
~~~~~~~~~~~~~~~~~~

This paramater specifies the name of the checkpointing file. The use for
the file depends on the simulator run mode parameter.

Replay timestep
~~~~~~~~~~~~~~~

This parameter is used when the run mode is ``Replay``. It specifies the timestep from which you want to start the simulation.

Simulator run mode
~~~~~~~~~~~~~~~~~~

The simulator can be run in different modes. Currently, the following run
modes are supported:

.. todo::

No starting from checkpointed file in initial mode?.

-  Initial - The simulator is built from scratch using the configuration
   file, and is saved every x timesteps according to the checkpointing
   frequency. If no configuration file is present, the initial saved
   state in the checkpointing file is used to start the simulation.


-  Extend - The simulation is extended from the last saved checkpoint in
   the checkpointing file.

-  Replay - The simulation is replayed from a specified timestep.

-  Extract - The configuration files are extracted from the checkpointing
   file. This mode will not actually run the simulator itself.

HDF5 file format
----------------

Table structure
~~~~~~~~~~~~~~~

+---------------------------------------------+---------+---------------------+
| /Configuration/configuration                | rank    | 1                   |
+---------------------------------------------+---------+---------------------+
|                                             | dims    | 1                   |
+---------------------------------------------+---------+---------------------+
|                                             | dtype   | ConfigDatatype      |
+---------------------------------------------+---------+---------------------+
| /amt\_timesteps                             | rank    | 1                   |
+---------------------------------------------+---------+---------------------+
|                                             | dims    | 1                   |
+---------------------------------------------+---------+---------------------+
|                                             | dtype   | H5T\_NATIVE\_UINT   |
+---------------------------------------------+---------+---------------------+
| /last\_timestep                             | rank    | 1                   |
+---------------------------------------------+---------+---------------------+
|                                             | dims    | 1                   |
+---------------------------------------------+---------+---------------------+
|                                             | dtype   | H5T\_NATIVE\_UINT   |
+---------------------------------------------+---------+---------------------+
| /person_time_independent                    | rank    | 1                   |
+---------------------------------------------+---------+---------------------+
|                                             | dims    | amt\_persons        |
+---------------------------------------------+---------+---------------------+
|                                             | dtype   | PersonTIDatatype    |
+---------------------------------------------+---------+---------------------+
| /Timestep\_n/randomgen                      | rank    | 1                   |
+---------------------------------------------+---------+---------------------+
|                                             | dims    | 1                   |
+---------------------------------------------+---------+---------------------+
|                                             | dtype   | StrType             |
+---------------------------------------------+---------+---------------------+
| /Timestep\_n/person\_time\_dependent        | rank    | 1                   |
+---------------------------------------------+---------+---------------------+
|                                             | dims    | amt\_persons        |
+---------------------------------------------+---------+---------------------+
|                                             | dtype   | PersonTDDatatype    |
+---------------------------------------------+---------+---------------------+
| /Timestep\_n/calendar                       | rank    | 1                   |
+---------------------------------------------+---------+---------------------+
|                                             | dims    | 1                   |
+---------------------------------------------+---------+---------------------+
|                                             | dtype   | CalendarDatatype    |
+---------------------------------------------+---------+---------------------+
| /Timestep\_n/travellers                     | rank    | 1                   |
+---------------------------------------------+---------+---------------------+
|                                             | dims    | amt\_travellers     |
+---------------------------------------------+---------+---------------------+
|                                             | dtype   | TravellerDataType   |
+---------------------------------------------+---------+---------------------+
| /Timestep\_n/household\_clusters            | rank    | 1                   |
+---------------------------------------------+---------+---------------------+
|                                             | dims    | amt\_persons        |
+---------------------------------------------+---------+---------------------+
|                                             | dtype   | H5T\_NATIVE\_UINT   |
+---------------------------------------------+---------+---------------------+
| /Timestep\_n/primary\_community\_clusters   | rank    | 1                   |
+---------------------------------------------+---------+---------------------+
|                                             | dims    | amt\_persons        |
+---------------------------------------------+---------+---------------------+
|                                             | dtype   | H5T\_NATIVE\_UINT   |
+---------------------------------------------+---------+---------------------+
| /Timestep\_n/secondary\_community\_clusters | rank    | 1                   |
+---------------------------------------------+---------+---------------------+
|                                             | dims    | amt\_persons        |
+---------------------------------------------+---------+---------------------+
|                                             | dtype   | H5T\_NATIVE\_UINT   |
+---------------------------------------------+---------+---------------------+
| /Timestep\_n/work\_clusters                 | rank    | 1                   |
+---------------------------------------------+---------+---------------------+
|                                             | dims    | amt\_workers        |
+---------------------------------------------+---------+---------------------+
|                                             | dtype   | H5T\_NATIVE\_UINT   |
+---------------------------------------------+---------+---------------------+
| /Timestep\_n/school\_clusters               | rank    | 1                   |
+---------------------------------------------+---------+---------------------+
|                                             | dims    | amt\_students       |
+---------------------------------------------+---------+---------------------+
|                                             | dtype   | H5T\_NATIVE\_UINT   |
+---------------------------------------------+---------+---------------------+


Custom datatypes
~~~~~~~~~~~~~~~~

ConfigDatatype
^^^^^^^^^^^^

-  StrType - config\_content

-  StrType - disease\_content

-  StrType - holidays\_content

-  StrType - age\_contact\_content


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

-  H5T\_NATIVE\_HBOOL - participant

-  H5T\_NATIVE\_UINT - health\_status

-  H5T\_NATIVE\_UINT - disease\_counter

-  H5T\_NATIVE\_UINT - on\_vacation

CalendarDatatype
^^^^^^^^^^^^^^^^

-  H5T\_NATIVE\_HSIZE - day

-  StrType - date

TravellerDataType
^^^^^^^^^^^^^^^^^
This type consists of person data from original simulator, as well as data from the new simulator.
Person data which is similair over both simulators is only saved once (such as gender data).

Other than that, the data type also contains metadata information:

- H5T\_NATIVE\_VARIABLE - home\_sim\_name

- H5T\_NATIVE\_VARIABLE - dest\_sim\_name

- H5T\_NATIVE\_UINT - home\_sim\_index

- H5T\_NATIVE\_UINT - dest\_sim\_index

- H5T\_NATIVE\_UINT - days\_left



.. role:: underline
    :class: underline


Elaboration
~~~~~~~~~~~

First of all, the configuration files are saved. This allows for independent runs for later simulations,
by using the stored configurations.

In terms of person data, the time independent data is saved once. The time dependent data is stored at each save.

The order of person id's in the different cluster types is saved as well. This,
in combination with the saving of the rng state, guarantees that the run can be
resumed exactly similair to the state in which it was saved. This also allows
the exact same end results when running the simulation :underline:`without multithreading`.

As part of the multi region extension, travellers are saved too. This allows
for a reconstruction of the simulation with multi region travellers present.

