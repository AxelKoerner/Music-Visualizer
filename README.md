# Music-Visualizer

A project to visualize music on an led strip. I'm using the SparkFun Spectrum Shield to split the audio into 7 different frequency bands (https://www.sparkfun.com/products/13116). Each band can be read out on its own. The amplitude of the individual frequency band is used to generate different visualizer effects. 

The project is designed to work with different microchips, led strips. It's possible to run this code with any led strip as long as it has a data pin. 
Beyond that i added global variables to change certaint hardware requirements. 
I also added a few effects that dont require a SparkFun Spectrum Shield. They are called not reactive visualizer modes. 
