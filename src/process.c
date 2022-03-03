/*
    Arquivo utilitário para definir e gerenciar processos.
*/

typedef struct {
    unsigned int pid;
    int status;
    int arrivalTime;
    int cpuBurstTime;
    int cpuBurstTimeRemaining;
    int quantumRemaining;
    int ioBurstTimeRemaining;
    int ioStartTime;
    int ioType;
    int waitingTime;
    int turnaroundTime;
    int endTime;
} Process;

Process * newProcess(int arrivalTime, int cpuBurstTime, int ioStartTime, int ioType){
    Process *p;
    p = malloc(sizeof(*p));
    p->pid = rand() % (4294967290);
    p->status = STATUS_NULL;
    p-> arrivalTime = arrivalTime;
    p-> cpuBurstTime = cpuBurstTime;
    p-> cpuBurstTimeRemaining = cpuBurstTime;
    p-> ioStartTime = ioStartTime;
    p-> ioType = ioType;
    p->waitingTime = 0;
    p->turnaroundTime = 0;
    p->endTime = 0;
    
    // this ugly code is necessary cause C doesnt support map data structure natively.
    if(ioType == IO_TYPE_DISC) 
        p-> ioBurstTimeRemaining = IO_TIME_DISC;
    else if (ioType == IO_TYPE_TAPE)
        p-> ioBurstTimeRemaining = IO_TIME_TAPE;
    else if (ioType == IO_TYPE_PRINTER)
        p-> ioBurstTimeRemaining = IO_TIME_PRINTER;
        
    return p;
}