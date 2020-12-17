#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <math.h>
#include <cstdlib>
#include <stdio.h>
#include <vector>
#include "MiniPID.h"

using namespace std;

int main()
{

	MiniPID pid=MiniPID(0,0,0);
	//condition
	double output = 0;
	double setpoint = 100;
	double p,i,d;

	//params for gentic algorithm
	int num_gene = 4;
	int max_iter = 100;
	int iter=0;
	int division_ptr;
	double mutation_ratio=0.1;

	//params for crossover
	double temp_set[num_gene][3];
	double temp_gain[2][3];


	//params for PID
	//actucal rise_time, overshoot, std_error
	int rise_time;
	double overshoot;
	double result[num_gene][3];
	int end_index;

	//target rise_time, overshoot, std_error
	int ref_rise_time=5;
	double ref_overshoot=5;
	double ref_std_error=1;

	//if error keep "count" cycle finish pid getOutput
	int finish_count=5;
	int end_count=0;
	int iter_pid=0;
	int max_iter_pid=1000;



	//Run genetic Algorithm
	//Step 1: gen gentic set
	double gain_set[num_gene][3];
	srand((unsigned int) time(NULL));
	for(int k=0; k< num_gene; k++)
	{
		p=(rand() %100)/100.0;
		i=(rand() %100+1)/100.0;
		d=(rand() %100)/1000.0;

		gain_set[k][0]=p;
		gain_set[k][1]=i;
		gain_set[k][2]=d;
	}

	printf("Condition\n");
	printf("Rising time: %d\n", ref_rise_time);
	printf("Overshooting time: %f\n", ref_overshoot);

	printf("Start Genetic Algorithm\n");
	while(iter<max_iter)
	{
		iter++;
		printf("Iteration: %d/%d\n",iter,max_iter);
		//Step 2: Run PID for gain_set
		for(int n=0; n<num_gene;n++)
		{
			//Run PID
			p=gain_set[n][0];
			i=gain_set[n][1];
			d=gain_set[n][2];

			pid.setPID(p,i,d);

			iter_pid=0;
			rise_time=0;
			overshoot=0;
			end_count=0;

			while(end_count!=finish_count && iter_pid < max_iter_pid)
			{
				iter_pid++;

				output = pid.getOutput(output, setpoint);


				if((output-setpoint)>overshoot)
					overshoot = output-setpoint;

				if(fabs(output-setpoint)<=ref_std_error)
				{
					end_count++;
					if(end_count!=finish_count)
						rise_time=iter_pid+1-finish_count;
				}
			}
			result[n][0]= rise_time;
			result[n][1]= overshoot;
			result[n][2]= 0;
		}


		//Step 3: check end condition
		for(end_index=0; end_index<num_gene;end_index++)
		{
			if(result[end_index][0] != 0 && result[end_index][0] <= ref_rise_time && result[end_index][1] <= ref_overshoot)
				break;
		}

		//Step 4: compute fitness
		double sum_rise=0;
		double sum_over=0;
		for(int n=0; n<num_gene;n++)
		{
			sum_rise += result[n][0];
			sum_over += result[n][1];
		}

		for(int n=0; n<num_gene;n++)
		{
			if(result[n][0] != 0)
			{
				result[n][2] = result[n][0]/sum_rise + result[n][1]/sum_over;
			}
		}

		//Escape while
		if(end_index < num_gene)
			break;

		//Step 5: change gene
		//crossover
		double ratio;
		double min, max;
		double first;
		double second;
		for(int n=0; n<num_gene;n++)
		{
			//First
			ratio=(rand() %200)/100.0;
			min=0;
			max=0;
			for(int k=0; k<num_gene;k++)
			{
				max=max+result[k][2];
				if(min<=ratio && max>ratio)
				{
					first = k;
					temp_gain[0][0]=gain_set[k][0];
					temp_gain[0][1]=gain_set[k][1];
					temp_gain[0][2]=gain_set[k][2];
				}
				min=max;
			}

			//Second
			second=first;
			while(first==second)
			{
				second=(rand() %num_gene);
				temp_gain[1][0]=gain_set[int(second)][0];
				temp_gain[1][1]=gain_set[int(second)][1];
				temp_gain[1][2]=gain_set[int(second)][2];
			}
			/*
			second=first;
			while(first==second)
			{
				ratio=(rand() %200)/100.0;
				min=0;
				max=0;
				for(int k=0; k<num_gene;k++)
				{
					max=max+result[k][2];
					if(min<=ratio && max>ratio)
					{
						second=k;
						temp_gain[1][0]=gain_set[k][0];
						temp_gain[1][1]=gain_set[k][1];
						temp_gain[1][2]=gain_set[k][2];
					}
					min=max;
				}
			}
			*/
			
			division_ptr= rand()%2;
			
			
			if(result[int(first)][2]>=result[int(second)][2])
			{
				temp_set[n][0] = temp_gain[0][0];
				temp_set[n][2] = temp_gain[1][2];
			}
			else
			{
				temp_set[n][0] = temp_gain[1][0];
				temp_set[n][2] = temp_gain[0][2];
			}

			temp_set[n][1] = division_ptr<1 ? temp_gain[0][1] : temp_gain[1][1];

		}

		//Mutation
		for(int n=0; n<num_gene;n++)
		{
			p=(rand() %100)/100.0;
			i=(rand() %100+1)/100.0;
			d=(rand() %100)/1000.0;

			ratio=(rand() %100)/100.0;
			if(ratio<=mutation_ratio)
			{
				gain_set[n][0]=p;
				gain_set[n][1]=i;
				gain_set[n][2]=d;
			}
			else
			{
				gain_set[n][0]=temp_set[n][0];
				gain_set[n][1]=temp_set[n][1];
				gain_set[n][2]=temp_set[n][2];
			}

		}



	}

	if(end_index<num_gene)
	{
		printf("Test Successful!\n");
		printf("Genetic algorithm iteration: %d\n",iter);
		printf("PID gain is %f %f %f\n",gain_set[end_index][0],gain_set[end_index][1],gain_set[end_index][2]);
		printf("Rising time: %f\n", result[end_index][0]);
		printf("Overshooting time: %f\n",result[end_index][1]);
	}
	else
	{
		printf("Fail to find PID gain\n");
		printf("Need for iteration\n");
	}
}
