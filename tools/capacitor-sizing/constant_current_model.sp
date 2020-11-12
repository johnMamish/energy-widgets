constant current model netlist

.include lib/1N4148.lib

*******
* This subcircuit is a full-bridge rectifier
*     ac0    one of the 2 non-polarized ac inputs
*     ac1    one of the 2 non-polarized ac inputs
*     outp   positive rectified output
*     outn   negative rectified output (probably wanna connect this one to ground)
*******
.subckt rectifier ac0 ac1 outp outn
    X1 ac1  outp  1N4148
    X2 ac0  outp  1N4148
    X3 outn ac1   1N4148
    X4 outn ac0   1N4148
.ends rectifier

*******
* This circuit models the current draw of a constant-current system with a variable efficiency
* buck regulator on the input.
*
* If the voltage from pos to neg is less than some vmin, then no current is consumed.
*******
.subckt current_sink pos neg
    B1 pos neg I = V(pos,neg) > 2 ? 10m : 1u
.ends current_sink


*** The harvester consists of a piecewise voltage source in series with a 260 ohm resistor.
*** Its output nodes are 'harvest0' and 'harvest1'
.model filesrc filesource(file="motor_voc.m" amploffset=[0] amplscale=[1])
asrc %vd([harvestint1 harvest0]) filesrc
Rharvest harvestint1 harvest1 260

*** Instantiate rectifier
X1 harvest1 harvest0 capout gnd rectifier

*** Harvest capacitor and resistor to draw charge off of it
C1 capout gnd 330u
X2 capout gnd current_sink

*** Transient simulation of 300 milliseconds at timestep of 1u
.control
    tran 1us 300ms
    plot harvestint1 harvest1 capout
.endc

.end