# Documentation

Generate printouts and wave files for some components

This serves various purposes:

 - check visually if all runs nicely, e.g. filter curves are as expected
 - check the audio itself without bothering the host
 - have printouts for clients, audioengineers, marketing
 - some printouts demonstrate also how to get data for a frontend, e.g.: Filtercurves, without running a simulation

## Requirements

To keep the programming part easy you need:

- sox
- gnuplot

sox will transform the raw data output to a desired format (.wav .flac etc.)

gnuplot creates some prints (.txt, .svg, .png etc.)



