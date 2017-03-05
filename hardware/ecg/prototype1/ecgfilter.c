#include <stdio.h>

main (int argc, char **argv) {

	int i, last_qrs=0;
	double ch1,ch2;

	// V low freq LPF (tracks drift over multiple hb)
	double lpf=0;

	// LPF to reduce 50Hz mains noise.
	double lpf2=0;

	// Top and bottom envelope
	double et=-1,eb=+1;

	double pch1,dch1,r;

	// Heart rate (bpm)
	double hr;

	// Heart period (samples)
	double hp=500;

	double hr_lpf=60;

	double hp_lpf=500;

	fscanf (stdin, "%lf %lf", &ch1,&ch2);

	lpf=lpf2=pch1=ch1;


	while (!feof(stdin)) {

		fscanf (stdin, "%lf %lf", &ch1,&ch2);

		lpf += (ch1-lpf)/256;
		lpf2 += (ch1-lpf2)/16;

		if (lpf2>et) {
			et = lpf2;
		} else {
			et -= (et-lpf)/2048;
		}

		if (lpf2 < eb) {
			eb = lpf2;
		} else {
			eb -= (eb-lpf)/256;
		}


		if ( (i - last_qrs) > (hp_lpf*0.7) ) {
			r = (lpf2-lpf) / (et-lpf);
			if (r > 0.7) {

				if ((i-last_qrs)<(hp_lpf*1.3)) {
					hr = 60.0 / ( (double)(i-last_qrs)/500.0);
					hr_lpf += (hr - hr_lpf)/16;
					hp_lpf += (hp - hp_lpf)/16;
				}
				last_qrs=i;
			}
		}


		dch1 = ch1-pch1;
		pch1 = ch1;

		fprintf(stdout, "%d %f %f %f %f %f %f %f %f %f\n", i, ch1, lpf, lpf2, et, eb, dch1, r, hr, hr_lpf );

		i++;

	}
}

