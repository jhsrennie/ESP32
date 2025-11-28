SieveBenchmark
--------------
This is a port of Dave Plummer's "drag race" code to speed test processors:

https://github.com/PlummersSoftwareLLC/Primes

The code here is the PrimeC solution 1 code:

https://github.com/PlummersSoftwareLLC/Primes/tree/drag-race/PrimeC/solution_1

copied and pasted with the bare minimum of changes to make it run on an ESP32. Simply upload this code and after 5 seconds it will print:

  No. passes = nnn

where nnn is the number of passes achieved in 5 seconds

I initially ran this using the Arduino IDE and the code for that is in the SieveArd subdirectory. Then I got curious about the difference in the results and reran it using the ESP-IDF to compile the program. This code is in the SieveIDF subdirectory.

I have listed both sets of results below. The Arduino IDE optimises for size (-Os) so I used the same optimisation for the IDF build, but since the IDF also allows me to optimise for speed (-O2) I tried that as well. The IDF results show both figures.

I would be cautious about reading too much into these figures. For example the C6 is 20% faster than the C3 when compiled for size but the difference is far smaller when compiled for performance. This suggests the measured difference is an artefact of the benchmark. The S3 is always faster than the original ESP32, but that is expected since the S3 has the L7 Extensa core while the ESP32 has the older L6 core.

Arduino sketch
--------------
                    No. passes
Original 240MHz ESP32  155
S3 supermini           187
C6 supermini           129
C3 supermini           110

IDF compilation
---------------
                      No. passes
Optimisation flag      -Os  -O2
Original 240MHz ESP32  148  157
S3 supermini           188  200
C6 supermini           129  141
C3 supermini           106  133

For comparison, on my laptop with an i5-1240p CPU I get around 18500 passes when compiled with -Os and around 23500 when compiled with -O2.