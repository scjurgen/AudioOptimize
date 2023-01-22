# Performance test

There are always 2 types of tests in the **performance test**.

- check the speed in your buildsystem
- check relative speeds when trying to optimize code for speed

Important for all tests: build and run in **Release Mode**!

## Buildsystem speed

You will need to adapt the values for your build machine.
```c++
    // docker debian bookworm
    if (std::string(__VERSION__).find(std::string("11.2")) != std::string::npos)
    {
        EXPECT_NEAR(66, msecs, 20);
    }
    // local build mac
    else if (std::string(__VERSION__).find("Apple LLVM 13.") != std::string::npos)
    {
        EXPECT_NEAR(90, msecs, 20);
    }
    else
    {
        std::cerr << "THIS VERSION IS NOT TESTED AND NEEDS CALIBRATION: " << __VERSION__ << std::endl;
    }
#endif
#ifdef _MSC_VER
    std::cerr << "THIS MSC VERSION IS NOT TESTED AND NEEDS CALIBRATION: " << _MSC_VER << std::endl;
#endif
#else
    std::cerr << "Debug version will not be tested" << std::endl;
#endif

```
When building in the cloud (e.g. circleci) you might bump up the established values because the virtual 
machines might be varying a lot the speed.

### Important to understand

You should test on the target machine(s), which could be an embedded device. So run the code on the target machine.
Whenever you have a buildsystem that is crosscompiling you could try to automatize the process of uploading and running 
the code to your targetdevice (flashing, ssh copying, uploading)


## Optimizing performance

Optimizing performance tests are run by running two algorithms at the same time and check which one performs better.

```YourSecretClass sut; // sut = system under test, if you want you could also call it suo (system Under Observation)```

```YourSecretClassOptimized sutOptimized;```

### Important to understand

You should optimize for the target machine, which could be also an embedded device. 
So run the code on the target machine.


# Modules

In the following sections some explanations, pitfalls and challenges when optimizing certain aspects.

## Crossfader

The crossfader works only for the time it fades, once the cross fade is done it simply copies samples.
For this motive you have to retrigger in the performance test the fade, otherwise you will get only results of copying memory.

The crossfader contains now 5 versions to showcase what can be done to optimize.

- std::sin std::cos version: which is the *theoretical* implementation and meets the engineers dream
- polynomial approximations: has less multiplications
- sin cos stepper: has even less multiplications, pretty precise
- sin cos tables: high cost setting up, very fast
- just linear: for sure fast, but also sufficient for our hearing experience?

## Digital Delay

This showcases how to get rid of if depending branches

Most delays are implemented as a kind of ringbuffer with a reading and writing head.
When these heads reach the buffer end they need to be wrapped back to the beginning.

Two techniques for this:

```c++
auto head{0u};
const auto bufferSize{10000u};
if (++head == bufferSize) [[unlikely]] // this might branch, but is unlikely
{
    head = 0;
}
// or
head = ++head % bufferSize; // some processors can do this in 1 step
```

In any case, the inc-if condition might be run as 3 instructions on modern architectures:
```asm
        inc     r14
        cmp     r14, 10000
        cmovae  r14, r13
```


There exists also a bitmask version, only possible when using buffersizes of 2^n.
This version is often used in smaller embedded systems
```c++
auto head{0u};
const auto bufferSize{1u<<16}; // 65536
head = ++head & (bufferSize-1);
```
which might translate (with clang x86-64 to a blazing fast) 

```asm
        inc     ebp
        and     ebp, 65535
```

Another word for the if branch version. Modern compilers might generate a whole bunch 
of different versions, depending on unrolling, cache prediction, cpu architecture.

In any case, measuring your implementation is the key for success, not cherry-picked 
strip down versions.




