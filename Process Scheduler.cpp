#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <queue>


using namespace std;

struct input{
    string command = ""; //process command
    int time = 0; //process time
    bool active;

    //for initializing inputs
    input(string command, int time): command(command), time(time), active(false) {}
};

struct process{
    int ID; //process ID number
    int timeStart; //time process starts executing
    int idx; // tracks the index of the instructions vector
    int pTime; // tracks time of current running process
    int CPUTime; // tracks total time at CPU
    int USERInteracts; // tracks number of USER interactions
    int SSDAccesses; // tracks SSD acesses
    string pCommand; //stores previous command
    vector<input> instructions; //list of instructions in order
    bool complete; //true if process is complete, otherwise false
    bool started; // true if process has begun, otherwise false
    bool ready; // tracks ready state
    bool blocked; //tracks blocked state
    bool running; //tracks running state
    bool queued; //tracks if process is in queue
};

//function declarations
//reads input
void readInput(vector<process> &processes, int &nprocesses, int &ncores);
//outputs list of commands stored
void outputProcesses(vector<process> processes);
//attempts to empty the high and low priority queue processes into the CPU if there is an available core
void emptyQueues(vector<process> &processes, int &availableCores, queue<int> &hq, queue<int> &lq, int clockTime);
//handles CPU command
void CPUcommand(process &a, queue<int> &hq, queue<int> &lq, int &availableCores, int clockTime);
//handles USER command
void USERcommand(process &a, int clockTime);
//handles SSD command
void SSDcommand(process &a, queue<int> &sq, bool &ssdEmpty, int clockTime);
//concludes an active instruction
void concludeInstruction(process &a, vector<process> processes, int &availableCores, bool &ssdEmpty, string cmd, int clockTime);


int main(){
    int ncores; //tracks number of CPU cores
    int nprocesses = 0; //tracks number of processes
    int clockTime = 0; //iterator for time
    int tempTime;
    string line; //stores input line to be parsed later
    process p; //stores latest process
    vector<process> processes; //stores all processes

    //reads input
    readInput(processes, nprocesses, ncores);
    
    int availableCores = ncores; //tracks available cores
    bool terminate = false;
    bool ssdEmpty = true; // tracks if a process is in the ssd
    //the queues below store the process ID to track order of process commands entering and leaving these queues
    queue<int> hq; //high priority queue
    queue<int> lq; //low priority queue
    queue<int> sq; //ssd queue
    

    //hardware simulation
    while(!terminate){
        for(auto &a : processes){
            //starting a process
            if(!a.started && a.timeStart == clockTime){
                //cout << "Process " << a.ID << " has started at " << clockTime << " ms" << endl;
                a.started = true;
                a.pCommand = "NEW";
            }
            
            if(a.started && !a.complete){
                //continuing active instructions
                if(a.instructions[a.idx].active){
                    string cmd = a.instructions[a.idx].command;
                    a.pTime++;

                    //concluding an active instruction
                    if(a.pTime == a.instructions[a.idx].time){
                        concludeInstruction(a, processes, availableCores, ssdEmpty, cmd, clockTime);
                    }
                }

                //starting an instruction
                if(!a.complete && !a.instructions[a.idx].active){
                    string cmd = a.instructions[a.idx].command;

                    //CPU command
                    if(cmd == "CPU"){
                        CPUcommand(a, hq, lq, availableCores, clockTime);
                    }
                    //SSD command
                    else if(cmd == "SSD"){
                        SSDcommand(a, sq, ssdEmpty, clockTime);
                    }
                    //USER command
                    else if(cmd == "USER"){
                        USERcommand(a, clockTime);
                    }
                }
            }
        }

        // Process queued CPU commands if available cores are free
        emptyQueues(processes, availableCores, hq, lq, clockTime);
        //cout << clockTime << endl;

        clockTime++;
        
        terminate = true;
        for(auto a : processes){
            if(!a.complete)
            terminate = false;
        }
    }

    return 0;
}


//fuction code
void readInput(vector<process> &processes, int &nprocesses, int &ncores){
    int tempTime; //temporary time variable
    string line; // temp storage for input read from file
    process p; // stores last read process
    string fileName;
    cout << "Enter input file name: ";
    cin >> fileName;
    ifstream inFile(fileName);
    //ifstream inFile("input111.txt"); //file reader
    inFile >> line;
    inFile >> ncores;
    //cout << line + " " << ncores << endl;

    inFile >> line;
    //reads input
    while(line != "END"){
        //NEW creates a new process
        if(line == "NEW"){
            if(nprocesses > 0){
                processes.push_back(p);
                p = {};
            }

            inFile >> tempTime;
            

            p.ID = nprocesses;
            p.timeStart = tempTime;
            p.idx = 0;
            p.complete = false;
            p.started = false;
            p.blocked = false;
            p.ready = false;
            p.running = false;
            p.queued = false;
            p.CPUTime = 0;
            p.USERInteracts = 0;
            p.SSDAccesses = 0;
            nprocesses++;

            //cout << line + " " << tempTime << endl;
        }
        // other inputs are commands stored in the newest process
        else{
            inFile >> tempTime;
            if(line == "CPU"){
                p.CPUTime += tempTime;
            }
            else if(line == "USER"){
                p.USERInteracts++;
            }
            else if(line == "SSD"){
                p.SSDAccesses++;
            }
            p.instructions.push_back(input(line, tempTime));
        }
        inFile >> line;
    }
    processes.push_back(p);
    //cout << endl << endl;


    inFile.close();
}

void outputProcesses(vector<process> processes){
    for(auto a : processes){
        cout << "Process " << a.ID << endl;
        cout << "Start Time: " << a.timeStart << endl;
        for(auto b : a.instructions){
            cout << b.command + " " << b.time << endl;
        }
        cout << endl;
    }
    cout << endl;
}

void emptyQueues(vector<process> &processes, int &availableCores, queue<int> &hq, queue<int> &lq, int clockTime){
    if (availableCores > 0) {
        if (!hq.empty()) {
            int procID = hq.front(); //store process ID from front of hq
            process &a = processes[procID];
            if (!a.running && !a.complete && availableCores > 0) {
                availableCores--;
                a.instructions[a.idx].active = true;
                a.pTime = 0;
                a.running = true;
                a.ready = false;
                a.queued = false;
                hq.pop();
                //cout << "Process " << a.ID << " started CPU from high priority queue at " << clockTime << endl;
            }
        } 
        else if (!lq.empty()) {
            int procID = lq.front(); //store process ID from front of lq
            process &a = processes[procID];
            if (!a.running && !a.complete && availableCores > 0) {
                availableCores--;
                a.instructions[a.idx].active = true;
                a.pTime = 0;
                a.running = true;
                a.ready = false;
                a.queued = false;
                lq.pop();
                //cout << "Process " << a.ID << " started CPU from low priority queue at " << clockTime << endl;
            }
        }
    }
}

void CPUcommand(process &a, queue<int> &hq, queue<int> &lq, int &availableCores, int clockTime){
    if(availableCores > 0){
        //if both queues are empty then run CPU
        if(hq.empty() && lq.empty()){
            availableCores--;
            a.instructions[a.idx].active = true;
            a.pTime = 0;
            a.running = true;
            //cout << "Process " << a.ID << " started CPU at " << clockTime << endl;
        }
        //otherwise check if process ID is at front of high priority queue
        else if(hq.front() == a.ID){
                                
            availableCores--;
            a.instructions[a.idx].active = true;
            a.pTime = 0;
            a.running = true;
            a.ready = false;
            a.queued = false;
            hq.pop();
            //cout << "Process " << a.ID << " started CPU at " << clockTime << endl;
        }
        //checks low priority queue next
        else if(lq.front() == a.ID && hq.empty()){
            availableCores--;
            a.instructions[a.idx].active = true;
            a.pTime = 0;
            a.running = true;
            a.ready = false;
            a.queued = false;
            lq.pop();
            //cout << "Process " << a.ID << " started CPU at " << clockTime << endl;
        }
                            
    }
                        
    else if(!a.queued){
        //avoids dupes in queue
        if(a.pCommand == "SSD"){
            lq.push(a.ID);
        }
        else{
            hq.push(a.ID);
        }
        a.queued = true;
        a.ready = true;
    }
}

void USERcommand(process &a, int clockTime){
    a.instructions[a.idx].active = true;
    a.pTime = 0;
    a.blocked = true;
    //cout << "Process " << a.ID << " started USER at " << clockTime << endl;
}

void SSDcommand(process &a, queue<int> &sq, bool &ssdEmpty, int clockTime){
    if(ssdEmpty){
        if(sq.empty()){
            ssdEmpty = false;
            a.instructions[a.idx].active = true;
            a.pTime = 0;
            a.blocked = true;
            //cout << "Process " << a.ID << " started SSD at " << clockTime << endl;
        }
        else if(sq.front() == a.ID){
            ssdEmpty = false;
            a.instructions[a.idx].active = true;
            a.pTime = 0;
            a.blocked = true;
            sq.pop();
            a.queued = false;
            //cout << "Process " << a.ID << " started SSD at " << clockTime << endl;
        }
    }
    else{
        if(!a.queued){
            sq.push(a.ID);
            a.queued = true;
        }
    }
}

void concludeInstruction(process &a, vector<process> processes, int &availableCores, bool &ssdEmpty, string cmd, int clockTime){
    //cout << "Process " << a.ID << " has finished " + cmd << " at " << clockTime << endl;
    a.pCommand = cmd;
    if(cmd == "CPU"){
        a.running = false;
        availableCores++;
    }
    else if(cmd == "SSD"){
        ssdEmpty = true;
        a.blocked = false;
    }
    else if(cmd == "USER"){
        a.blocked = false;
    }
    a.idx++;
    a.pTime = 0;
    if(a.idx == a.instructions.size()){
        a.complete = true;
        a.running = false;
        a.blocked = false;
        a.ready = false;
        cout << "Process " << a.ID << " is terminated at " << clockTime << "ms" << endl;
        cout << "It started at " << clockTime << "ms" << endl;
        cout << "It used " << a.CPUTime << "ms of CPU time, performed " << a.SSDAccesses << " SSD access(es), ";
        cout << "and interacted with its USER " << a.USERInteracts << " time(s)" << endl << endl;

        cout << "PROCESS TABLE" << endl;
        for(auto b : processes){
            cout << "Process " << b.ID << " is ";
            if(b.complete || b.ID == a.ID){
                cout << "TERMINATED" << endl;
            }
            else if(b.running){
                cout << "RUNNING" << endl;
            }
            else if(b.ready){
                cout << "READY" << endl;
            }
            else if(b.blocked){
                cout << "BLOCKED" << endl;
            }
        }
        cout << endl;
    }
}