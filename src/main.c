#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "constants.c"
#include "process.c"
#include "queue.c"

Queue *highPriorityQueue;                   // Fila de alta prioridade.
Queue *lowPriorityQueue;                    // Fila de baixa prioridade.
Queue *discQueue;                           // Fila de disco.
Queue *tapeQueue;                           // Fila fita.
Queue *printerQueue;                        // Fila de impressora.

Process *processes[NUMBER_OF_PROCESSES];    // Lista com todos os processos.

Process *executingProcess;                  // Processo que se encontra em execução.
int hasExecutingProcess = FALSE;            // Indica se há processo em execução.

int t = 0;                                  // System Time;

void lookupForNewProcesses();
void checkIOWaitingProcess(Queue *q, Queue *priorityQueue);
void scheduleNewProcessForRunning();
void runExecutingProcess();
int hasActiveProcesses();
void start();

int main(){
    srand(time(NULL));

    // Cria filas.
    highPriorityQueue = newQueue();
    lowPriorityQueue = newQueue();
    discQueue = newQueue();
    tapeQueue = newQueue();
    printerQueue = newQueue();
    
    // Cria processos (arrivalTime, cpuBurstTime, ioStartTime, ioType).
    for (int i = 0; i < NUMBER_OF_PROCESSES; i++)
    {
        // Intervalo [0, MAX_ARRIVAL_TIME]
        int randomArrivalTime = rand() % MAX_ARRIVAL_TIME;
        // Intervalo [0, MAX_BURST_TIME]
        int randomBurstTime = rand() % (MAX_BURST_TIME) + 1;
        // Intervalo [0, randomBurstTime-1]
        int randomIoStartTime = rand() % (randomBurstTime + 1);
        // Tipos de IO: 1 - disco, 2 - fita magnetica, 3 - impressora
        int randIoType = (rand() % NUMBER_OF_IO_TYPES) + 1;
        processes[i] = newProcess(randomArrivalTime, randomBurstTime, randomIoStartTime, randIoType);
    }

    start();
    
    //Monta tabela com resultados obtidos
    for(int i = 0; i < NUMBER_OF_PROCESSES; i++){
        Process *p = processes[i];
        printf("\nProcesso [PID %d]: Tempo Chegada: %d; Tempo da finalização: %d; TAT: %d; WT: %d", p->pid, p->arrivalTime, p->endTime, p->turnaroundTime, p->waitingTime);
    }
    printf("\n\n");

    return 0;
}

void start(){
    while(1){
        lookupForNewProcesses();

        checkIOWaitingProcess(discQueue, lowPriorityQueue);
        checkIOWaitingProcess(tapeQueue, highPriorityQueue);
        checkIOWaitingProcess(printerQueue, highPriorityQueue);
    
        if(!hasExecutingProcess)
            scheduleNewProcessForRunning();

        if(hasExecutingProcess)
            runExecutingProcess();

        if(!hasActiveProcesses()) break;

        t++;
    }
}

// Checa se chegaram novos processos, se sim inclui na lista de alta prioridade.
void lookupForNewProcesses(){
    for(int i = 0; i < NUMBER_OF_PROCESSES; i++){
        Process *p = processes[i];
        int processHasJustArrived = (p->status == STATUS_NULL && t >= p->arrivalTime) ? TRUE : FALSE;
        if(processHasJustArrived){
            printf("[t = %d] Process with PID %d has arrived.\n", t, p->pid);
            p->status = STATUS_READY;
            push(highPriorityQueue, p);
        }
    }
}

// Checa se há processo esperando I/O, se já terminou inclui na fila de alta/baixa prioridade.
void checkIOWaitingProcess(Queue *q, Queue *priorityQueue){
    if(queueHasProcesses(q)){
        Process *p = q->processOnTop;
        int processHasFinishedWaiting = (p->status == STATUS_WAITING && p->ioBurstTimeRemaining == 0) ? TRUE : FALSE;
        if(processHasFinishedWaiting){
            p->status = STATUS_READY;
            push(priorityQueue, p);
            pop(q);
            printf("[t = %d] Process with PID %d has finished waiting for I/O.\n", t, p->pid);
        }
        else{
            if(p->ioBurstTimeRemaining > 0) p->ioBurstTimeRemaining -= 1;
        }
    }
}

// Checa se há processo em execução, se não hover escala novo processo.
void scheduleNewProcessForRunning(){
    if(queueHasProcesses(highPriorityQueue)){
        executingProcess = pop(highPriorityQueue);
        executingProcess->status = STATUS_RUNNING;
        executingProcess->quantumRemaining = QUANTUM;
        hasExecutingProcess = 1;
        printf("[t = %d] Process with PID %d has started running.\n", t, executingProcess->pid);
    }
    else if(queueHasProcesses(lowPriorityQueue)){
        executingProcess = pop(lowPriorityQueue);
        executingProcess->status = STATUS_RUNNING;
        executingProcess->quantumRemaining = QUANTUM;
        hasExecutingProcess = 1;
        printf("[t = %d] Process with PID %d has started running.\n", t, executingProcess->pid);
    }
}

// Executa uma unidade de tempo do processo que está em execução.
void runExecutingProcess(){
    if(executingProcess->cpuBurstTimeRemaining > 0)
        executingProcess->cpuBurstTimeRemaining -= 1;
    executingProcess->quantumRemaining -= 1;

    int processIsRequestingIO = (t >= executingProcess->ioStartTime && executingProcess->ioBurstTimeRemaining > 0);
    if(processIsRequestingIO){
        if(executingProcess->ioType == IO_TYPE_DISC) 
            push(discQueue, executingProcess);
        else if (executingProcess->ioType == IO_TYPE_TAPE)
            push(tapeQueue, executingProcess);
        else if (executingProcess->ioType == IO_TYPE_PRINTER)
            push(printerQueue, executingProcess);

        executingProcess->status = STATUS_WAITING;
        hasExecutingProcess = FALSE;
        printf("[t = %d] Process with PID %d has requested I/O.\n", t, executingProcess->pid);
        return;
    }

    int processHasTerminated = (executingProcess->cpuBurstTimeRemaining == 0 && executingProcess->ioBurstTimeRemaining == 0);
    if(processHasTerminated){
        executingProcess->status = STATUS_TERMINATED;
        hasExecutingProcess = FALSE;
        printf("[t = %d] Process with PID %d has terminated.\n", t, executingProcess->pid);
        executingProcess->endTime = t;
        executingProcess->turnaroundTime = executingProcess->endTime - executingProcess->arrivalTime;
        executingProcess->waitingTime = executingProcess->turnaroundTime - executingProcess->cpuBurstTime;
    }
    else if(executingProcess->quantumRemaining == 0){
        executingProcess->status = STATUS_READY;
        push(lowPriorityQueue, executingProcess);
        hasExecutingProcess = FALSE;
        printf("[t = %d] Process with PID %d was preempted.\n", t, executingProcess->pid);
    }
}

int hasActiveProcesses(){
    for(int i = 0; i < NUMBER_OF_PROCESSES; i++)
        if(processes[i]->status != STATUS_TERMINATED)
            return TRUE;
    return FALSE;
}
