//parses the input file into Process and event
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdlib.h>

#define _CRT_SECURE_NO_WARNINGS

/*

CHANGE fp1 AND fp2 IN BOTH PARSER() AND MAIN() ACCORDING TO
THE INPUT FILE THAT YOU WANT TO USE

To Do:
Once a process goes into blocked state and it meets the threshold, swap out that
same process [DONE]

When a process exits/terminates, swap back in the latest process
Use a circular queue (FIFO) to determine which process is swapped back in [DONE]

Will need a blocked tracker, keeping track of the number of blocked processes
in relation to the total processes to check if it hit the threshold [DONE]

Keep track of overhead latency, every time a process is swapped in/out it adds a fixed latency
Bus is only wide enough to swap in/out one process, every time two processes is swapped it doubles the fixed latency [DONE]

Simulation should choose the most optimal combination (80%, 90%, 100%) and (1 or 2 swapped processes) based on the least latency. [DONE]

Design an input file that swaps in/out processes multiple times [DONE]

Results:
Swap out/in is working
Overall latency is working

*/

struct Process {
	char pid[8];
	char state[30];
	char queue[15];
};

char temp_arr[20][5];


void parser() {
	int i;
	char* rch;
	char str[300];
	char LineInFile[40][300];
	int lineP, lineQ;
	char* sch;
	char tokenizedLine[10][10];

	FILE* fp1;
	FILE* fp2;
	fp1 = fopen("inp2.txt", "r");			//open the original input file
	fp2 = fopen("inp2_parsed.txt", "w");	//output the Process ID and event to another file. 
	//You can store in variables instead of printing to file

	lineP = 0;
	i = 0;

	printf("Started parsing...\n");

	//copy first line of the original file to the new file
	if (fgets(str, sizeof(str), fp1) != NULL)
		fprintf(fp2, "%s", str);

	//parse each remaining line into Process event
	//while loop with fgets reads each line
	while (fgets(str, sizeof(str), fp1) != NULL)
	{
		lineP = 0;
		rch = strtok(str, ":;.");					// use strtok to break up the line by : or . or ; This would separate each line into the different events
		while (rch != NULL)
		{
			strcpy(LineInFile[lineP], rch);			//copy the events into an array of strings
			lineP++;								//keep track of how many events are in that line
			rch = strtok(NULL, ":;.");				//needed for strtok to continue in the while loop
		}


		//for each event (e.g. Time slice for P7 expires) pull out process number and event
		for (i = 1; i < lineP - 1; i++)
		{
			lineQ = 0;
			sch = strtok(LineInFile[i], " ");
			while (sch != NULL)
			{
				strcpy(tokenizedLine[lineQ], sch);		//use strtok to break up each line into separate words and put the words in the array of strings
				lineQ++;								//count number of valid elements
				sch = strtok(NULL, " ");
			}

			//tokenizedLine has the event separated by spaces (e.g. Time slice for P7 expires)
			if (strcmp(tokenizedLine[1], "requests") == 0)						//Process requests an I/O device
			{

				fprintf(fp2, "%s %s %s ", tokenizedLine[0], tokenizedLine[1], tokenizedLine[3]);
				//fprintf(fp2, "%s %s %s ", tokenizedLine[0], tokenizedLine[1], tokenizedLine[3]);
			}
			else if ((strcmp(tokenizedLine[2], "dispatched") == 0))				//Process is dispatched
			{
				fprintf(fp2, "%s %s ", tokenizedLine[0], tokenizedLine[2]);
			}
			else if (strcmp(tokenizedLine[0], "Time") == 0)						//Process has timed off
			{
				fprintf(fp2, "%s %s ", tokenizedLine[3], tokenizedLine[4]);
			}
			else if (strcmp(tokenizedLine[3], "out") == 0)						//Process is swapped out
			{
				fprintf(fp2, "%s %s ", tokenizedLine[0], tokenizedLine[3]);
			}
			else if (strcmp(tokenizedLine[3], "in") == 0)						//Process is swapped in
			{
				fprintf(fp2, "%s %s ", tokenizedLine[0], tokenizedLine[3]);
			}
			else if (strcmp(tokenizedLine[1], "interrupt") == 0)				//An interrupt has occured
			{
				fprintf(fp2, "%s %s ", tokenizedLine[4], tokenizedLine[1]);
			}
			else																//Process has been terminated
			{
				fprintf(fp2, "%s %s ", tokenizedLine[0], tokenizedLine[2]);
			}

		}
		fprintf(fp2, "\n");
	}

	printf("Parsing complete\n\n");

	fclose(fp1);
	fclose(fp2);

}

void queue_parser(char* queue) {
	char queue_cp[100];
	strcpy(queue_cp, queue);
	char* temp_parse = strtok(queue_cp, " ");
	int k = 0;
	// clean out temp_arr each time queue parser is called

	for (int j = 0; j < 20; j++) {
		strcpy(temp_arr[j], "");
	}
	while (temp_parse != NULL)
	{
		strcpy(temp_arr[k], temp_parse);		//use strtok to break up each line into separate words and put the words in the array of strings
		k++;									//count number of valid elements
		temp_parse = strtok(NULL, " ");
	}
}

int main()
{

	parser(); // run parser and stored parsed information in inp2_parsed.txt
	int swap_cap = 0;
	float usr_threshold = 0.0;
	
	printf("Choose the swapping threshold (0.8, 0.9, 1.0): ");
	
	// Ask the user to choose the threshold
	scanf("%f", &usr_threshold);
	
	printf("Choose the swapping capacity (1 or 2): ");
	scanf("%d", &swap_cap);
	printf("First input: %.1f\n", usr_threshold);
	printf("Second input: %d\n", swap_cap);

	srand(time(NULL));  // Random num seed
	FILE* fp1;
	FILE* fp2;
	fp1 = fopen("inp2.txt", "r");			//open the original input file
	fp2 = fopen("inp2_parsed.txt", "r");	//output the Process ID and event to another file. 
	char str[300];
	char str1[300];
	const char delim[] = " ";
	int curr_processes[20];
	int blocked_circular_queue[20]; for (int i = 0; i < 20; i++) { blocked_circular_queue[i] = -1; }	// keep track of which process to swap in
	int blocked_q_rear = 0;						// keep track of q index to update
	int blocked_q_front = 0;					// keep track of q index to swap in
	int total_processes = 0;					// # of processes
	int blocked_processes = 0;					// keep track of # of blocked processes
	int process_threshold;
	int first = 0;
	int overall_latency = 0;					// keep track of overall latency
	int rand_process_swap_ind = 0;					// random process to swap out if swap capacity is 2

	struct Process processes[20];			// Create an array of 20 processes
	// dynamically allocate memory for each q in case it overflows
	char* printer_q = (char*)malloc(10 * sizeof(char));
	char* disk_q = (char*)malloc(10 * sizeof(char));
	char* kb_q = (char*)malloc(10 * sizeof(char));

	strcpy(disk_q, "");						// initialize memory location for queues
	strcpy(printer_q, "");
	strcpy(kb_q, "");

	int prev_id[20];
	int prev_track = 0;

	for (int i = 0; i < 20; i++) {			// Populate each process with respective ID and Ready state and default values for prev_id
		char str[3];
		char str1[5] = "P";
		sprintf(str, "%d", i + 1);
		strcat(str1, str);
		strcpy(processes[i].pid, str1);
		strcpy(processes[i].queue, "");
		prev_id[i] = 100;
	}
	while (fgets(str, sizeof(str), fp1) != NULL)	// loops through the file and retrieves each line
	{
		printf("%s\n", str);	// prints the line
		fgets(str1, sizeof(str1), fp2);	// gets the first line of parsed file
		char* token = strtok(str1, delim);
		char* token_next;
		while (token != NULL)			// separate lines of parsed file by spaces; token is the tokenized parts of the string
		{
			if (first == 0) {			// check if first line, store process info
				char temp[300];
				strcpy(temp, str);
				char* temp_cp = strtok(temp, delim);
				int i = 0;
				while (strcmp(temp_cp, "end\n") != 0) {
					if (temp_cp[0] == 'P') {
						curr_processes[i] = atoi(temp_cp + 1);
						i++;
					}
					else {
						strcpy(processes[curr_processes[i - 1] - 1].state, temp_cp);
						if (strcmp(temp_cp, "New") != 0) {
							total_processes = total_processes + 1;		// Add processes to total if the state is not "New"
						}
						if (strcmp(temp_cp, "Blocked") == 0) {
							blocked_processes = blocked_processes + 1;
						}
					}
					temp_cp = strtok(NULL, delim);
				}
				process_threshold = total_processes * usr_threshold; // determine the # of processes for threshold
				break;
			}
			//printf("%s\n", token);
			if (token[0] == 'P') {
				token_next = strtok(NULL, delim);
				//printf("The operation for %s is %s\n", token, token_next); // states the operation for current process
				int temp_id;
				if (strlen(token) > 2) {
					char temp_str[5];
					strcpy(temp_str, token + 1);
					temp_id = atoi(temp_str);
				}
				else {
					temp_id = token[1] - '0';
				}
				//printf("token_next is (%s)\n", token_next);
				if (strcmp(token_next, "requests") == 0) {				   // compares value at memory addr of token_next with corresponding state
					token_next = strtok(NULL, delim);
					blocked_processes = blocked_processes + 1;
					if (strcmp(token_next, "disk") == 0) {
						strcpy(processes[temp_id - 1].queue, "disk");
						if (strcmp(disk_q, "") != 0) {
							strcat(disk_q, " ");
							strcat(disk_q, token);
						}
						else {
							strcat(disk_q, token);
						}
					}
					else if (strcmp(token_next, "printer") == 0) {
						strcpy(processes[temp_id - 1].queue, "printer");
						if (strcmp(printer_q, "") != 0) {
							strcat(printer_q, " ");
							strcat(printer_q, token);
						}
						else {
							strcat(printer_q, token);
						}
					}
					else {
						strcpy(processes[temp_id - 1].queue, "keyboard");
						if (strcmp(kb_q, "") != 0) {
							strcat(kb_q, " ");
							strcat(kb_q, token);
						}
						else {
							strcat(kb_q, token);
						}
					}
					strcpy(processes[temp_id - 1].state, "Blocked*");

					// Check if process needs to be swapped out
					if (blocked_processes >= process_threshold) {
						strcpy(processes[temp_id - 1].state, "Blocked/Suspended*");	// Suspend blocked state
						// Store in process id in circular q and keep track of next index
						blocked_circular_queue[blocked_q_rear] = temp_id - 1;
						blocked_q_rear = blocked_q_rear + 1;
						// Update overall latency
						overall_latency = overall_latency + 1;
						if (swap_cap == 2) {
							// If swap capacity is 2 choose a random Blocked Process to swap out
							rand_process_swap_ind = rand() % 20;
							while (strcmp(processes[rand_process_swap_ind].state, "Blocked") != 0) {
								rand_process_swap_ind = rand() % 20;	// keep generating random number until Blocked process is found
							}
							strcpy(processes[rand_process_swap_ind].state, "Blocked/Suspended*");	// Suspend blocked state
							// Store in process id in circular q and keep track of next index
							blocked_circular_queue[blocked_q_rear] = rand_process_swap_ind;
							blocked_q_rear = blocked_q_rear + 1;
							// Update overall latency
							overall_latency = overall_latency + 1;
						}
					}
					prev_id[prev_track] = temp_id - 1;
					prev_track++;
				}
				else if (strcmp(token_next, "admitted") == 0) {
					// Check if process needs to be swapped out
					if (blocked_processes == process_threshold) {
						strcpy(processes[temp_id - 1].state, "Ready/Suspended*");	// Suspend blocked state
						// Store in process id in circular q and keep track of next index
						blocked_circular_queue[blocked_q_rear] = temp_id - 1;
						blocked_q_rear = blocked_q_rear + 1;
						// Update overall latency
						overall_latency = overall_latency + 1;
						if (swap_cap == 2) {
							// If swap capacity is 2 choose a random Blocked Process to swap out
							rand_process_swap_ind = rand() % 20;
							while (strcmp(processes[rand_process_swap_ind].state, "Blocked") != 0) {
								rand_process_swap_ind = rand() % 20;	// keep generating random number until Blocked process is found
							}
							strcpy(processes[rand_process_swap_ind].state, "Blocked/Suspended*");	// Suspend blocked state
							// Store in process id in circular q and keep track of next index
							blocked_circular_queue[blocked_q_rear] = rand_process_swap_ind;
							blocked_q_rear = blocked_q_rear + 1;
							// Update overall latency
							overall_latency = overall_latency + 1;
						}
					}
					else {
						strcpy(processes[temp_id - 1].state, "Ready*");
					}
					// Update total processes and new process threshold
					total_processes = total_processes + 1;
					process_threshold = total_processes * usr_threshold;
					prev_id[prev_track] = temp_id - 1;
					prev_track++;
				}
				else if (strcmp(token_next, "dispatched") == 0) {
					strcpy(processes[temp_id - 1].state, "Running*");
					prev_id[prev_track] = temp_id - 1;
					prev_track++;
				}
				else if (strcmp(token_next, "expires") == 0) {
					strcpy(processes[temp_id - 1].state, "Ready*");
					prev_id[prev_track] = temp_id - 1;
					prev_track++;
				}
				else if (strcmp(token_next, "terminated") == 0) {
					strcpy(processes[temp_id - 1].state, "Exit*");
					prev_id[prev_track] = temp_id - 1;
					prev_track++;

					// Update total processes and new process threshold
					total_processes = total_processes - 1;
					process_threshold = usr_threshold * total_processes;
					// Check if valid index
					if (blocked_circular_queue[blocked_q_front] != -1) {
						// Swap back in latest process
						for (int i = 0; i < swap_cap; i++) {
							size_t temp_size = strlen(processes[blocked_circular_queue[blocked_q_front]].state);
							if (processes[blocked_circular_queue[blocked_q_front]].state[temp_size - 1] == '*') {
								processes[blocked_circular_queue[blocked_q_front]].state[temp_size - 11] = '\0';
							}
							else {
								processes[blocked_circular_queue[blocked_q_front]].state[temp_size - 10] = '\0';
							}
							strcat(processes[blocked_circular_queue[blocked_q_front]].state, "*");
							prev_id[prev_track] = blocked_circular_queue[blocked_q_front];
							prev_track++;
							blocked_circular_queue[blocked_q_front] = -1;			// update index to be read already
							blocked_q_front = blocked_q_front + 1;					// update front index
							// Update overall latency
							overall_latency = overall_latency + 1;
						}
					}
				}
				/*
				else if (strcmp(token_next, "out") == 0) {
					// check if running
					if ((strcmp(processes[temp_id - 1].state, "Running") == 0) || (strcmp(processes[temp_id - 1].state, "Running*") == 0)) {
						strcpy(processes[temp_id - 1].state, "Ready/Suspended*");
						prev_id[prev_track] = temp_id - 1;
						prev_track++;
					}
					else {
						strcat(processes[temp_id - 1].state, "/Suspended*");	// append Suspended state to end of state
						prev_id[prev_track] = temp_id - 1;
						prev_track++;
					}
				}

				else if (strcmp(token_next, "in") == 0) {
					size_t temp_size = strlen(processes[temp_id - 1].state);
					if (processes[temp_id - 1].state[temp_size-1] == '*') {
						processes[temp_id - 1].state[temp_size - 11] = '\0';
					}
					else {
						processes[temp_id - 1].state[temp_size - 10] = '\0';
					}
					strcat(processes[temp_id - 1].state, "*");
					prev_id[prev_track] = temp_id - 1;
					prev_track++;
				}
				*/
				else if (strcmp(token_next, "interrupt") == 0) {
					// Update amount of blocked processes
					blocked_processes = blocked_processes - 1;
					if (strcmp(processes[temp_id - 1].state, "Blocked") == 0) {
						strcpy(processes[temp_id - 1].state, "Ready*");
						if (strcmp(processes[temp_id - 1].queue, "disk") == 0) {
							if (strcmp(processes[temp_id - 1].pid, disk_q) == 0) {
								disk_q[0] = '\0';
								prev_id[prev_track] = temp_id - 1;
								prev_track++;
								token = strtok(NULL, delim);
								continue;
							}
							int i = 0;
							queue_parser(disk_q);
							while (strcmp(temp_arr[i], "") != 0) {
								if (strcmp(temp_arr[i], processes[temp_id - 1].pid) == 0) {
									// Code to take out PID from queue
									// Assuming LIFO, delete from top of the stack
									size_t temp_size = strlen(disk_q);
									disk_q[temp_size - 3] = '\0';
								}
								i++;
							}
						}
						else if (strcmp(processes[temp_id - 1].queue, "printer") == 0) {
							if (strcmp(processes[temp_id - 1].pid, printer_q) == 0) {
								printer_q[0] = '\0';
								prev_id[prev_track] = temp_id - 1;
								prev_track++;
								token = strtok(NULL, delim);
								continue;
							}
							int i = 0;
							queue_parser(printer_q);
							while (strcmp(temp_arr[i], "") != 0) {
								if (strcmp(temp_arr[i], processes[temp_id - 1].pid) == 0) {
									// Code to take out PID from queue
									// Assuming LIFO, delete from top of the stack
									size_t temp_size = strlen(printer_q);
									printer_q[temp_size - 3] = '\0';
								}
								i++;
							}
						}
						else if (strcmp(processes[temp_id - 1].queue, "keyboard") == 0) {
							if (strcmp(processes[temp_id - 1].pid, kb_q) == 0) {
								kb_q[0] = '\0';
								prev_id[prev_track] = temp_id - 1;
								prev_track++;
								token = strtok(NULL, delim);
								continue;
							}
							int i = 0;
							queue_parser(kb_q);
							while (strcmp(temp_arr[i], "") != 0) {
								if (strcmp(temp_arr[i], processes[temp_id - 1].pid) == 0) {
									// Code to take out PID from queue
									// Assuming LIFO, delete from top of the stack
									size_t temp_size = strlen(kb_q);
									kb_q[temp_size - 3] = '\0';
								}
								i++;
							}
						}
					}
					// do the same for Blocked/Suspended state as well
					else if (strcmp(processes[temp_id - 1].state, "Blocked/Suspended") == 0) {
						strcpy(processes[temp_id - 1].state, "Ready/Suspended*");
						if (strcmp(processes[temp_id - 1].queue, "disk") == 0) {
							if (strcmp(processes[temp_id - 1].pid, disk_q) == 0) {
								disk_q[0] = '\0';
								prev_id[prev_track] = temp_id - 1;
								prev_track++;
								token = strtok(NULL, delim);
								continue;
							}
							int i = 0;
							queue_parser(disk_q);
							while (strcmp(temp_arr[i], "") != 0) {
								if (strcmp(temp_arr[i], processes[temp_id - 1].pid) == 0) {
									// Code to take out PID from queue
									// Assuming LIFO, delete from top of the stack
									size_t temp_size = strlen(disk_q);
									disk_q[temp_size - 3] = '\0';
								}
								i++;
							}
						}
						else if (strcmp(processes[temp_id - 1].queue, "printer") == 0) {
							if (strcmp(processes[temp_id - 1].pid, printer_q) == 0) {
								printer_q[0] = '\0';
								prev_id[prev_track] = temp_id - 1;
								prev_track++;
								token = strtok(NULL, delim);
								continue;
							}
							int i = 0;
							queue_parser(printer_q);
							while (strcmp(temp_arr[i], "") != 0) {
								if (strcmp(temp_arr[i], processes[temp_id - 1].pid) == 0) {
									// Code to take out PID from queue
									// Assuming LIFO, delete from top of the stack
									size_t temp_size = strlen(printer_q);
									printer_q[temp_size - 3] = '\0';
								}
								i++;
							}
						}
						else if (strcmp(processes[temp_id - 1].queue, "keyboard") == 0) {
							if (strcmp(processes[temp_id - 1].pid, kb_q) == 0) {
								kb_q[0] = '\0';
								prev_id[prev_track] = temp_id - 1;
								prev_track++;
								token = strtok(NULL, delim);
								continue;
							}
							int i = 0;
							queue_parser(kb_q);
							while (strcmp(temp_arr[i], "") != 0) {
								if (strcmp(temp_arr[i], processes[temp_id - 1].pid) == 0) {
									// Code to take out PID from queue
									// Assuming LIFO, delete from top of the stack
									size_t temp_size = strlen(kb_q);
									kb_q[temp_size - 3] = '\0';
								}
								i++;
							}
						}
					}
					prev_id[prev_track] = temp_id - 1;
					prev_track++;
				}

			}
			token = strtok(NULL, delim);
		}
		if (first == 0) {
			first = 1;
			continue;
		}
		printf("Current states are: "); // Prints the current states
		for (int i = 0; i < 20; i++) {
			if (curr_processes[i] < -1) {
				break;
			}

			printf("%s %s ", processes[curr_processes[i] - 1].pid, processes[curr_processes[i] - 1].state);
		}
		printf("\n");
		printf("The total processes in memory is %d\n", total_processes);
		printf("The blocked processes are %d\n", blocked_processes);
		printf("The process threshold is %d\n", process_threshold);
		printf("\n");
		printf("disk queue: %s\n", disk_q);
		printf("printer queue: %s\n", printer_q);
		printf("keyboard queue: %s\n", kb_q);
		printf("\n");
		for (int i = 0; i < 20; i++) {		// searches for previous updates and takes out '*'
			if (prev_id[i] != 100) {
				size_t temp_size = strlen(processes[prev_id[i]].state);
				processes[prev_id[i]].state[temp_size - 1] = '\0';	// use end null character to cut off the last character
				prev_id[i] = 100;
			}
		}
		prev_track = 0;
	}
	printf("The overall latency for this trial is %d\n", overall_latency);
	return 0;
}