
Population Generator
====================

Using the generator
-------------------

Input files
~~~~~~~~~~~

The population generator needs two files:

-  The xml configuration

-  A file with the configuration of families

An example of an xml configuration file looks as follows:

.. todo:: example xml config?

Meaning of the attributes in this file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   * ``Population::total``: The total size of the population, the result may contain a small difference

   * ``Population::provinces``: The amount of provinces (currently has no effect in stride)

   * ``Population.Random::generator``: The RNG that is used, for a list of allowed random generators, see later

   * ``Population.Random::seed``: The seed of the random generator

   * ``Population.Family::file``: The file containing the family configurations

   * ``Population.Cities.City::name``: The name of a city, this city will be generated

   * ``Population.Cities.City::pop``: The amount of people in this city

   * ``Population.Cities.City::lat``: The latitude of the city

   * ``Population.Cities.City::lon``: The longitude of the city

   * ``Population.Villages::radius``: When calculating the position of a village, the generator first calulates the middle of the cities. Then it calculates the maximum distance between this middle and any city. The radius is then used as a factor to determine the maximum distance a village is located from the middle.

   * ``Population.Villages.Village``: This contains a template of a village. When a village is needed, the generator will pick a random template

   * ``Population.Villages.Village::min``: The minimum size of the village

   * ``Population.Villages.Village::max``: The maximum size of the village

   * ``Population.Villages.Village::fraction``: The chance of this village template being chosen, all fractions must add up to one

   * ``Population.Education.Mandatory``: Contains the configuration for mandatory schools

   * ``Population.Education.Mandatory::total_size``: Size of an institution

   * ``Population.Education.Mandatory::cluster_size``: Size of a group within a school

   * ``Population.Education.Mandatory::radius``: When a pupil is searching for a school, he looks for schools within this radius If he can’t find a school, the range is doubled

   * ``Population.Education.Optional``: It's attributes and purpose are the same as Mandatory schools

   * ``Population.Education.Optional.Far::fraction``: Fraction of students that goes to a school that is further away from his home

   * ``Population.Work::size``: The size of a workplace

   * ``Population.Work.Far::fraction``: The fraction of working people that goes to workplaces that are located far away from their homes

   * ``Population.Work.Far::radius``: Analogue as the radius for schools

   * ``Population.Community::size``: The size of a community

   * ``Population.Community:: average_per_person``: Currently not used

   * ``Population.Community::radius``: Analogue as the radius for schools

   * ``Population.School_work_profile.Mandatory::min``: The minimum age for
     students on a mandatory school

   * ``Population.School_work_profile.Mandatory::max``: The maximum age for
     students on a mandatory school

   * ``Population.School_work_profile.Employable::fraction``: Fraction of
     people that is employed The others are either student on an optional
     school, or unemployed

   * ``Population.School_work_profile.Employable.Young_employee::min``: Minimum age for students at an optional school

   * ``Population.School_work_profile.Employable.Young_employee::max``: Maximum age for students at an optional school

   * ``Population.School_work_profile.Employable.Young_employee::fraction``: Fraction of people within the age range of students that have a job

   * ``Population.School_work_profile.Employable.Employee::min``: Minimum age of a working person

   * ``Population.School_work_profile.Employable.Employee::max``: Maximum age of a working person

Family configuration file
~~~~~~~~~~~~~~~~~~~~~~~~~

This contains the possible configurations of each family (based on age).
Every row is a family. The Population Generator randomly chooses
configuration.
