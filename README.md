# Optimize audio plugins tutorial

## synopsis

This tutorial outlines how to optimize audio plugin code
using a not so simple echo-plugin as a base.
The echo has 2 delay lines (left - right) which are modulated by a sine lfo.
Feedback, equalizer and a diffusor.

The c++ code is in some parts c++20 adapted, however, it should be easy for you to rewind it to c++17 if needed.


# preparing

Before optimizing anything you must make sure that the plugin works correctly. You don't want to
making changes that will *destroy* your current implementation. 

For this reason we have extensive unit-tests for every single code and also an integration test of the final plugin code.
The tests should also be run using valgrind to ensure you haven't leaks.

# measuring performance

An important task is measuring your base performance.

For this reason we will use single sandboxed tests (using googles gtest suite) to establish a base line (for the 
processor architecture) and parallel running tests of the original and optimized version. Running them in parallel is 
to lower the probability that other processes interfere with our test, because the scheduler will interrupt our 2 running
tests in the same moment and then pick up our processes again. IO related task should then have less influence.

For more information refer to the [README.md](dsp-code/unit-tests/performance/README.md) in 
the [dsp-code/unit-tests/performance](dsp-code/unit-tests/performance) folder.


# improving tactics

There are various tactics that can be used

- change math (examples modulation with sine lfo, all-pass)
- predict branches (example delay)
- reorganize code blocks (example biquad filter)


# Todo

- [ ] cleanup all tests
- [ ] add valgrind DockerFile
- [ ] check with MSVC 
- [ ] Document all tests and relative optimisations
- [ ] Remove unused Modules




