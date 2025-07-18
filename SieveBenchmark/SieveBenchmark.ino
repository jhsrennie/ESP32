//----------------------------------------------------------------------
// This is a port of Dave Plummer's "drag race" code to speed test
// processors:
// https://github.com/PlummersSoftwareLLC/Primes
//
// The code here is the PrimeC solution 1 code:
// https://github.com/PlummersSoftwareLLC/Primes/tree/drag-race/PrimeC/solution_1
// copied and pasted with the bare minimum of changes to make it run on
// an ESP32 (mostly replacing printf by Serial.printf).
//
// Simply upload this code and after 5 seconds it will print:
//   mckoss-c830;NNN;5.0;1;algorithm=wheel,faithful=yes,bits=1
// where NNN is the number of passes achieved in 5 seconds
//
// On the original 240MHz ESP32 I got 150 passes
// On a 160MHz ESP32-C3 I got 109 passes
// On my laptop with an i5-1240p CPU I got 5316 passes
//----------------------------------------------------------------------
#include <assert.h>  // assert
#include <stdint.h>  // uint32_t
#include <stdlib.h>  // calloc
#include <time.h>    // clock, CLOCKS_PER_SEC
#include <string.h>  // strcmp

#define NUM_CHECKS 4
struct {
    int n;
    int piN;
} primeChecks[NUM_CHECKS] = {
    {1e6, 78498},
    {1e7, 664579},
    {1e8, 5761455},
    {1e9, 50847534},
    };

#define TRUE (1)
#define FALSE (0)
#define BOOL int

#define WORD uint64_t
#define BITS_PER_WORD (int)(sizeof(WORD) * 8)
#define BYTE unsigned char

#define indexOf(n) ((n) / 2 / BITS_PER_WORD)
#define maskOf(n) (WORD)1 << (n / 2) % BITS_PER_WORD
#define allocOf(n) indexOf(n) + 1

int maxNumber = 1e6;
BOOL fDebug = TRUE;

typedef struct {
    int maxNumber;
    WORD *buffer;
} SIEVE;

unsigned int usqrt(int n)
{
    unsigned int x;
    unsigned int xLast;

    xLast = 0;
    x = n / 2;

    while (x != xLast) {
        xLast = x;
        x = (x + n / x) / 2;
    }
    return x;
}

//
// Prime number sieve - full bitmapped but ignoring
// multiples of 2, 3, and 5.  Only testing numbers
// congruent to:
//
//   1, 7, 11, 13, 17, 19, 23, and 29 (mod 30)
//
// maxNumber - find all primes strictly LESS than this number.
//
SIEVE *primes8of30(int maxNumber) {
    // Starts off zero-initialized.
    WORD *buffer = (WORD *)calloc(allocOf(maxNumber), sizeof(WORD));
    unsigned int maxFactor = usqrt(maxNumber) + 1;

    // Allocate sieve "object"
    SIEVE *psieve = (SIEVE*) calloc(1, sizeof(SIEVE));
    psieve->buffer = buffer;
    psieve->maxNumber = maxNumber;

    // Only numbers congruent to candidates mod 30 can be prime.
    unsigned int candidates[8] = {1, 7, 11, 13, 17, 19, 23, 29};

    // Cached bitmaps and index offsets for bit twiddling loop.
    WORD masks[BITS_PER_WORD];
    unsigned int offsets[BITS_PER_WORD];

    // Build a stepping map.
    unsigned int steps[8];
    for (int i = 0; i < 8; i++) {
        steps[i] = (candidates[(i + 1) % 8] - candidates[i] + 30) % 30;
    }

    // We get 2, 3 and 5 for "free" since we ignore their multiples.
    int count = 3;
    unsigned int p;

    // First step is from 7 to 11 (steps[1]).
    unsigned int step = 1;

    // Look for next prime
    for (p = 7; p <= maxFactor; p += steps[step], step = (step + 1) % 8) {
        // A 1 bit means it's composite - keep searching.
        if (buffer[indexOf(p)] & maskOf(p)) {
            continue;
        }

        count++;
        // if (p < 1000) Serial.printf("%d, ", p);

        // The following loop is the hotspot for this algorithm.
        // No need to start less than p^2 since all those
        // multiples have already been marked.

        // Performance optimization: since the bit mask we or
        // into the word (and the index offset added to the base)
        // RECUR every 64 iterations of this loop (for 64-bit words), we
        // can precalculate them and use them over and over until the end
        // of the sieve array.

        unsigned int base = indexOf(p * p);
        unsigned int cumOffset = 0;

        for (int i = 0; i < BITS_PER_WORD; i++) {
            masks[i] = 0;
        }

        int iUsed = 0;
        int offset = 0;

        for (int i = 0, m = p * p; i < BITS_PER_WORD; i++, m += 2 * p) {
            masks[iUsed] |= maskOf(m);
            offset = indexOf(m + 2 * p) - indexOf(m);
            // Don't advance to a new word unless the next
            // bit is in the same word!
            if (offset != 0) {
                offsets[iUsed] = offset;
                iUsed++;
                cumOffset += offset;
            }
        }

        // In this case, the bits in the last word can just
        // be merged to the first.
        if (offset == 0) {
            masks[0] |= masks[iUsed];
        }

        // In all cases, iUsed will be 1 BEYOND the last mask used.

        // Now just rip through the array or-ing in these masks in an
        // identical pattern.
        unsigned int iStop = indexOf(maxNumber);
        unsigned int i = base;
        for (; i <= iStop - cumOffset;) {
            for (int j = 0; j < iUsed; j++) {
                buffer[i] |= masks[j];
                i = i + offsets[j];
            }
        }

        // Finish last few words being careful about array bounds.
        for (int j = 0; j < iUsed && i <= iStop; j++) {
            buffer[i] |= masks[j];
            i = i + offsets[j];
        }
    }

    // The wrap-around buffer pattern is expected to overwrite the
    // 7 and 11 primes as composite, erroneously.
    buffer[indexOf(7)] &= ~maskOf(7);
    buffer[indexOf(11)] &= ~maskOf(11);

    return psieve;
}

int countPrimes(SIEVE *psieve) {
    WORD *buffer = psieve->buffer;
    int maxNumber = psieve->maxNumber;

    // 2, 3, and 5 are known prime
    int count = 3;
    if (fDebug) {
        Serial.printf("2, 3, 5, ");
    }

    // Only numbers congruent to candidates mod 30 can be prime.
    unsigned int candidates[8] = {1, 7, 11, 13, 17, 19, 23, 29};

    // Build a stepping map.
    unsigned int steps[8];
    for (int i = 0; i < 8; i++) {
        steps[i] = (candidates[(i + 1) % 8] - candidates[i] + 30) % 30;
    }

    // First step is from 7 to 11 (steps[1]).
    unsigned int step = 1;

    for (unsigned int p = 7; p < maxNumber; p += steps[step], step = (step + 1) % 8) {
        if ((buffer[indexOf(p)] & maskOf(p)) == 0) {
            count++;
            if (fDebug && p < 1000) {
                Serial.printf("%d, ", p);
            }
        }
    }

    if (fDebug) {
        Serial.printf("...\n");
    }

    return count;
}

// Free up resources for sieve object
void freeSieve(SIEVE *psieve) {
    free(psieve->buffer);
    free(psieve);
}

void timedTest(int secs, SIEVE *primeFinder(int), char *title) {
    clock_t startTicks = clock();
    clock_t currentTicks;

    // One pass confirms accuracy.
    int passes = 1;

    clock_t limitTicks = secs * CLOCKS_PER_SEC + startTicks;

    // Check first for accuracy against known values.
    SIEVE *psieve = primeFinder(maxNumber);
    int primeCount = countPrimes(psieve);
    freeSieve(psieve);

    if (fDebug) {
        Serial.printf("Found %d primes up to %d.\n\n", primeCount, maxNumber);
    }

    for (int i = 0; i < NUM_CHECKS; i++) {
        if (primeChecks[i].n == maxNumber) {
            if (primeCount != primeChecks[i].piN) {
                Serial.printf("%s Expected %d primes - but found %d!\n", title,
                    primeChecks[i].piN, primeCount);
                assert(FALSE);
            }
        }
    }

    fflush(stdout);

    while (TRUE) {
        currentTicks = clock();
        if (currentTicks >= limitTicks) {
            break;
        }
        passes++;
        psieve = primeFinder(maxNumber);
        freeSieve(psieve);
    }

    float elapsed = (float) currentTicks - startTicks;

    Serial.printf("%s;%d;%0.1f;1;algorithm=wheel,faithful=yes,bits=1\n", title, passes, elapsed / CLOCKS_PER_SEC);
    Serial.printf("No. passes = %d\n", passes);
    fflush(stdout);
}

int cmain() {
    int secs = 5;

    if (fDebug) {
        Serial.printf("Calculate primes up to %d.\n", maxNumber);
        Serial.printf("Timer resolution: %d ticks per second.\n", (int) CLOCKS_PER_SEC);
        Serial.printf("Word size: %d bits.\n", BITS_PER_WORD);
        Serial.printf("\n");
    }

    timedTest(secs, primes8of30, "mckoss-c830");

    return (0);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  cmain();
}

void loop() {
  delay(1000);
}
