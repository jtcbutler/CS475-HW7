#include <stdio.h>
#include <stdlib.h>
#include "vector.h"
#include "banker.h"

typedef struct
ScenarioInfo
{
	int number_of_resources;
	int number_of_processes;
	int *resource_vector;
	int **demand_matrix;
	int **allocation_matrix;
}
ScenarioInfo;

ScenarioInfo parseScenario(const char *filename);
void scenarioIntegrityCheck(ScenarioInfo info);
void scenarioSafetyCheck(ScenarioInfo info);

int main(int argc, char *argv[])
{
	// TODO: attempt to open scenario file and scan data into allocated structures
	if(argc != 2)
	{
		fprintf(stderr, "Usage: %s <scenario file>\n", argv[0]);
		return EXIT_FAILURE;
	}

	ScenarioInfo info = parseScenario(argv[1]);
	scenarioIntegrityCheck(info);
	scenarioSafetyCheck(info);

	return 0;
}

ScenarioInfo parseScenario(const char *filename)
{
	FILE *file = fopen(filename, "r");
	if(file == NULL)
	{
		fprintf(stderr, "Unable to open file \"%s\"", filename);
		exit(EXIT_FAILURE);
	}

	ScenarioInfo info = (ScenarioInfo){0};

	fscanf(file, "%d", &info.number_of_resources);
	fscanf(file, "%d", &info.number_of_processes);

	info.resource_vector = malloc(sizeof(int) * info.number_of_resources);

	info.demand_matrix = malloc(sizeof(int*) * info.number_of_processes);
	info.allocation_matrix = malloc(sizeof(int*) * info.number_of_processes);

	for(int i = 0; i < info.number_of_processes; i++)
	{
		info.demand_matrix[i] = malloc(sizeof(int) * info.number_of_resources);
		info.allocation_matrix[i] = malloc(sizeof(int) * info.number_of_resources);
	}

	for(int i = 0; i < info.number_of_resources; i++)
	{
		fscanf(file, "%d", &info.resource_vector[i]);
	}

	for(int i = 0; i < info.number_of_processes; i++)
	{
		for(int j = 0; j < info.number_of_resources; j++)
		{
			fscanf(file, "%d", &info.demand_matrix[i][j]);
		}
	}

	for(int i = 0; i < info.number_of_processes; i++)
	{
		for(int j = 0; j < info.number_of_resources; j++)
		{
			fscanf(file, "%d", &info.allocation_matrix[i][j]);
		}
	}

	fclose(file);

	return info;
}

void scenarioIntegrityCheck(ScenarioInfo info)
{
	for(int i = 0; i < info.number_of_resources; i++)
	{
		int allocated = 0;

		for(int j = 0; j < info.number_of_processes; j++)
		{
			allocated += info.allocation_matrix[j][i];
		}

		if(allocated > info.resource_vector[i])
		{
			fprintf(stderr, "Integrity test failed: allocated resources exceed total resources\n");
			exit(EXIT_FAILURE);
		}
	}

	for(int i = 0; i < info.number_of_processes; i++)
	{
		for(int j = 0; j < info.number_of_resources; j++)
		{
			if(info.allocation_matrix[i][j] > info.demand_matrix[i][j])
			{
				fprintf(stderr, "Integrity test failed: allocated resources exceed demand for Thread %d\n", i);
				fprintf(stderr, "Need %d instances of resource %d\n", info.demand_matrix[i][j] - info.allocation_matrix[i][j], j);
				exit(EXIT_FAILURE);
			}
		}
	}
}

void scenarioSafetyCheck(ScenarioInfo info)
{
	int *available = malloc(sizeof(int) * info.number_of_resources);
	for(int i = 0; i < info.number_of_resources; i++)
	{
		available[i] = info.resource_vector[i];
	}

    int *complete = malloc(sizeof(int) * info.number_of_processes);
	for(int i = 0; i < info.number_of_processes; i++)
	{
		complete[i] = 0;
	}

    int proceed = 1;

    while(proceed)
	{
        proceed = 0;

        for (int i = 0; i < info.number_of_processes; i++) 
		{
            if (!complete[i]) 
			{
                int completion_possible = 1;
                for (int j = 0; j < info.number_of_resources; j++) 
				{
                    if (info.demand_matrix[i][j] > available[j]) 
					{
                        completion_possible = 0;
                        break;
                    }
                }

                if (completion_possible) 
				{
                    for (int j = 0; j < info.number_of_resources; j++) 
					{
                        available[j] += info.allocation_matrix[i][j];
                    }
                    complete[i] = 1;
                    proceed = 1;
                }
            }
        }
    }

	for(int i = 0; i < info.number_of_processes; i++)
	{
		if(!complete[i])
		{
			printf("Unsafe: ");
			for(int j = i; j < info.number_of_processes; j++)
			{
				if(!complete[j])
				{
					printf(" T%d", j);
				}
			}
			printf(" can't finish\n");
			exit(EXIT_FAILURE);
		}
	}
	printf("Safe\n");
}
