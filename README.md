
# SWR2 Refactor

Before using this program please read all the license files in the 
~/swr-2.5/doc/ directory. Running this program implies understanding
and acceptance of any terms outlined therin.

To compile you must first generate the Makefile with the following command:

``cmake .``

Note the punctuation mark. It won't work without it.

To compile type:

``make``

If you have multiple cores you can do a parallel build
with ``make -j4`` where you can change the 4 to the number of cores you have.

To run the mud run the startup script by typing:

``./run-swr &``

To connect to the mud point your favorite mud client to port 9999. To change the default port you
can either run the startup script using the desired port number as a 
command line argument or edit the script.

To give your character all the commands first save and quit. Then locate your 
player file in the ~/swr-2.5/player/ directories. Edit the line that says 
Toplevel changing the number from 1 to 200. Then log back on. Type "holylight"
and "config +roomflags" for full immortal vision.

Oh yeah,
this code is unfinished and buggy despite my best efforts.

Enjoy

For bug reports, new releases and general discussion visit
https://swr2.ceris.no/
