#include <iostream>
#include <cstdlib>
#include <sndfile.h>
#include <math.h>


using namespace std;

void bits(double* in, double* out, int count)
{
    // number of bits
    int nbBits = 10;
    unsigned mask = 0x1000;
    // number of value possible for a nbBits integer
    unsigned coeff = static_cast<unsigned>(pow(2, nbBits));
    unsigned tmp = 0;

    while( --count >= 0)
    {
	tmp = static_cast<unsigned>(*in * coeff);
	tmp |= mask;
	*out = static_cast<double>(tmp) / coeff;
	in++;
	out++;
    }
}

void delay(double* in, double* out, int count)
{
    const int DELAY = 300;
    double feedback = 0.5;
    int cursor = 0;
    double buffer[DELAY] = { 0.0 };
    // Delay 
    while(--count >= 0)
    {
	double x = *in++;
	double y = buffer[cursor];

	buffer[cursor++] = x + y * feedback;

	if (cursor >= DELAY)
	    cursor = 0;

	*out++ = y;
    }
}

double max(double* in, int count)
{
    double max = *in;
    for(int i = 0; i < count; i++)
    {
	if(in[i] > max)
	{
	    max = in[i];
	}
    }
    return max;
}

void huitbits(double* in, double* out, int count)
{
    // number of bits
    char mask = 0x08;
    // number of value possible for a nbBits integer
    char tmp = 0;

    while( --count >= 0)
    {
	
	tmp = static_cast<char>(*in * 256);
	tmp |= mask;
	*out = static_cast<double>(tmp) / 256.;
	in++;
	out++;
    }
}

void process(double *in, double *out, int count)

{
    bits(in, out, count);
    delay(in, out, count);
}




int main(int argc, char *argv[])
{
    if (argc != 3)
    {
	cerr << "Usage : " << argv[0] << " input.wav output.wav" << endl;
	return EXIT_FAILURE;
    }

    SF_INFO infos;
    SNDFILE *file = sf_open(argv[1], SFM_READ, &infos);

    if (file == NULL)
    {
	cerr << sf_strerror(file) << endl;
	return EXIT_FAILURE;
    }

    SF_INFO infosWrite;
    infosWrite.samplerate = 44100;
    infosWrite.channels = 1;
    infosWrite.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

    if (sf_format_check(&infosWrite) == 0)

    {
	cerr << "Error while checking output file format." << endl;
	return EXIT_FAILURE;
    }

    SNDFILE *fileToWrite = sf_open(argv[2], SFM_WRITE, &infosWrite);
    if (fileToWrite == NULL)
    {
	cerr << sf_strerror(fileToWrite) << endl;
	return EXIT_FAILURE;
    }


    int countFrames = infos.frames;
    double *in = new double[countFrames];
    double *out = new double[countFrames];

    sf_readf_double(file, in, countFrames);

    process(in, out, countFrames);
    sf_close(file);


    // Ecriture dans le fichier de sortie
    sf_count_t countWrite = sf_writef_double(fileToWrite, out, countFrames);
    if (countWrite != countFrames)
    {
	cerr << "Error while writing samples: " 
	     << countWrite 
	     << " written instead of " 
	     << countFrames 
	     << endl;
    }


    if (sf_close(fileToWrite) != 0)
	cerr << "Error while closing the file." << endl;


    delete[] in;
    delete[] out;


    return EXIT_SUCCESS;
}

