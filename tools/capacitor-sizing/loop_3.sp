loop test

*** it looks like variables with length 1 can't be accessed after a transient simulation has been
*** run. This is due to a bug corrected in commit c86cd740d68b7735 with commit message
*** "fix a bug: re-enable reading vecs with length 1"

.control
    let some_variable_name = 0
    while (some_variable_name < 5)
        tran 1 100
        print some_variable_name
        let some_variable_name = some_variable_name + 1
    end
.endc