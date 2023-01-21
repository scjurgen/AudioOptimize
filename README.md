# Optimize audio plugins tutorial

## synopsis

This small tutorial outlines how to optimize audio plugin code
using a not so simple echo as a base.
The echo has 2 delay lines (left right) which are modulated by a sine lfo.
Feedback, eq, a diffusor.

# preparing

Before optimizing anything you must make sure that the plugin works correctly. You don't want
making changes that will *destroy* the implementation. 

For this reason we have extensive unit-tests for every single code and also an integration test.
The tests should also be run using valgrind.

# measuring performance

Another important task is measuring your base performance.

For this reason we will use single sandboxed tests (using googles gtest suite) to establish a base line (for the processor architecture) and
parallel tests of the original and optimized version.

# improving tactics

There are various tactics that can be used

- change math (examples modulation with sine lfo, all-pass)
- predict branches (example delay)
- reorganize code blocks (example biquad filter)






