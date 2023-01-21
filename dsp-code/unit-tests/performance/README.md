# Performance test

There are always 2 types of test in the Performance test.

- check the speed in your buildsystem
- check relatives speeds when trying to speed optimize code

# Buildsystem speed

You will need to adapt the values for your build machine.

When building in the cloud (e.g. circleci) you might bump up the established values because the virtual 
machines might be varying a lot the speed.

## Important to understand

You should test on the target machine(s), which could be an embedded device. So run the code on the target machine.
Whenever you have a buildsystem that is crosscompiling you could try to automatize the process of uploading and running 
the code to your targetdevice (flashing, ssh copying, uploading)


# Optimizing performance

Optimizing performance tests are run by running two algorithms at the same time and check which one performs better.

```YourSecretClass sut; // sut = system under test, if you want you could also call it suo (system Under Observation)```

```YourSecretClassOptimized sutOptimized;```

## Important to understand

You should optimize for the target machine, which could be also an embedded device. 
So run the code on the target machine.
