# host based unit tests for JLed

The tests are using the [catch unit testing framework](https://github.com/catchorg/Catch2).

Pass `OPT` argument to make to control optimization, which affects code
coverage, e.g.:
* `make clean coverage OPT=-O0`
* `make clean test OPT=-O2` 
