/*****************************************************
*      SYSC4001 - F2022 - Assignment 1 Solution      *
******************************************************
* By: Ben Earle (BenEarle@cmail.carleton.ca)         *
* Modified by Claire Villanueva and Divya            *
* Vithiyatharan - F2023, Assignment 2                *
******************************************************
* This sample solution was prepared for Dr. Wainer   *
* in fall of 2022. This solution uses a linked list  *
* to store the each states processes. They are       *
* scheduled in a FIFO manner. If you have any        *
* questions please do not hesitate to reach out via  *
* email or during the scheduled lab times.           *
******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#define min(a, b) (((a) < (b)) ? (a) : (b))

enum STATE {
    STATE_NEW,
    STATE_READY,
    STATE_RUNNING,
    STATE_WAITING,
    STATE_TERMINATED
};
static const char *STATES[] = { "NEW", "READY", "RUNNING", "WAITING", "TERMINATED"};

struct process {
    int pid;
    int arrival_time;
    int total_cpu_time;
    int cpu_time_remaining;
    int io_frequency;
    int io_duration;
    int io_time_remaining;
    int priority;
    int process_memory;
    int throughput_time;
    int turnaround_time;
    int waiting_time;
    enum STATE s;
};

struct node {
    struct process *p;
    struct node *next;
    struct node *size;
};

typedef struct process *proc_t;
typedef struct node *node_t;

struct memory_segment {
    bool filled;
    int size;
    proc_t process;
};

proc_t create_proc(int pid, int arrival_time, int total_cpu_time, int io_frequency, int io_duration, int priority,
                   int process_memory, int throughput_time, int turnaround_time, int waiting_time){
    proc_t temp;
    temp = (proc_t) malloc(sizeof(struct process));

    temp->pid=pid;
    temp->arrival_time = arrival_time;
    temp->total_cpu_time = total_cpu_time;
    temp->cpu_time_remaining = total_cpu_time;
    temp->io_frequency = io_frequency;
    temp->io_duration = io_duration;
    temp->io_time_remaining = io_frequency;
    temp->s = STATE_NEW;
    temp->priority = priority;
    temp->process_memory;
    temp->throughput_time = 0;
    temp->turnaround_time = 0;
    temp->waiting_time = 0;

    return temp;
}

node_t create_node(proc_t p){
    node_t temp;
    temp = (node_t) malloc(sizeof(struct node));

    temp->next = NULL;
    temp->p = p;
    temp->size = 0;

    return temp;
}

void print_nodes(node_t head) {
    node_t current = head;
    proc_t p;

    if(head == NULL){
        printf("EMPTY\n");
        return;
    }

    while (current != NULL) {
        p = current->p;
        printf("Process ID: %d\n", p->pid);
        printf("Priority: %d\n", p->priority);
        printf("CPU Arrival Time: %dms\n", p->arrival_time);
        printf("Time Remaining: %ds of %dms\n", p->cpu_time_remaining, p->total_cpu_time);
        printf("IO Duration: %dms\n", p->io_duration);
        printf("IO Frequency: %dms\n", p->io_frequency);
        printf("Current state: %s\n", STATES[p->s]);
        printf("Time until next IO event: %dms\n", p->io_time_remaining);
        printf("\n");
        current = current->next;
    }
}

node_t push_node(node_t head, node_t temp){
    node_t prev;

    if(head == NULL){
        head = temp;
    } else {
        prev = head;
        while(prev->next != NULL){
            prev = prev->next;
        }
        prev->next = temp;
    }
    temp->next = NULL;
    temp->size += 1;

    return head;
}

int remove_node(node_t *head, node_t to_be_removed){
    node_t temp, prev;

    if(to_be_removed == *head){
        *head = (*head)->next;
        to_be_removed->next = NULL;

        return 1;
    } else {
        temp = *head;
        // Iterate through the list until we've checked every node
        while(temp->next != NULL){
            prev = temp;
            temp = temp->next;
            if(temp == to_be_removed){
                prev->next = temp->next;
                // NOTE:Calling function must free to_be_removed when finished with it.
                // Since the address of the node to be removed was passed to this function
                // the calling function must already have a reference to it
                to_be_removed->next = NULL;

                return 1;
            }
        }
    }

    return -1;
}

node_t read_proc_from_file(char *input_file){
    int MAXCHAR = 128;
    char row[MAXCHAR];
    node_t new_list=NULL, node;
    proc_t proc;
    int pid, arrival_time, total_cpu_time, io_frequency, io_duration, priority, memory;

    FILE* f = fopen(input_file, "r");
    if(f == NULL){
        printf("NULL FILE\n\n\n\n");
    }

    fgets(row, MAXCHAR, f);

    do {
        fgets(row, MAXCHAR, f);
        if(strlen(row)<10) continue;

        pid = atoi(strtok(row, ","));
        arrival_time = atoi(strtok(NULL, ","));
        total_cpu_time = atoi(strtok(NULL, ","));
        io_frequency = atoi(strtok(NULL, ","));
        io_duration = atoi(strtok(NULL, ","));
        priority = atoi(strtok(NULL, ","));
        memory = atoi(strtok(NULL, ","));

        proc = create_proc(pid, arrival_time, total_cpu_time, io_frequency, io_duration, priority, memory, 0, 0, 0);
        node = create_node(proc);
        new_list = push_node(new_list, node);

    } while (feof(f) != true);

    return new_list;
}

void clean_up(node_t list){
    node_t temp;
    while(list != NULL){
        temp = list;
        list = list->next;
        free(temp->p);
        free(temp);
    }
}

int get_time_to_next_event(int cpu_clock, node_t running, node_t new_list, node_t waiting_list){
    node_t temp;
    int next_exit=INT_MAX, next_block=INT_MAX, next_arrival=INT_MAX, next_io=INT_MAX;

    if(running != NULL){

        next_exit = running->p->cpu_time_remaining;
        next_block = running->p->io_time_remaining;
    }

    temp = new_list;
    while(temp != NULL){
        next_arrival = min(temp->p->arrival_time - cpu_clock, next_arrival);
        temp = temp->next;
    }

    temp = waiting_list;
    while(temp != NULL){
        next_io = min(temp->p->io_time_remaining, next_io);
        temp = temp->next;
    }

    return min(min(next_exit, next_block), min(next_arrival, next_io));
}

/**
 * Calculate and print throughput, average turnaround time, and wait time for a given scenario
 *
 */
void print_metrics(int verbose, int num_processes, int cpu_clock, double total_wait_time, double total_turnaround){
    double throughput;
    double avg_turnaround;
    double avg_wait_time;

    // calculate throughput
    throughput = (double) num_processes / cpu_clock;

    // calculate average turnaround time
    avg_turnaround = total_turnaround / num_processes;

    // calculate average wait time
    avg_wait_time = total_wait_time / num_processes;

    printf("\nTHROUGHPUT: %f processes per ms\n", throughput);
    printf("AVERAGE TURNAROUND TIME: %.2fms\n", avg_turnaround);
    printf("TOTAL WAIT TIME: %.2fms\n", total_wait_time);
    printf("AVERAGE WAIT TIME: %.2fms\n", avg_wait_time);
    if(verbose) printf("-------------------------------------------------------------------------------------\n");

}

void fcfs(node_t new_list, int verbose){
    int next_step = 0, cpu_clock = 0, num_processes = 0;
    double total_throughput = 0.0, total_turnaround = 0.0, total_wait_time = 0.0;

    bool simulation_completed = false;
    node_t ready_list = NULL;
    node_t waiting_list = NULL;
    node_t terminated = NULL;
    node_t temp;
    node_t node;
    node_t running = NULL;

    printf("\nSIMULATING FCFS WITHOUT PREEMPTION\n");

    if(verbose)printf("\n-------------------------------------------------------------------------------------\n");

    printf("Time of transition,PID,Old State,New State\n");
    do {
        cpu_clock += next_step;
        node = waiting_list;

        while(node != NULL){
            if (node ==NULL) break;
            node->p->io_time_remaining -= next_step;
            if(node->p->io_time_remaining <= 0){
                node->p->s = STATE_READY;
                node->p->io_time_remaining = node->p->io_frequency;

                total_wait_time +=1000;

                temp = node->next;
                remove_node(&waiting_list, node);
                ready_list = push_node(ready_list, node);
                printf("%d,%d,%s,%s\n", cpu_clock, node->p->pid, STATES[STATE_WAITING], STATES[STATE_READY]);

                node = temp;
            } else {
                node = node->next;
            }
        }

        node = new_list;
        while(node!= NULL) {
            if(node->p->arrival_time == cpu_clock){
                node->p->s = STATE_READY;

                total_wait_time +=1000;

                temp = node->next;
                remove_node(&new_list, node);
                ready_list = push_node(ready_list, node);
                printf("%d,%d,%s,%s\n", cpu_clock, node->p->pid, STATES[STATE_NEW], STATES[STATE_READY]);

                node = temp;
            } else {
                node = node->next;
            }
        }

        if(running == NULL){
            if(ready_list!=NULL){
                running = ready_list;
                running->p->s = STATE_RUNNING;

                remove_node(&ready_list, running);
                printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_READY], STATES[STATE_RUNNING]);
            } else{
                running = NULL;
                if(verbose) printf("%d: CPU is idle\n", cpu_clock);
            }
        } else {
            running->p->cpu_time_remaining -= next_step;
            running->p->io_time_remaining -= next_step;

            if(running->p->cpu_time_remaining <= 0){
                running->p->s = STATE_TERMINATED;
                running->p->turnaround_time += cpu_clock - running->p->arrival_time;

                total_turnaround += running->p->turnaround_time;
                total_wait_time += running ->p->waiting_time;

                terminated = push_node(terminated,running);
                num_processes++;
                printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_RUNNING], STATES[STATE_TERMINATED]);

                if(ready_list!=NULL){
                    running = ready_list;
                    running->p->s = STATE_RUNNING;
                    total_wait_time +=1000;
                    remove_node(&ready_list, running);
                    printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_READY], STATES[STATE_RUNNING]);

                } else{
                    running = NULL;
                    if(verbose) printf("%d: CPU is idle\n", cpu_clock);
                }

            } else if(running->p->io_time_remaining <= 0){
                running->p->io_time_remaining = running->p->io_duration;
                running->p->s = STATE_WAITING;
                running->p->waiting_time += running->p->io_duration;


                total_wait_time += running->p->waiting_time;

                waiting_list = push_node(waiting_list,running);
                printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_RUNNING], STATES[STATE_WAITING]);

                if(ready_list!=NULL){
                    running = ready_list;
                    total_wait_time +=1000;
                    running->p->s = STATE_RUNNING;
                    remove_node(&ready_list, running);
                    printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_READY], STATES[STATE_RUNNING]);

                } else {
                    running = NULL;
                    if(verbose) printf("%d: CPU is idle\n", cpu_clock);
                }
            }
        }

        next_step = get_time_to_next_event(cpu_clock, running, new_list, waiting_list);

        if(verbose){
            printf("-------------------------------------------------------------------------------------\n");
            printf("At CPU time %ds...\n", cpu_clock);
            printf("-------------------------------\n");
            printf("The CPU is currently running:\n");
            print_nodes(running);
            printf("-------------------------------\n");
            printf("The new process list is:\n");
            print_nodes(new_list);
            printf("-------------------------------\n");
            printf("The ready queue is:\n");
            print_nodes(ready_list);
            printf("-------------------------------\n");
            printf("The waiting list is:\n");
            print_nodes(waiting_list);
            printf("-------------------------------\n");
            printf("The terminated list is:\n");
            print_nodes(terminated);
            printf("-------------------------------------------------------------------------------------\n");
        }

        simulation_completed = (ready_list == NULL) && (new_list == NULL) && (waiting_list == NULL) && (running == NULL);
    } while(!simulation_completed);
    if(verbose) printf("Simulation completed in %dms.\n", cpu_clock);
    if(verbose) printf("-------------------------------------------------------------------------------------\n");

    print_metrics(verbose, num_processes, cpu_clock, total_wait_time, total_turnaround);
    clean_up(terminated);
}

void round_robin(node_t new_list, int verbose) {
    int timeout = 1000;

    int next_step = 0, cpu_clock = 0, num_processes = 0;
    double total_throughput = 0.0, total_turnaround = 0.0, total_wait_time = 0.0;

    bool simulation_completed = false;
    node_t ready_list = NULL;
    node_t waiting_list = NULL;
    node_t terminated = NULL;
    node_t temp;
    node_t node;
    node_t running = NULL;

    printf("\nSIMULATING ROUND ROBIN WITH 1000ms TIMEOUT\n");

    if(verbose)printf("\n-------------------------------------------------------------------------------------\n");

    printf("Time of transition,PID,Old State,New State\n");

    do {
        cpu_clock += next_step;
        node = waiting_list;

        while (node != NULL) {
            if (node == NULL) break;
            node->p->io_time_remaining -= next_step;
            if (node->p->io_time_remaining <= 0) {
                node->p->s = STATE_READY;
                node->p->io_time_remaining = node->p->io_frequency;

                total_wait_time +=1000;

                temp = node->next;
                remove_node(&waiting_list, node);
                ready_list = push_node(ready_list, node);
                printf("%d,%d,%s,%s\n", cpu_clock, node->p->pid, STATES[STATE_WAITING], STATES[STATE_READY]);

                node = temp;
            } else {
                node = node->next;
            }
        }

        node = new_list;
        while (node != NULL) {
            if (node->p->arrival_time == cpu_clock) {
                node->p->s = STATE_READY;

                total_wait_time +=1000;

                temp = node->next;
                remove_node(&new_list, node);
                ready_list = push_node(ready_list, node);
                printf("%d,%d,%s,%s\n", cpu_clock, node->p->pid, STATES[STATE_NEW], STATES[STATE_READY]);

                node = temp;
            } else {
                node = node->next;
            }
        }

        if (running == NULL) {
            if (ready_list != NULL) {
                running = ready_list;
                running->p->s = STATE_RUNNING;
                remove_node(&ready_list, running);
                printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_READY], STATES[STATE_RUNNING]);

            } else {
                running = NULL;
                if (verbose) printf("%d: CPU is idle\n", cpu_clock);
            }

        } else {
            running->p->cpu_time_remaining -= next_step;
            running->p->io_time_remaining -= next_step;

            if (running->p->cpu_time_remaining >= timeout) {
                running->p->cpu_time_remaining -= timeout; // remove quantum from time remaining

                running->p->s = STATE_READY;

                ready_list = push_node(ready_list, running);

                printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_RUNNING], STATES[STATE_READY]);

                remove_node(&running, running);

            } else if (running->p->cpu_time_remaining <= 0) {
                running->p->s = STATE_TERMINATED;
                running->p->turnaround_time += cpu_clock - running->p->arrival_time;

                total_turnaround += running->p->turnaround_time;
                total_wait_time += running ->p->waiting_time + 1;

                terminated = push_node(terminated, running);
                num_processes++;
                printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_RUNNING], STATES[STATE_TERMINATED]);
                remove_node(&running, running);

                if (ready_list != NULL) {
                    running = ready_list;
                    total_wait_time +=1000;
                    running->p->s = STATE_RUNNING;
                    remove_node(&ready_list, running);
                    printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_READY], STATES[STATE_RUNNING]);

                } else {
                    running = NULL;
                    if (verbose) printf("%d: CPU is idle\n", cpu_clock);
                }

            } else if (running->p->io_time_remaining <= 0) {
                running->p->io_time_remaining = running->p->io_duration;
                running->p->s = STATE_WAITING;
                running->p->waiting_time += running->p->io_duration;

                total_wait_time += running->p->waiting_time;

                waiting_list = push_node(waiting_list, running);
                printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_RUNNING], STATES[STATE_WAITING]);

                if (ready_list != NULL) {
                    running = ready_list;
                    total_wait_time +=1000;
                    running->p->s = STATE_RUNNING;
                    remove_node(&ready_list, running);
                    printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_READY], STATES[STATE_RUNNING]);

                } else {
                    running = NULL;
                    if (verbose) printf("%d: CPU is idle\n", cpu_clock);
                }

            } else { // theoretically, the process should never enter here
                if(verbose)printf("SOMETHING WENT WRONG!\n");

            }

        }


        next_step = get_time_to_next_event(cpu_clock, running, new_list, waiting_list);

        if (verbose) {
            printf("-------------------------------------------------------------------------------------\n");
            printf("At CPU time %dms...\n", cpu_clock);
            printf("-------------------------------\n");
            printf("The CPU is currently running:\n");
            print_nodes(running);
            printf("-------------------------------\n");
            printf("The new process list is:\n");
            print_nodes(new_list);
            printf("-------------------------------\n");
            printf("The ready queue is:\n");
            print_nodes(ready_list);
            printf("-------------------------------\n");
            printf("The waiting list is:\n");
            print_nodes(waiting_list);
            printf("-------------------------------\n");
            printf("The terminated list is:\n");
            print_nodes(terminated);
            printf("-------------------------------------------------------------------------------------\n");
        }

        simulation_completed =
                (ready_list == NULL) && (new_list == NULL) && (waiting_list == NULL) && (running == NULL);
    } while (!simulation_completed);
    if (verbose) printf("Simulation completed in %dms.\n", cpu_clock);
    if (verbose) printf("-------------------------------------------------------------------------------------\n");

    print_metrics(verbose, num_processes, cpu_clock, total_wait_time, total_turnaround);
    clean_up(terminated);
}

void external_priority(node_t new_list, int verbose){
    int next_step = 0, cpu_clock = 0, num_processes = 0;
    double total_throughput = 0.0, total_turnaround = 0.0, total_wait_time = 0.0;

    bool simulation_completed = false;
    node_t ready_list = NULL;
    node_t waiting_list = NULL;
    node_t terminated = NULL;
    node_t temp;
    node_t node;
    node_t running = NULL;

    printf("\nSIMULATING EXTERNAL PRIORITY WITHOUT PREEMPTION\n");

    if(verbose)printf("\n-------------------------------------------------------------------------------------\n");

    printf("Time of transition,PID,Old State,New State\n");

    do {
        cpu_clock += next_step;
        node = waiting_list;

        while(node != NULL){
            if (node ==NULL) break;
            node->p->io_time_remaining -= next_step;
            if(node->p->io_time_remaining <= 0){
                node->p->s = STATE_READY;
                node->p->io_time_remaining = node->p->io_frequency;

                total_wait_time +=1000;

                temp = node->next;
                remove_node(&waiting_list, node);
                ready_list = push_node(ready_list, node);
                printf("%d,%d,%s,%s\n", cpu_clock, node->p->pid, STATES[STATE_WAITING], STATES[STATE_READY]);

                node = temp;
            } else {
                node = node->next;
            }
        }

        node = new_list;
        while(node!= NULL) {
            if(node->p->arrival_time == cpu_clock){
                node->p->s = STATE_READY;

                total_wait_time +=1000;

                temp = node->next;
                remove_node(&new_list, node);
                ready_list = push_node(ready_list, node);
                printf("%d,%d,%s,%s\n", cpu_clock, node->p->pid, STATES[STATE_NEW], STATES[STATE_READY]);

                node = temp;
            } else {
                node = node->next;
            }
        }

        if(running == NULL){
            if(ready_list!=NULL){
                node_t highest_priority = ready_list;
                node_t current = ready_list;

                while(current!=NULL){
                    if(current->p->priority < highest_priority->p->priority){
                        highest_priority = current;
                    }
                    current = current->next;
                }

                running = highest_priority;
                running->p->s = STATE_RUNNING;
                remove_node(&ready_list, running);
                printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_READY], STATES[STATE_RUNNING]);
            } else{
                running = NULL;
                if(verbose) printf("%d: CPU is idle\n", cpu_clock);
            }
        } else {
            running->p->cpu_time_remaining -= next_step;
            running->p->io_time_remaining -= next_step;

            if (running->p->cpu_time_remaining <= 0) {
                running->p->s = STATE_TERMINATED;
                running->p->turnaround_time += cpu_clock - running->p->arrival_time;

                total_turnaround += running->p->turnaround_time;
                total_wait_time += running->p->waiting_time;

                terminated = push_node(terminated, running);
                num_processes++;
                printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_RUNNING], STATES[STATE_TERMINATED]);

                if (ready_list != NULL) {
                    // PRIORITY LOGIC
                    node_t highest_priority = ready_list;
                    node_t current = ready_list->next;

                    while(current!=NULL){
                        if(current->p->priority < highest_priority->p->priority){
                            highest_priority = current;
                        }
                        current = current->next;
                    }
                    if(verbose)printf("Selected Highest Priority Process PID %d: Priority %d\n", highest_priority->p->pid, highest_priority->p->priority);

                    total_wait_time +=1000;
                    running = highest_priority;
                    running->p->s = STATE_RUNNING;
                    remove_node(&ready_list, running);
                    printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_READY], STATES[STATE_RUNNING]);

                } else {
                    running = NULL;
                    if (verbose) printf("%d: CPU is idle\n", cpu_clock);
                }
            } else if(running->p->io_time_remaining <= 0){
                running->p->io_time_remaining = running->p->io_duration;
                running->p->s = STATE_WAITING;
                running->p->waiting_time += running->p->io_duration;
                total_wait_time += running->p->waiting_time;
                waiting_list = push_node(waiting_list,running);
                printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_RUNNING], STATES[STATE_WAITING]);

                if(ready_list!=NULL){
                    node_t highest_priority = ready_list; // set ready_list as default highest priority
                    node_t current = ready_list->next;

                    while(current!=NULL){
                        if(current->p->priority < highest_priority->p->priority){
                            highest_priority = current;
                        }
                        current = current->next;
                    }
                    if(verbose)printf("Selected Highest Priority Process PID %d: Priority %d\n", highest_priority->p->pid, highest_priority->p->priority);

                    running = highest_priority;
                    total_wait_time +=1000;
                    running->p->s = STATE_RUNNING;
                    remove_node(&ready_list, running);
                    printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_READY], STATES[STATE_RUNNING]);

                } else {
                    running = NULL;
                    if(verbose) printf("%d: CPU is idle\n", cpu_clock);
                }
            }
        }

        next_step = get_time_to_next_event(cpu_clock, running, new_list, waiting_list);

        if(verbose){
            printf("-------------------------------------------------------------------------------------\n");
            printf("At CPU time %dms...\n", cpu_clock);
            printf("-------------------------------\n");
            printf("The CPU is currently running:\n");
            print_nodes(running);
            printf("-------------------------------\n");
            printf("The new process list is:\n");
            print_nodes(new_list);
            printf("-------------------------------\n");
            printf("The ready queue is:\n");
            print_nodes(ready_list);
            printf("-------------------------------\n");
            printf("The waiting list is:\n");
            print_nodes(waiting_list);
            printf("-------------------------------\n");
            printf("The terminated list is:\n");
            print_nodes(terminated);
            printf("-------------------------------------------------------------------------------------\n");
        }

        simulation_completed = (ready_list == NULL) && (new_list == NULL) && (waiting_list == NULL) && (running == NULL);
    } while(!simulation_completed);
    if(verbose) printf("Simulation completed in %dms.\n", cpu_clock);
    if(verbose) printf("-------------------------------------------------------------------------------------\n");

    print_metrics(verbose, num_processes, cpu_clock, total_wait_time, total_turnaround);
    clean_up(terminated);
}

struct memory_segment initialize_memory(int scheduler_type, struct memory_segment *memory) {
    int capacity[4];

    if (scheduler_type == 3) {
        capacity[0] = 500;
        capacity[1] = 250;
        capacity[2] = 150;
        capacity[3] = 100;
    } else {
        capacity[0] = 300;
        capacity[1] = 300;
        capacity[2] = 150;
        capacity[3] = 50;
    }

    for (int i = 0; i < 4; i++) {
        memory[i].filled = false;
        memory[i].size = capacity[i];
        memory[i].process = NULL;
    }

    return *memory;
}

void add_memory(proc_t *p, struct memory_segment *memory, int *total_memory_used,
                int *memory_segments_used, int *total_memory_free, int *usable_memory_free) {
    for (int i = 0; i < 4; i++) {
        if (!memory[i].filled && memory[i].size >= (*p)->process_memory) {
            memory[i].process = *p;
            memory[i].filled = true;

            (*memory_segments_used) += 1;
            (*total_memory_used) += (*p)->process_memory;

            break;
        } else {
            (*usable_memory_free) += memory[i].size;
        }
    }
    (*total_memory_free) = 1000 - (*total_memory_used);
}

void remove_memory(proc_t *p, struct memory_segment *memory) {
    for (int i = 0; i < 4; i++) {
        if (memory[i].filled && memory[i].process == *p) {
            memory[i].filled = false;
            memory[i].process = NULL;
            break;
        }
    }
}

void fcfs_with_memory(node_t new_list, struct memory_segment memory[], int verbose) {
    int next_step = 0, cpu_clock = 0, num_processes = 0;
    bool simulation_completed = false;
    node_t ready_list = NULL;
    node_t waiting_list = NULL;
    node_t terminated = NULL;
    node_t temp;
    node_t node;
    node_t running = NULL;

    int total_memory_used = 0;
    int memory_segments_used = 0;
    int total_memory_free = 0;
    int usable_memory_free = 0;

    printf("\nSIMULATING FCFS WITH MEMORY MANAGEMENT\n");

    if (verbose) printf("\n-------------------------------------------------------------------------------------\n");

    printf("Time of transition,PID,Old State,New State\n");
    do {
        cpu_clock += next_step;
        node = waiting_list;

        while (node != NULL) {
            if (node == NULL) break;
            node->p->io_time_remaining -= next_step;
            if (node->p->io_time_remaining <= 0) {
                node->p->s = STATE_READY;
                node->p->io_time_remaining = node->p->io_frequency;

                temp = node->next;
                remove_node(&waiting_list, node);
                ready_list = push_node(ready_list, node);
                printf("%d,%d,%s,%s\n", cpu_clock, node->p->pid, STATES[STATE_WAITING], STATES[STATE_READY]);

                node = temp;
                printf("Total Memory In Use: %d\nMemory Partitions Used: %d\nTotal Memory Free: %d\nUsable Free Memory: %d\n",
                       total_memory_used, memory_segments_used, total_memory_free, usable_memory_free);
            } else {
                node = node->next;
            }
        }

        node = new_list;
        while (node != NULL) {
            if (node->p->arrival_time == cpu_clock) {
                node->p->s = STATE_READY;

                temp = node->next;
                remove_node(&new_list, node);
                ready_list = push_node(ready_list, node);
                printf("%d,%d,%s,%s\n", cpu_clock, node->p->pid, STATES[STATE_NEW], STATES[STATE_READY]);

                node = temp;
                printf("Total Memory In Use: %d\nMemory Partitions Used: %d\nTotal Memory Free: %d\nUsable Free Memory: %d\n",
                       total_memory_used, memory_segments_used, total_memory_free, usable_memory_free);
            } else {
                node = node->next;
            }
        }

        if (running == NULL) {
            if (ready_list != NULL) {
                running = ready_list;
                running->p->s = STATE_RUNNING;

                add_memory(&(running->p), memory, &total_memory_used, &memory_segments_used, &total_memory_free, &usable_memory_free);

                remove_node(&ready_list, running);
                printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_READY], STATES[STATE_RUNNING]);
            } else {
                running = NULL;
                if (verbose) printf("%d: CPU is idle\n", cpu_clock);
            }
        } else {
            running->p->cpu_time_remaining -= next_step;
            running->p->io_time_remaining -= next_step;

            if (running->p->cpu_time_remaining <= 0) {
                running->p->s = STATE_TERMINATED;
                running->p->turnaround_time += cpu_clock - running->p->arrival_time;
                terminated = push_node(terminated, running);
                num_processes++;
                printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_RUNNING], STATES[STATE_TERMINATED]);

                // Memory Management
                remove_memory(&(running->p), memory);

                if (ready_list != NULL) {
                    running = ready_list;
                    running->p->s = STATE_RUNNING;

                    add_memory(&(running->p), memory, &total_memory_used, &memory_segments_used, &total_memory_free, &usable_memory_free);

                    remove_node(&ready_list, running);
                    printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_READY], STATES[STATE_RUNNING]);

                } else {
                    running = NULL;
                    if (verbose) printf("%d: CPU is idle\n", cpu_clock);
                }

            } else if (running->p->io_time_remaining <= 0) {
                running->p->io_time_remaining = running->p->io_duration;
                running->p->s = STATE_WAITING;
                running->p->waiting_time += running->p->io_duration;
                waiting_list = push_node(waiting_list, running);
                printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_RUNNING], STATES[STATE_WAITING]);

                // Memory Management
                remove_memory(&(running->p), memory);

                if (ready_list != NULL) {
                    running = ready_list;
                    running->p->s = STATE_RUNNING;

                    add_memory(&(running->p), memory, &total_memory_used, &memory_segments_used, &total_memory_free, &usable_memory_free);

                    remove_node(&ready_list, running);
                    printf("%d,%d,%s,%s\n", cpu_clock, running->p->pid, STATES[STATE_READY], STATES[STATE_RUNNING]);

                } else {
                    running = NULL;
                    if (verbose) printf("%d: CPU is idle\n", cpu_clock);
                }
            }
        }

        next_step = get_time_to_next_event(cpu_clock, running, new_list, waiting_list);

        if (verbose) {
            printf("-------------------------------------------------------------------------------------\n");
            printf("At CPU time %ds...\n", cpu_clock);
            printf("-------------------------------\n");
            printf("The CPU is currently running:\n");
            print_nodes(running);
            printf("-------------------------------\n");
            printf("The new process list is:\n");
            print_nodes(new_list);
            printf("-------------------------------\n");
            printf("The ready queue is:\n");
            print_nodes(ready_list);
            printf("-------------------------------\n");
            printf("The waiting list is:\n");
            print_nodes(waiting_list);
            printf("-------------------------------\n");
            printf("The terminated list is:\n");
            print_nodes(terminated);
            printf("-------------------------------\n");
            printf("Memory Status:\n");
            for (int i = 0; i < 4; i++) {
                printf("Size = %d:, Filled? = %s, Process PID = %d\n", memory[i].size, memory[i].filled ?
                                                                                       "true" : "false", (memory[i].process != NULL) ? memory[i].process->pid : -1);

            }
            printf("\n");
            printf("-------------------------------------------------------------------------------------\n");
        }

        simulation_completed = (ready_list == NULL) && (new_list == NULL) && (waiting_list == NULL) && (running == NULL);

    } while(!simulation_completed);
    if(verbose) printf("Simulation completed in %ds.\n", cpu_clock);
    if(verbose) printf("-------------------------------------------------------------------------------------\n");

    clean_up(terminated);
}

void run_scheduler(int scheduler_type, node_t new_list, int verbose) {
    if(scheduler_type == 0){
        fcfs(new_list, verbose);
    } else if (scheduler_type == 1){
        external_priority(new_list, verbose);
    } else if (scheduler_type == 2){
        round_robin(new_list, verbose);
    } else if (scheduler_type == 3) {
        struct memory_segment memory[4];
        initialize_memory(scheduler_type, memory);
        fcfs_with_memory(new_list, memory, verbose);
    } else if (scheduler_type == 4) {
        struct memory_segment memory[4];
        initialize_memory(scheduler_type, memory);
        fcfs_with_memory(new_list, memory, verbose);
    }else {
        printf("No scheduler selected\n");
        exit(1);
    }
}

int main( int argc, char *argv[]) {

    node_t new_list = NULL;
    char *input_file;
    int verbose;
    int scheduler_type;

    if(argc == 3){
        input_file = argv[1];
        verbose = 0;
        scheduler_type = atoi(argv[2]);
    } else if( argc == 4 ) {
        input_file = argv[1];
        verbose = atoi(argv[2]);
        scheduler_type = atoi(argv[3]);
    } else {
        printf("Three or four args expected.\n");
        return -1;
    }

    // Process meta data should be read from a text file
    if(verbose) printf("------------------------------- Loading all processes -------------------------------\n");
    new_list = read_proc_from_file(input_file);
    if(verbose) print_nodes(new_list);
    if(verbose) printf("-------------------------------------------------------------------------------------\n");

    run_scheduler(scheduler_type, new_list, verbose);

}
