# metal-halide-ctrl

![](hw.jpg)

A controller for Schiederwerk [PVG 12-12 AC](http://schiederwerk.de/images/infobox/doc/id26/PVG12-12ACSL-spec-sheet.pdf) Electronic Lamp Power Supply for metal halide lamps. 

The controller is based on Arduino board that is powered via 24V bus provided by PVG power supply. Two thermistors are used to measure temperature on the heatsink of the 
PVG unit. Cooling fan is controlled via PWM from Arduino. Potentiometer and another PWM output is used to control dimming of the metal halide lamp. Startup delay and fault indication is implemented.
