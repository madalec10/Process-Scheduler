# Process-Scheduler

The goal of this project was to create a first in first out process scheduler with high and low priority queues for both multicore and single core CPUs. I used a tick based system to keep track of time in order to move the processes along. There are 5 places a process can go in this simulation, the CPU, the SSD, the USER, the high priority queue, and the low priority queue. 

The program first reads the user specified input file. input102.txt has been provided as an example of the format. While reading the input it stores the data. Once all data is stored the simulation begins and the programs are run simultaniously on a ticking timer. Every time a process terminates the program displays the current status of all processes. Once all processes have concluded it will output all event data.
