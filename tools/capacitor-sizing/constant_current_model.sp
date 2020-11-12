constant current model

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
* This circuit is just a basic LC low-pass filter with parasitic resistive elements
*******
.subckt lc in gnd out
    L1 in outrl 0
    RL1 outrl out 10m
    C1 outcl gnd 10u
    RC1 outcl out 10m
.ends lc

*******
* This circuit models the current draw of a constant-current system with a variable efficiency
* buck regulator on the input.
*
* If the voltage from pos to neg is less than some vmin, then no current is consumed.
*******
.subckt current_sink pos neg Isys=1m Vsys=2
    * If the current is a step function, it results in a badly conditioned simulation
    * This step voltage source is fed into a low-pass LC filter whose output controls the current
    Bvctl vctlstep gnd V = V(pos,neg) >= Vsys ? 1 : 0
    X1 vctlstep gnd vctl lc

    * ammeter for I-source modelling system
    V1 pos pos1 0

    * I-source that models whole system
    B1 pos1 neg I = V(vctl) * Isys

    * Voltages calculating
    B2 efficency gnd V = Vsys / V(pos,neg)
    B3 pgood gnd V = I(V1) * V(pos,neg) * V(efficency)
.ends current_sink

*.options reltol=0.1 abstol=1e-3 vntol=0.1e-3

*** The harvester consists of a piecewise voltage source in series with a 260 ohm resistor.
*** Its output nodes are 'harvest0' and 'harvest1'
.model filesrc filesource(file="motor_voc.m" amploffset=[0] amplscale=[1])
asrc %vd([harvestint1 harvest0]) filesrc
*V1 harvestint1 harvest0 5
Rharvest harvestint1 harvest1 260

*** Instantiate rectifier
X1 harvest1 harvest0 capout gnd rectifier

*** Harvest capacitor and resistor to draw charge off of it
Charvest capout gnd 300u
Vsysammeter capout capoutsys 0
X2 capoutsys gnd current_sink Isys=1m Vsys=2

*** Integrate power
*ESENS vs gnd X2.pgood gnd 1
A1 %v(X2.pgood) harvested_energy integrator
.model integrator int(in_offset=0.0 gain=1 out_lower_limit=-1e12 out_upper_limit=1e12 limit_range=1e-9 out_ic=0.0)

.measure tran vtest find v(harvested_energy) AT=999ms

*** Transient simulation of 300 milliseconds at timestep of 1u
.control
    let loop = [0 0]
    while (loop[0] < 3)
        let c_val = 10u + (loop[0] * 10u)
        alter Charvest c_val
        echo within while loop "$&loop: Tran results"
        tran 10u 1000m
        run
        *plot harvestint1 harvest1 capout
        *plot X2.pgood
        *plot harvested_energy
        *plot I(Vsysammeter)
        let loop[0] = loop[0] + 1
    end
.endc

.end