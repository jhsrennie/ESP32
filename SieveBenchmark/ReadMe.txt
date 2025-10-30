SieveBenchmark
--------------
This is a port of Dave Plummer's "drag race" code to speed test processors:

https://github.com/PlummersSoftwareLLC/Primes

The code here is the PrimeC solution 1 code:

https://github.com/PlummersSoftwareLLC/Primes/tree/drag-race/PrimeC/solution_1

copied and pasted with the bare minimum of changes to make it run on an ESP32 (mostly replacing printf by Serial.printf).

Simply upload this code and after 5 seconds it will print:

  No. passes = nnn

where nnn is the number of passes achieved in 5 seconds

Original 240MHz ESP32  152 passes
S3 supermini           187 passes
C6 supermini           128 passes
C3 supermini           109 passes

On my laptop with an i5-1240p CPU I get around 7100 passes