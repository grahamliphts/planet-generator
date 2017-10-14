/* drand48.c
Random number generator between 0 and 1.
Link this program of a program requires drand48()
if there is no drand48() in stdlib.h of your OS.
*/

#define C 16807
#define A 2147483647.0
double drand48();
void srand48(long int);
double yz;

void srand48(long int seed)
{
	yz = (double)seed;
}

double drand48()
{
	long int ki;
	double uu;
	ki = (C* yz) / A;
	yz = C* yz - ki*A;
	uu = yz / (A - 1);
	return uu;
}

static unsigned long int next = 1;

int rand(void) // RAND_MAX assumed to be 32767
{
	next = next * 1103515245 + 12345;
	return (unsigned int)(next / 65536) % 32768;
}

void srand(unsigned int seed)
{
	next = seed;
}
