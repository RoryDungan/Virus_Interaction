#pragma once

#include <ctime>
#include <cstdlib>

class Random {
    public:
        static Random& getSingleton() {
            static Random mRandom;
            return mRandom;
        }

        int getRand(const int min, const int max) {
            return rand() % (max-min+1) + min; // difference between max and min + 1 + min
        }

        int getRand() {
            return rand();
		}

        void seed(unsigned int newSeed) {
            srand(newSeed);
        }

    protected:
    private:
        Random() { // Private constructor
            srand((unsigned int)time(NULL));
        }

        Random(const Random&);              // Prevent copy-construction
        Random& operator=(const Random&);   // Prevent assignment
};
