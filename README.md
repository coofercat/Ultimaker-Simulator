Ultimaker-Simulator
===================

An Arduino sketch that makes any Arduino behave a bit like an Ultimaker 3D
printer that's running Marlin firmware.

This sketch can be used to develop serial gcode senders without needing a full
printer to hand.

When a simulated print runs, most Gcode commands are just read and essentially
ignored (other than to acknowledge them). M105 will return a valid temperature,
and to help a little bit with debugging, M106/M107 will turn the Arduino pin 13
LED on and off (as a sort of virtual fan).

In addition, a non-standard command, P1 will cause the next numbered,
checksummed command will be considered a checksum failure. This can be used to
simulate some serial transfer issues.
