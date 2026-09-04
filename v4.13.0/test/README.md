# host based unit tests for JLed

* the tests are using the [catch unit testing framework](https://github.com/catchorg/Catch2).
* test results are available on [coveralls](https://coveralls.io/github/jandelgado/jled)

Pass `OPT` argument to `make` to control optimization, which affects code
coverage, e.g.:
* `make clean coverage OPT=-O0`
* `make clean test OPT=-O2` 
