parameter sweep
* resistive divider , R1 swept from start_r to stop_r
* replaces .STEP R1 1k 10k 1k
R1 1 2 1k
R2 2 0 1k
VDD 1 0 DC 1
.dc VDD 0 1 .1
.control
let start_r = 1k
let stop_r = 10k
let delta_r = 1k
let r_act = start_r
* loop
while r_act le stop_r
alter r1 r_act
run
write dc -sweep.out v(2)
set appendwrite
let r_act = r_act + delta_r
end
plot dc1.v(2) dc2.v(2) dc3.v(2) dc4.v(2) dc5.v(2)
+ dc6.v(2) dc7.v(2) dc8.v(2) dc9.v(2) dc10.v(2)
.endc
.end