
Population Generator
====================

Using the generator
-------------------

Command line interface
~~~~~~~~~~~~~~~~~~~~~~

We provided the population generator with a command line interface (TCLAP).
This interface contains a help flag which gives more information about the specific arguments.
In order to display this help you must execute the following command:

.. code-block:: bash

  ./pop_generator -h

Input files
~~~~~~~~~~~

The population generator needs two files:

-  The xml configuration

-  A file with the configuration of families

An example of an xml configuration file can be found here: ``src/main/resources/templates/PopGenerator.xml``

.. literalinclude:: PopGenExample.xml

Meaning of the attributes in this file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   * ``population::total``: The total size of the population, the result may contain a small difference

   * ``population::provinces``: The amount of provinces (currently has no effect in stride)

   * ``population.family::file``: The file containing the family configurations

   * ``population.commutingdata::start_radius``: The start radius for when a person is commuting, this person will be assigned to a cluster within this radius

   * ``population.commutingdata::factor``: If there are no clusters within the start radius, multiply it by this factor and search for other clusters

   * ``population.cities.city::name``: The name of a city, this city will be generated

   * ``population.cities.city::pop``: The amount of people in this city

   * ``population.cities.city::lat``: The latitude of the city

   * ``population.cities.city::lon``: The longitude of the city

   * ``population.cities.city.airport::name``: The name of an airport situated in this city

   * ``population.villages::radius``: When calculating the position of a village, the generator first calulates the weighted middle (average latitude and longitude) of the cities. Then it calculates the maximum distance between this middle and any city. The radius is then used as a factor to determine the maximum distance a village is located from the middle.

   * ``population.villages.village``: This contains a template of a village. When a village is needed, the generator will pick a random template

   * ``population.villages.village::min``: The minimum size of the village

   * ``population.villages.village::max``: The maximum size of the village

   * ``population.villages.village::fraction``: The chance of this village template being chosen, all fractions must add up to one

   * ``population.education.mandatory``: Contains the configuration for mandatory schools

   * ``population.education.mandatory::total_size``: Size of a mandatory school

   * ``population.education.mandatory::cluster_size``: Size of a group within a mandatory school

   * ``population.education.optional``: It's attributes and purpose are the same as mandatory schools

   * ``population.education.optional::total_size``: Size of an optiona school

   * ``population.education.optional::cluster_size``: Size of a group within an optional school

   * ``population.education.optional.far::fraction``: Fraction of students that goes to a school that is further away from his home

   * ``population.work::size``: The size of a workplace

   * ``population.work.far::fraction``: The fraction of working people that goes to workplaces that are located far away from their homes

   * ``population.community::size``: The size of a community

   * ``population.community:: average_per_person``: Currently not used

   * ``population.school_work_profile.mandatory::min``: The minimum age for
     students on a mandatory school

   * ``population.school_work_profile.mandatory::max``: The maximum age for
     students on a mandatory school

   * ``population.school_work_profile.employable::fraction``: Fraction of
     people that is employed. The others are either students on an optional
     school, or unemployed.

   * ``population.school_work_profile.employable.young_employee::min``: Minimum age for students at an optional school

   * ``population.school_work_profile.employable.young_employee::max``: Maximum age for students at an optional school

   * ``population.school_work_profile.employable.young_employee::fraction``: Fraction of people within the age range of students that have a job

   * ``population.school_work_profile.employable.employee::min``: Minimum age of a working person (that is too old for any school)

   * ``population.school_work_profile.employable.employee::max``: Maximum age of a working person (that is too old for any school)

Restrictions / options
~~~~~~~~~~~~~~~~~~~~~~~~~
Some min-max pairs should not overlap. E.g. min-max pairs of villages or the maximum age of a young employee should be smaller than the minimum age of an "old"
 employee.
 Also, the ``factor`` in ``population.commutingdata`` should be greater than 1.0.
 Cities aren't required to have airports, and they can have as many airports as you'd like.
Family configuration file
~~~~~~~~~~~~~~~~~~~~~~~~~

This contains the possible configurations of each family (based on age).
Every row is a family. The Population Generator randomly chooses
configuration.

.. _1:

Random generators
~~~~~~~~~~~~~~~~~
The population generator uses the mt19937 random generator by default. Besides the mt19937 random generator, you can choose one of the following:

  - default_random_engine
  - mt19937_64
  - minstd_rand0
  - minstd_rand
  - ranlux24_base
  - ranlux48_base
  - ranlux24
  - ranlux48
  - knuth_b

For more information about these generators please go to http://www.cplusplus.com/reference/random/.
