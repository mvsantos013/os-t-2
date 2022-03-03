/*
    Arquivo utilitário para criar filas FIFO e métodos para gerenciar as filas (push e pop).
*/

typedef struct {
    int head;
    int tail;
    Process *processes[MAX_QUEUE_SIZE];
    int processesLength;
    Process *processOnTop;
} Queue;


Queue * newQueue(){
    Queue *q;
    q = malloc(sizeof(*q));
    q->head = 0;
    q->tail = 0;
    return q;
}

// Adiciona processo na fila.
void push(Queue *q, Process *process){   
    q->processes[q->tail] = process;
    q->processesLength++;
    if (q->tail == MAX_QUEUE_SIZE)      
        q->tail = 0;
    else
        q->tail++;
    if (q->head == q->tail) 
        fprintf(stderr, "full\n");
    
    if(q->processesLength == 1) q->processOnTop = process;
}

// Remove processo da fila.
Process * pop(Queue *q){
    Process *process;
    process = q->processes[q->head];
    q->processesLength--;

    if (q->head == MAX_QUEUE_SIZE) 
        q->head = 0;
    else
        q->head++;
    
    if(q->processesLength > 0) q->processOnTop = q->processes[q->head];

    return process;
}

// Verifica se a fila não está vazia.
int queueHasProcesses(Queue *q){
    return (q->processesLength > 0) ? TRUE: FALSE;
}