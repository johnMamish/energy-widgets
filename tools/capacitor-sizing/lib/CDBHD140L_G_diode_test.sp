diode sanity check

.include CDBHD140L_G.lib

X1 pos gnd CDBHD140L_G_schottky
V1 pos gnd 500m

** the internet and the ngspice manual don't tell me how the hell to look at the current of a DC
** operating point calculation, so this is what we're gonna do.
B1 result gnd V=I(X1)

.control
    let i = 0
    let v = 0.01
    let vals = unitvec(50)
    let results = unitvec(50)
    while (i < 50)
        alter V1 v
        op
        print result
        print v
        let results[i] = result
        let vals[i] = v
        let i = i + 1
        let v = v + 0.01
    end
    plot ylog results vs vals
    print results[20]
    print results[30]
.endc

.end