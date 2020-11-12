diode sanity check netlist

.include lib/1N4148.lib

V1 vpos gnd dc 0 PULSE (0 5 1u 1u 1u 1 1)
R1 vpos out 1k
C1 out gnd 1u
X1 gnd out 1N4148

.control
    tran 1us 5ms
    plot out
.end