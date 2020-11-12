constant current model

V1 in 0 dc 0 PULSE (0 5 1u 1u 1u 1 1)
R1 in out 1k
C1 out gnd 1u ic=0

.tran 1u 50m

.control let loop = 0
    while (cont < 3)
        *echo within while loop "$&loop: Tran results"
        run
        plot out
        let loop = loop + 1
    end
    unlet loop
.endc

.end