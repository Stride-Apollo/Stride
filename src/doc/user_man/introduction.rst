Introduction
============

This manual provides a brief description of the Stride software and its
features. Stride stands for **S**\ imulation of **tr**\ ansmission of
**i**\ nfectious **d**\ is\ **e**\ ases and is an agent-based modeling
system for close-contact disease transmission developed by researchers
at the University of Antwerp and Hasselt University, Belgium. The
simulator uses census-based synthetic populations that capture the
demographic and geographic distributions, as well as detailed social
networks. Stride is an open source software. The authors hope to make
large-scale agent-based epidemic models more useful to the community.
More info on the project and results obtained with the software can be
found in :cite:`willem2015`.

The model population consists of households, schools, workplaces and
communities, which represent a group of people we define as a “cluster”.
Social contacts can only happen within a cluster. When school or work is
off, people stay at home and in their primary community and can have
social contacts with the other members. During other days, people are
present in their household, secundary community and a possible workplace
or school.

We use a ``Simulator`` class to organize the activities from the
people in an ``Area``. The Area class has a ``Population``,
different ``Cluster`` objects and a ``Contact Handler``. 
The ``Contact Handler`` performs Bernoulli trials to
decide whether a contact between an infectious and susceptible person
leads to disease transmission. People transit through
Susceptible-Exposed-Infected-Recovered states, similar to an
influenza-like disease. Each ``Cluster`` contains a link to its
members and the ``Population`` stores all personal data, with
``Person`` objects. The implementation is based on the open source
model from Grefenstette et al. :cite:`grefenstette2013`. The
household, workplace and school clusters are handled separately from the
community clusters, which are used to model general community contacts.
The ``Population`` is a collection of ``Person`` objects.
