//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      @ Ircam 2014
//      ModalToolBox.ccp
//      programmed by Robert Piechaud
//

#include "ModalToolbox.h"
#include "include/ffts.h"
#include "core/RealTimeEssentials.h"
#include <stdlib.h>
#include <math.h>
#include <complex>

using namespace std;

ModalToolbox::ModalToolbox()
{
    //NOTHING
}

ModalToolbox::~ModalToolbox()
{
    //NOTHING
}

void ModalToolbox::testFFT( char* wavName )
{
	printf("ModalToolbox: fft analysis\n");
    
    unsigned int fs = 16384;
    unsigned int N = 8192;
    float Ts = 0;

	FILE * wav = 0;
	long size = 0;
	long result = 0;
	char* buffer = 0;
	float* samples = 0;
    std::string fileRoot ( "plate" );
    if ( wavName )
        fileRoot = wavName;
	std::string filename ( "media/" );
    filename.append( fileRoot ).append( ".wav" );
	wav = fopen ( filename.c_str(), "r" );
	if ( !wav )
		printf( "error opening wav file '%s'!\n", filename.c_str() );
	else
	{
		fseek (wav , 0 , SEEK_END);
		size = ftell (wav);
		rewind (wav);
		char* buffer;
		buffer = (char*) malloc (sizeof(char)*size);
		result = fread ( buffer, 1, size, wav );
		if (result != size)
		{
			printf("error while reading wav file!\n");
			exit (3);
		}
		fs = *((unsigned int*)((char*)buffer+24));
		unsigned int chunkSize = *((unsigned int*)((char*)buffer+4));
        
		printf( "\tname=%s, audioformat=%u, numchannels=%u, samplerate=%u, bitspersample=%u, chunksize=%u\n",
					filename.c_str(),
					*((unsigned short*)((char*)buffer+20)),
					*((unsigned short*)((char*)buffer+22)),
					fs,
					*((unsigned short*)((char*)buffer+34)),
					(unsigned int) chunkSize );
        
		samples = (float*)((char*)buffer+44+36);
		fclose(wav);
	}

	float __attribute__ ((aligned(32))) *input = (float*)valloc(2 * N * sizeof(float));
    float __attribute__ ((aligned(32))) *output = (float*)valloc(2 * N * sizeof(float));

	float F = 55;
	for( int i = 0; i < N; ++i )
	{
		if ( samples )
			input[2*i] = *(samples+i);
		else
			input[2*i] = cos((float)2*PI*F*(i*Ts));
		input[2*i+1] = 0.0f;
	}

	if ( buffer )
		free( buffer );
	
	ffts_plan_t *p = ffts_init_1d( N, -1 );
	if( p )
	{
		ffts_execute(p, input, output);
		float max = 0;
		float mean = 0;
	
		const int oversampling = 4;
		float stepSize = N/oversampling;
		Ts = (float)1/fs;
		float freqPerBin = fs/(float) N;
		double expected = 2.*PI*(double)stepSize/(double) N;
		for( int i = 0; i < N/2; i++ )
		{
			float modX=sqrt(output[2*i]*output[2*i]+output[2*i+1]*output[2*i+1]);
			if ( max < modX )
				max = modX;
			mean += modX;
		}
		mean /= (N/2);
		printf("%d samples analyzed at %.2f kHz sample frequ. | mean amp=%.2f | max amp=%.2f | freq δ=%.2f \n",
					N, fs*1.0e-3, mean, max, (float)fs/N );
		float mod_km1 = 0;
		float mod_km2 = 0;
		double lastPhase = 0;
		float Q = 0.95;
		std::complex<float> Xkm1, Xk, Xkp1;
		mean /= max;
		for( int i = 0; i < fmin( 220, fmin(N/2, 4000*N/fs)); i++ )
		{  
			if ( i > 2 )
			{
				mod_km1 = sqrt(output[2*(i-1)]*output[2*(i-1)]+output[2*(i-1)+1]*output[2*(i-1)+1]);
				mod_km2 = sqrt(output[2*(i-2)]*output[2*(i-2)]+output[2*(i-2)+1]*output[2*(i-2)+1]);
			}
			float imag = output[2*i+1];
			float real = output[2*i];
			//Xk.im = imag;
			double mod = sqrt(real*real + imag*imag);
			double phase = atan2( (double)imag, (double)real );

			Xk = std::complex<float>(output[2*i], output[2*i+1]);
			Xkm1 = std::complex<float>(output[2*(i-1)], output[2*(i-1)+1]);
			Xkp1 = std::complex<float>(output[2*(i+1)], output[2*(i+1)+1]);
			std::complex<float> Delta = Q*(Xkm1-Xkp1)/((float)2*Xk+Xkm1+Xkp1);
			//float delta = Delta.real();
			//float trueFreq = (i+delta)*fs/N;
            float freq = (float)i*fs/N;
            float Df = fs/N;
            //float delta = (2*Df/PI)*atan( (norm(Xkp1)-norm(Xkm1))/(2*Df*(1+fmin( norm(Xk)-norm(Xkp1), norm(Xk)-norm(Xkm1) ) ) ) );
            float delta = (2*Df/PI)*atan( (100/max)*(abs(Xkp1)-abs(Xkm1))/(2*Df) );
			float trueFreq = freq + delta;
			if ( mod > mean*20*max && norm(Xkp1) < norm(Xk) && norm(Xkm1) < norm(Xk) )
			{
				printf( "\tPeak!  amp(X[%d])=%.2f, φ(X[%d])=%.2f rad, ", i, mod, i, phase);
				printf( "freq~%.2f (δ=%f,  |Xk+1|-|Xk-1|=%f, true freq=%.3f)\n", freq, delta, abs(Xkp1)-abs(Xkm1), trueFreq );
			}
		}
    printf( "\n" );
    std::string path ("data/out/fft.txt");
		FILE* graph = fopen( path.c_str(),"w");
		for ( int k = 0; k < fmin(N/2, 400); ++k )
		{
			float imag = output[2*k+1];
			float real = output[2*k];
			double mod = sqrt(real*real + imag*imag)/max;
			double phase = atan2( (double)imag, (double)real );
			fprintf( graph, "%f\n", mod*100 );
		}
		fclose( graph );
  	ffts_free(p);
	}
	else
	{
		printf("Plan unsupported\n");
	}
}

void ModalToolbox::outputFFT( float* samples, long sampleN )
{
	printf("ModalToolbox: fft output\n");
    
  unsigned int fs = 16384;
  unsigned int N = 131072;

	FILE * wav = 0;
	long size = 0;
	long result = 0;
	char* buffer = 0;

	float __attribute__ ((aligned(32))) *input = (float*)valloc(2 * N * sizeof(float));
  float __attribute__ ((aligned(32))) *output = (float*)valloc(2 * N * sizeof(float));

	float F = 55;
	for( int i = 0; i < N; ++i )
	{
		input[2*i] = *(samples+i);
		input[2*i+1] = 0.0f;
	}

	if ( buffer )
		free( buffer );
	
	ffts_plan_t *p = ffts_init_1d( N, -1 );
	if( p )
	{
		ffts_execute(p, input, output);
		float max = 0;
		float mean = 0;
	
		const int oversampling = 4;
		float stepSize = N/oversampling;
		float Ts = (float)1/fs;
		float freqPerBin = fs/(float) N;
		double expected = 2.*PI*(double)stepSize/(double) N;
		for( int i = 0; i < N/2; i++ )
		{
			float modX=sqrt(output[2*i]*output[2*i]+output[2*i+1]*output[2*i+1]);
			if ( max < modX )
				max = modX;
			mean += modX;
		}
		mean /= (N/2);
		printf("%d samples analyzed at %.2f kHz sample frequ. | mean amp=%.2f | max amp=%.2f | freq δ=%.2f \n",
					N, fs*1.0e-3, mean, max, (float)fs/N );
		float mod_km1 = 0;
		float mod_km2 = 0;
		double lastPhase = 0;
		float Q = 0.95;
		std::complex<float> Xkm1, Xk, Xkp1;
		mean /= max;
    /*
		for( int i = 0; i < fmin( 220, fmin(N/2, 4000*N/fs)); i++ )
		{  
			if ( i > 2 )
			{
				mod_km1 = sqrt(output[2*(i-1)]*output[2*(i-1)]+output[2*(i-1)+1]*output[2*(i-1)+1]);
				mod_km2 = sqrt(output[2*(i-2)]*output[2*(i-2)]+output[2*(i-2)+1]*output[2*(i-2)+1]);
			}
			float imag = output[2*i+1];
			float real = output[2*i];
			//Xk.im = imag;
			double mod = sqrt(real*real + imag*imag);
			double phase = atan2( (double)imag, (double)real );

			Xk = std::complex<float>(output[2*i], output[2*i+1]);
			Xkm1 = std::complex<float>(output[2*(i-1)], output[2*(i-1)+1]);
			Xkp1 = std::complex<float>(output[2*(i+1)], output[2*(i+1)+1]);
			std::complex<float> Delta = Q*(Xkm1-Xkp1)/((float)2*Xk+Xkm1+Xkp1);
			//float delta = Delta.real();
			//float trueFreq = (i+delta)*fs/N;
      float freq = (float)i*fs/N;
      float Df = fs/N;
      //float delta = (2*Df/PI)*atan( (norm(Xkp1)-norm(Xkm1))/(2*Df*(1+fmin( norm(Xk)-norm(Xkp1), norm(Xk)-norm(Xkm1) ) ) ) );
      float delta = (2*Df/PI)*atan( (100/max)*(abs(Xkp1)-abs(Xkm1))/(2*Df) );
			float trueFreq = freq + delta;
			if ( mod > mean*20*max && norm(Xkp1) < norm(Xk) && norm(Xkm1) < norm(Xk) )
			{
				printf( "\tPeak!  amp(X[%d])=%.2f, φ(X[%d])=%.2f rad, ", i, mod, i, phase);
				printf( "freq~%.2f (δ=%f,  |Xk+1|-|Xk-1|=%f, true freq=%.3f)\n", freq, delta, abs(Xkp1)-abs(Xkm1), trueFreq );
			}
		}
    printf( "\n" );
    */
    std::string path ("data/out/fft.txt");
    printf( "writing fft...\n" );
		FILE* graph = fopen( path.c_str(),"w");
		for ( int k = 0; k < N/2; ++k )
		{
			float imag = output[2*k+1];
			float real = output[2*k];
			float mod = sqrt(real*real + imag*imag)/max;
			//double phase = atan2( (double)imag, (double)real );
			fprintf( graph, "%f\n", mod );
		}
		fclose( graph );
  	ffts_free(p);
	}
	else
	{
		printf("Plan unsupported\n");
	}
}

