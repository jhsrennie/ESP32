SieveBenchmark
--------------
This is a port of Dave Plummer's "drag race" code to speed test processors:

https:github.com/PlummersSoftwareLLC/Primes

The code here is the PrimeC solution 1 code:

https:github.com/PlummersSoftwareLLC/Primes/tree/drag-race/PrimeC/solution_1

copied and pasted with the bare minimum of changes to make it run on an ESP32 (mostly replacing printf by Serial.printf).

Simply upload this code and after 5 seconds it will print:

  mckoss-c830;NNN;5.0;1;algorithm=wheel,faithful=yes,bits=1

where NNN is the number of passes achieved in 5 seconds

On the original 240MHz ESP32 I got 150 passes
On a 160MHz ESP32-C3 I got 109 passes
On my laptop with an i5-1240p CPU I got 5316 passes