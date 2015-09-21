#include "stdio.h"
#include "string.h"

#define LINELEN 512 


int main2222(int argc, char *argv[])
{
	if (argc < 2) {
		printf("usage: analysis_ips filename [endtime]\n");
	}

	FILE * pFile;
	char line [LINELEN];

	pFile = fopen ( argv[1], "r");
	if (pFile == NULL) 
	{
		perror ("Error opening file");
		return -1;
	}

	double begin = 1.0;
	double end = 3.0;
	if (argc == 3) {
		end = atoi(argv[2])*1.0;
	}
	double period = end - begin;
	int num = 50;
	double interval = period / num;

	long line_count = 0;
	int slot = 0;
	long total[num];
	long drops[num];
	long overload[num];
	
	int i;
	for (i = 0; i < num; ++i) {
		total[i] = 0;
		drops[i] = 0;
		overload[i] = 0;
	}

	while ( fgets (line , LINELEN, pFile) != NULL )
	{
		if (line_count % 1000000 == 0) {
			printf("%d\n", line_count);
		}
		char status;
		double time;
		//line[16] = 0;

		sscanf(line, "%c %lf", &status, &time);
		//printf("%c %f\n", status, time);

		if (time == 0) {
			continue;
			printf("%s\n", line);
		}

		while (1) {
			double next = begin + (slot+1)*interval;
			
			if (time > next) {
				++slot;
				printf("--->slot: %d  next: %lf  time: %lf\n", slot, next, time);
			}
			else {
				break;
			}
		}
		
		line_count++;
		total[slot]++;
		if (status == 'd') {
			drops[slot]++;
		}

		if (status == 'r') {
			//printf("%s\n", line);
			char *str = strstr(line, "t/1/");
			if (str==NULL) {
				continue;
			}

			str = strstr(line, "MacRx");
			if (str==NULL) {
				continue;
			}

			str = strstr(line, "size=");
			if (str!=NULL) {
				char strpay[16];
				sscanf(str, "%s", strpay);
				int len = strlen(strpay);
				strpay[len-1] = 0;
				int pay = atoi(strpay+5);
				//printf("pay: %d\n", pay);
				//printf("pay: %s\n", strpay);

				overload[slot] += pay;
			}
		}
	}

	printf("total line: %d\n", line_count);
	for (i = 0; i < num; ++i) {
		//printf("%d %d\n", total[i], drops[i]);
		double edge = begin + (i+1)*interval;
		double droprate = 300.0*drops[i]/total[i];
		double ovl = overload[i]*8.0/interval/1000000;
		printf("%f\t%f\t%lf\n", edge, droprate, ovl);
	}

	fclose (pFile);
	return 0;
}
