#include <drv_rng.h>
#include <rng_helper.h>

void get_rnd_string(int length, char* out)
{
	const char* _rndstr = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\0";
	int i;
	for(i=0; i<length; i++)
	{
		out[i] = _rndstr[RNG_Get_RandomRange(0, (rt_strlen(_rndstr)-1))];
	}
}

void get_rnd_bytes(int length, unsigned char* out)
{
	int i;
	for(i=0; i<length; i++)
	{
		out[i] = RNG_Get_RandomRange(0, 255);
	}
}
