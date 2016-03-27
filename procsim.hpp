#ifndef PROCSIM_HPP
#define PROCSIM_HPP

#include <cstdint>

#define DEFAULT_K0 1
#define DEFAULT_K1 2
#define DEFAULT_K2 3
#define DEFAULT_R 8
#define DEFAULT_M 2
#define DEFAULT_F 4

#define F -1
#define T  1

/*
 * @author: Sahil Gupta
 * Tomasulo simulator for out of order execution and in-order
 * firing of intructions.
 */

typedef struct _proc_inst_t {
    uint32_t instruction_address;
    int32_t op_code;
    int32_t src_reg[2];
    int32_t dest_reg;
} proc_inst_t;

class proc {
public:
    uint64_t r = 0;
    uint64_t k0 = 0;
    uint64_t k1 = 0;
    uint64_t k2 = 0;
    uint64_t f = 0;
    uint64_t m = 0;

    proc(uint64_t r, uint64_t k0, uint64_t k1, uint64_t k2, uint64_t f, uint64_t m) {
        this->r = r;
        this->k0 = k0;
        this->k1 = k1;
        this->k2 = k2;
        this->f = f;
        this->m = m;
    }
};

typedef struct Register {
    int tag;
    int registerNum;
} RegisterEntry;

typedef struct CommonDataBus {
    int linNum;
    int index;
    int tag;
    int RegisterEntry;
    int FU;
} CommonDB;

typedef struct LinkedListNode {
    LinkedListNode *next;
    LinkedListNode *prev;
    proc_inst_t p_inst;
    int linNum;
    int index;
    int destinationT;
    int sourceT[2];
    int clock;
    int pipelineStage[6];
} LLNode;

typedef struct linkedList {
    LLNode* head;
    LLNode* tail;
    int NumObjects;
    int IsMemAvailable;
} LL;

typedef struct ROB {
    proc_inst_t p_inst;
    int linNum;
    int destinationT;
    int COMPLETED;
    int pipelineStage[6];
} ROBEntry;

typedef struct RobFIFODeck {
    int head;
    int tail;
    int NumObjects;
} RobFIFO;

typedef struct _proc_stats_t {
    float avg_inst_retired;
    unsigned long retired_instruction;
    unsigned long cycle_count;
} proc_stats_t;

bool read_instruction(proc_inst_t* p_inst);

void setup_proc(uint64_t r, uint64_t k0, uint64_t k1, uint64_t k2, uint64_t f, uint64_t m);
void run_proc(proc_stats_t* p_stats);
void complete_proc(proc_stats_t* p_stats);

#endif /* PROCSIM_HPP */
