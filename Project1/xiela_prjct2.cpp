#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <iostream>
#include <vector>
#include <cstdlib>

int	NowYear;		// 2025- 2030
int	NowMonth;		// 0 - 11

float	NowPrecip;		// inches of rain per month
float	prevPrecip;		// inches of rain per month
float	NowTemp;		// temperature this month
float	NowHeight;		// grain height in inches
int	    NowNumDeer;		// number of deer in the current population
int	    NowNumHopper;		// number of grasshoppers (thousands) in the current population

unsigned int seed = 0;

const float GRAIN_GROWS_PER_MONTH = 15.0;
const float ONE_DEER_EATS_PER_MONTH = 1.0;
const float ONE_HOPPER_EATS_PER_MONTH = 0.25;

const float AVG_PRECIP_PER_MONTH = 7.0;	// average
const float AMP_PRECIP_PER_MONTH = 6.0;	// plus or minus
const float RANDOM_PRECIP = 2.0;	// plus or minus noise

const float AVG_TEMP = 60.0;	// average
const float AMP_TEMP = 20.0;	// plus or minus
const float RANDOM_TEMP = 10.0;	// plus or minus noise

const float MIDTEMP = 40.0;
const float MIDPRECIP = 10.0;

omp_lock_t	    Lock;
volatile int	NumInThreadTeam;
volatile int	NumAtBarrier;
volatile int	NumGone;

void	InitBarrier(int);
void	WaitBarrier();


void
InitBarrier(int n)
{
    NumInThreadTeam = n;
    NumAtBarrier = 0;
    omp_init_lock(&Lock);
}


// have the calling thread wait here until all the other threads catch up:

void
WaitBarrier()
{
    omp_set_lock(&Lock);
    {
        NumAtBarrier++;
        if (NumAtBarrier == NumInThreadTeam)
        {
            NumGone = 0;
            NumAtBarrier = 0;
            // let all other threads get back to what they were doing
// before this one unlocks, knowing that they might immediately
// call WaitBarrier( ) again:
            while (NumGone != NumInThreadTeam - 1);
            omp_unset_lock(&Lock);
            return;
        }
    }
    omp_unset_lock(&Lock);

    while (NumAtBarrier != 0);	// this waits for the nth thread to arrive

#pragma omp atomic
    NumGone++;			// this flags how many threads have returned
}

float SQR(float x) {
    return x * x;
}

float
Ranf(float low, float high)
{
    float r = (float)rand();               // 0 - RAND_MAX
    float t = r / (float)RAND_MAX;       // 0. - 1.

    return   low + t * (high - low);
}

void
Environment() // calculates environmental parameters such as temp and precip
{
    if (NowMonth == 12)
    {
        NowYear++;
        NowMonth = 0;
    }
    float ang = (30. * (float)NowMonth + 15.) * (M_PI / 180.);	// angle of earth around the sun

    float temp = AVG_TEMP - AMP_TEMP * cos(ang);
    NowTemp = temp + Ranf(-RANDOM_TEMP, RANDOM_TEMP);

    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin(ang);
    prevPrecip = NowPrecip;
    NowPrecip = precip + Ranf(-RANDOM_PRECIP, RANDOM_PRECIP);
    if (NowPrecip < 0.)
        NowPrecip = 0.;
}

void
Deer()
{
    while (NowYear < 2031)
    {
        int nextNumDeer = NowNumDeer;
        int carryingCapacity = (int)(NowHeight);
        if (nextNumDeer < carryingCapacity)
            nextNumDeer++;
        else
            if (nextNumDeer > carryingCapacity)
                nextNumDeer--;

        if (nextNumDeer < 0)
            nextNumDeer = 0;

        WaitBarrier();
        NowNumDeer = nextNumDeer;

        WaitBarrier();

        WaitBarrier();
    }
    return;
}

void
GrainGrowth()
{
    while (NowYear < 2031)
    {
        
        float tempFactor = exp(-SQR((NowTemp - MIDTEMP) / 10.));

        float precipFactor = exp(-SQR((NowPrecip - MIDPRECIP) / 10.));

        float nextHeight = NowHeight;
        nextHeight += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
        nextHeight -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;
        nextHeight -= (float)NowNumHopper * ONE_HOPPER_EATS_PER_MONTH;
        if (nextHeight < 0.) 
            nextHeight = 0.;
        WaitBarrier();
        NowHeight = nextHeight;

        WaitBarrier();

        WaitBarrier();
    }
    return;
}

void
Watcher()
{
    while (NowYear < 2031)
    {
   
        WaitBarrier();
        
        WaitBarrier();
        
        //  print the current set of global state variables, 
        fprintf(stderr, "%2d , %2d , %6.2lf , %6.2lf ,%6.2lf , %2d%, %2d%\n", NowMonth + 1, NowYear, NowTemp, NowPrecip, NowHeight, NowNumDeer, NowNumHopper);
        
        NowMonth++;// increment the month count, and then 
        Environment();// use the new month to compute the new Temperature and Precipitation
        
        WaitBarrier();
    
    }
    return;
}

void
MyAgent() // Grasshoppers

{
    while (NowYear < 2031)
    {
        int nextNumHopper = NowNumHopper;
        int carryingCapacity = (int)(NowHeight);
        float nextHeight = NowHeight;

        // Grasshoppers hatch with warm temps and die with cold temps
        if (NowTemp > 65)
            nextNumHopper+= 5;
        
        if (NowTemp < 50)
            nextNumHopper /= 2;
            if (NowTemp < 30)
                nextNumHopper/= 3;

        // Ideal hatching conditions, warm temps and moisture followed by dry soil
        if (NowTemp > 70 && NowPrecip < prevPrecip) 
            nextNumHopper += 3;

        // Grasshoppers will swarm, ie eat more voraciously, if food is scarce and if population is high
        if (nextNumHopper < carryingCapacity)
            if (nextNumHopper > 10)
                nextHeight -= nextNumHopper / 2; 
            nextNumHopper -= 3; // they will also begin dying off because of lack of food

        if (nextNumHopper < 0)
            nextNumHopper = 0;

        NowHeight = nextHeight;

        WaitBarrier();
        NowNumHopper = nextNumHopper;        

        // DoneAssigning barrier:
        WaitBarrier();

        // DonePrinting barrier:
        WaitBarrier();
    }
    return;
}


int
main(int argc, char* argv[])
{
    // starting date and time:
    NowMonth = 0;
    NowYear = 2025;

    // starting state (feel free to change this if you want):
    NowNumDeer = 2;
    NowHeight = 5.;
    NowNumHopper = 0;

    Environment();

    omp_set_num_threads(4);	// same as # of sections
    InitBarrier(4);		// or 4
#pragma omp parallel sections
    {
#pragma omp section
        {
            Deer();
        }

#pragma omp section
        {
            GrainGrowth();
        }

#pragma omp section
        {
            MyAgent();	// your own
        }

#pragma omp section
        {
            Watcher();
        }
    }       // implied barrier -- all functions must return in order
        // to allow any of them to get past here
    return 0;
}
// specify how many threads will be in the barrier:
//	(also init's the Lock)