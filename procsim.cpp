#include <stdlib.h>
#include <stdio.h>
#include "procsim.hpp"

/*
 * @author: Sahil Gupta
 * Tomasulo simulator for out of order execution and in-order
 * firing of intructions.
 */

proc processor(NULL, NULL, NULL, NULL, NULL, NULL);
RegisterEntry RegisterFile[32];

LL dispQ;
LL schdQ0;
LL schdQ1;
LL schdQ2;
LLNode** FU0;
LLNode** FU1;
LLNode** FU2;
ROBEntry *robQ;
RobFIFO ROBFIFO;
int schdQLength[3];

CommonDB* CDB;
CommonDB* anotherCDB;
int CDBsize = 0;
int anotherCDBsize = 0;

int lineNumber;
bool isReadComplete = true;
bool isPipelined = true;
int clock = 1;

void STATUSUPDATESecondHalf();
void EXECSecondHalf();
void SCHEDSecondHalf();
void DISPSecondHalf();
void STATUSUPDATEFirstHalf();
void EXECFirstHalf();
void SCHEDFirstHalf();
void DISPFirstHalf();
void FETCH();

/**
 * Subroutine for initializing the processor. You many add and initialize any global or heap
 * variables as needed.
 * XXX: You're responsible for completing this routine
 *
 * @r ROB size
 * @k0 Number of k0 FUs
 * @k1 Number of k1 FUs
 * @k2 Number of k2 FUs
 * @f Number of instructions to fetch
 * @m Schedule queue multiplier
 */
void setup_proc(uint64_t r, uint64_t k0, uint64_t k1, uint64_t k2, uint64_t f, uint64_t m) {
    processor.r = r;
    processor.f = f;
    processor.k0 = k0;
    processor.k1 = k1;
    processor.k2 = k2;
    processor.m = m;

    schdQLength[0] = (int) m * k0;
    schdQLength[1] = (int) m * k1;
    schdQLength[2] = (int) m * k2;

    printf("INST\tFETCH\tDISP\tSCHED\tEXEC\tSTATE\tRETIRE\n");
    for (int i = 0; i < 32; i++) {
        RegisterFile[i].tag = -3;
        RegisterFile[i].registerNum = i;
    }
    robQ = (ROBEntry*) malloc(processor.r * sizeof (ROBEntry)); //ROBEntry
    CDB = (CommonDB *) malloc((processor.k0 + processor.k1 + processor.k2) * sizeof (CommonDB)); //CDB
    anotherCDB = (CommonDB *) malloc((processor.k0 + processor.k1 + processor.k2) * sizeof (CommonDB)); //CDB
    FU0 = (LLNode**) malloc(k0 * sizeof (LLNode*));
    FU1 = (LLNode**) malloc(k1 * 2 * sizeof (LLNode*));
    FU2 = (LLNode**) malloc(k2 * 3 * sizeof (LLNode*));

    ROBFIFO.head = 0;
    ROBFIFO.NumObjects = 0;
    ROBFIFO.tail = 0;

    dispQ.head = NULL;
    dispQ.tail = NULL;
    dispQ.NumObjects = (int) processor.r;
    dispQ.IsMemAvailable = (int) 0;

    schdQ0.head = NULL;
    schdQ0.tail = NULL;
    schdQ0.NumObjects = schdQLength[0];
    schdQ0.IsMemAvailable = (int) processor.k0 * 1;

    schdQ1.head = NULL;
    schdQ1.tail = NULL;
    schdQ1.NumObjects = schdQLength[1];
    schdQ1.IsMemAvailable = (int) processor.k1 * 2;

    schdQ2.head = NULL;
    schdQ2.tail = NULL;
    schdQ2.NumObjects = schdQLength[2];
    schdQ2.IsMemAvailable = (int) processor.k2 * 3;
}

int l0, l1, l2;

/**
 * Subroutine that simulates the processor.
 *   The processor should fetch instructions as appropriate, until all instructions have executed
 * XXX: You're responsible for completing this routine
 *
 * @p_stats Pointer to the statistics structure
 */
void run_proc(proc_stats_t* p_stats) {
    clock = 0;
    lineNumber = 0;
    while (isPipelined) {
        STATUSUPDATESecondHalf();
        EXECSecondHalf();
        SCHEDSecondHalf();
        DISPSecondHalf();
        clock = clock + 1;
        STATUSUPDATEFirstHalf();
        EXECFirstHalf();
        SCHEDFirstHalf();
        DISPFirstHalf();
        FETCH();
    }
    clock = clock - 1;
}

void printROB(int index) {
    printf("%d\t", robQ[index].linNum);
    printf("%d\t", robQ[index].pipelineStage[0]);//fetch
    printf("%d\t", robQ[index].pipelineStage[1]);//dispatch
    printf("%d\t", robQ[index].pipelineStage[2]);//scheduler
    printf("%d\t", robQ[index].pipelineStage[3]);//execute
    printf("%d\t", robQ[index].pipelineStage[4]);//status update
    printf("%d\n", robQ[index].pipelineStage[5]);//retire
}

/***********************************************************************
 * Helper functions to check whether the ROB is available, empty or
 * occupied. 
 * Also helps in adding entries to the reorder buffer
 ***********************************************************************/

int isReorderBufferEmpty() {
    if (ROBFIFO.tail == ROBFIFO.head && ROBFIFO.NumObjects == 0) {
        return 3;
    } else if (ROBFIFO.head == ROBFIFO.tail) {
        return 2;
    } else {
        return 4;
    }
}

int addEntryToReorderBuffer(LLNode* dispTempN) {
    int index = ROBFIFO.tail; //tag added to 

    if (isReorderBufferEmpty() != 2) { //if there is room in the ROB
        //Put item into ROB table
        robQ[ROBFIFO.tail].linNum = dispTempN->linNum;
        robQ[ROBFIFO.tail].destinationT = dispTempN->destinationT;
        robQ[ROBFIFO.tail].p_inst = dispTempN->p_inst;
        robQ[ROBFIFO.tail].COMPLETED = 0;
        ROBFIFO.tail = (ROBFIFO.tail + 1) % processor.r;
        ROBFIFO.NumObjects = ROBFIFO.NumObjects + 1;
    } else {
        return F;
    }

    return index;
}

/***********************************************************************
 * Helper functions to handle adding and removing of entries from the 
 * linked list data structure for the queues for the dispatcher, scheduler, 
 * reorder buffer
 ***********************************************************************/

void addElementToLinkedList(LL* pointersIn, LLNode* nextNode) {
    pointersIn->NumObjects--;
    nextNode->prev = pointersIn->tail;
    nextNode->next = NULL;
    if (pointersIn->tail != NULL) {
        pointersIn->tail->next = nextNode;
    } else {
        pointersIn->head = nextNode;
    }
    pointersIn->tail = nextNode;
}

void removeFromLinkedList(LL* pointersIn, LLNode* deleteNode) {
    if (deleteNode->prev == NULL) {
        if (deleteNode->next == NULL) {
            pointersIn->head = NULL;
            pointersIn->tail = NULL;
        } else {
            pointersIn->head = deleteNode->next;
            pointersIn->head->prev = NULL;
        }
    } else if (deleteNode->next == NULL) {
        pointersIn->tail = deleteNode->prev;
        pointersIn->tail->next = NULL;
    } else {
        deleteNode->prev->next = deleteNode->next;
        deleteNode->next->prev = deleteNode->prev;
    }
    pointersIn->NumObjects = pointersIn->NumObjects + 1;
}

/***********************************************************************
 * FETCH Stage of the Pipeline - Read the instructions and pass them to
 * dispatcher.
 ***********************************************************************/

void FETCH() {
    LLNode* createdReadN;
    proc_inst_t* p_inst;
    int readFlag = T;
    p_inst = (proc_inst_t*) malloc(sizeof (proc_inst_t));

    int i = 0;
    while (i < processor.f && readFlag == T) {
        if ((dispQ.NumObjects + l0 + l1 + l2) > 0) {
            readFlag = read_instruction(p_inst);
            if (readFlag == T) {
                lineNumber++;
                //Create a new node
                createdReadN = (LLNode*) malloc(sizeof (LLNode));
                createdReadN->p_inst = *p_inst;
                createdReadN->linNum = lineNumber;
                createdReadN->destinationT = lineNumber;
                createdReadN->sourceT[0] = -2;
                createdReadN->sourceT[1] = -2;
                createdReadN->clock = -2;
                createdReadN->pipelineStage[0] = clock;
                createdReadN->pipelineStage[1] = clock + 1;
                addElementToLinkedList(&dispQ, createdReadN);
            } else {
                isReadComplete = false;
            }

        } else {
            break;
        }
        i++;
    }
    free(p_inst);
}

void DISPFirstHalf() {
    l0 = 0;
    l1 = 0;
    l2 = 0;

    //Dispatch the instructions to the dispatcher
    int dispatcherFlag = T;
    proc_inst_t instructionDispatch;
    int index;

    for (LLNode* dispTempN = dispQ.head; dispatcherFlag != F && dispTempN != NULL;) {
        instructionDispatch = dispTempN->p_inst;
        if ((instructionDispatch.op_code == 0 || instructionDispatch.op_code == -1) && (schdQ0.NumObjects - l0) > 0) {

            if (isReorderBufferEmpty() != 2) {
                l0++;
                dispTempN->pipelineStage[2] = clock + 1;
                dispTempN->clock = -3;
                index = addEntryToReorderBuffer(dispTempN);
                dispTempN->index = index;
                dispTempN = dispTempN->next;
            } else {
                dispatcherFlag = F;
            }
        } else if (instructionDispatch.op_code == 1 && (schdQ1.NumObjects - l1) > 0) {
            if (isReorderBufferEmpty() != 2) {
                l1++;
                dispTempN->pipelineStage[2] = clock + 1;
                dispTempN->clock = -3;
                index = addEntryToReorderBuffer(dispTempN);
                dispTempN->index = index;
                dispTempN = dispTempN->next;
            } else {
                dispatcherFlag = F;
            }
        } else if (instructionDispatch.op_code == 2 && (schdQ2.NumObjects - l2) > 0) {
            if (isReorderBufferEmpty() != 2) {
                l2++;
                dispTempN->pipelineStage[2] = clock + 1;
                dispTempN->clock = -3;
                index = addEntryToReorderBuffer(dispTempN);
                dispTempN->index = index;
                dispTempN = dispTempN->next;
            } else {
                dispatcherFlag = F;
            }
        } else {
            dispatcherFlag = F;
        }
    }
}

int isClockTimingCorrect(int u) {
    int i = 0;
    if (u == 0) {
        for (int j = 0; j < processor.k0; j++) {
            if (FU1[j] != NULL && FU1[j]->clock == 1) {
                i++;
            }
            if (i >= processor.k0) {
                return 0;
            }
        }
    }
    if (u == 1) {
        for (int j = 0; j < processor.k1 * 2; j++) {
            if (FU1[j] != NULL && FU1[j]->clock == 2) {
                i++;
            }
            if (i >= processor.k1) {
                return 0;
            }
        }
    }
    if (u == 2) {
        for (int j = 0; j < processor.k2 * 3; j++) {
            if (FU2[j] != NULL && FU2[j]->clock == 3) {
                i++;
            }
            if (i >= processor.k2) {
                return 0;
            }
        }
    }
    return 1;
}

void SCHEDFirstHalf() {
    LLNode* temp0 = schdQ0.head;
    LLNode* temp1 = schdQ1.head;
    LLNode* temp2 = schdQ2.head;
    while (temp0 != NULL || temp1 != NULL || temp2 != NULL) {
        if (temp0 != NULL && schdQ0.IsMemAvailable > 0 && temp0->sourceT[0] == -3 && temp0->sourceT[1] == -3 && temp0->clock == -3 && isClockTimingCorrect(0)) {
            schdQ0.IsMemAvailable--;
            temp0->clock = 1;
            temp0->pipelineStage[3] = clock + 1;
            for (int j = 0; j < processor.k0; j++) {
                if (FU0[j] == NULL) {
                    FU0[j] = temp0;
                    break;
                }
            }
            temp0 = temp0->next;
        } else if (temp0 != NULL) {
            temp0 = temp0->next;
        }
        if (temp1 != NULL && schdQ1.IsMemAvailable > 0 && temp1->sourceT[0] == -3 && temp1->sourceT[1] == -3 && temp1->clock == -3 && isClockTimingCorrect(1)) {
            schdQ1.IsMemAvailable--;
            temp1->clock = 2;
            temp1->pipelineStage[3] = clock + 1;
            for (int j = 0; j < processor.k1 * 2; j++) {
                if (FU1[j] == NULL) {
                    FU1[j] = temp1;
                    break;
                }
            }
            temp1 = temp1->next;
        } else if (temp1 != NULL) {
            temp1 = temp1->next;
        }
        if (temp2 != NULL && schdQ2.IsMemAvailable > 0 && temp2->sourceT[0] == -3 && temp2->sourceT[1] == -3 && temp2->clock == -3 && isClockTimingCorrect(2)) {
            schdQ2.IsMemAvailable--;
            temp2->clock = 3;
            temp2->pipelineStage[3] = clock + 1;
            for (int j = 0; j < processor.k2 * 3; j++) {
                if (FU2[j] == NULL) {
                    FU2[j] = temp2;
                    break;
                }
            }
            temp2 = temp2->next;
        } else if (temp2 != NULL) {
            temp2 = temp2->next;
        }
    }
}

void EXECFirstHalf() {
    anotherCDBsize = 0;
    int w = 0;
    while (w < processor.k0 * 1) {
        if (FU0[w] != NULL) {
            FU0[w]->clock--;
            //Check if instructions is COMPLETED
            if (FU0[w]->clock == 0) {
                anotherCDB[anotherCDBsize].tag = FU0[w]->destinationT;
                anotherCDB[anotherCDBsize].index = FU0[w]->index;
                anotherCDB[anotherCDBsize].FU = 0;
                anotherCDB[anotherCDBsize].linNum = FU0[w]->linNum;
                anotherCDB[anotherCDBsize++].RegisterEntry = FU0[w]->p_inst.dest_reg;
                FU0[w]->pipelineStage[4] = clock + 1;
                FU0[w]->clock = -4;
            }
        }

        w++;
    }
    w = 0;

    while (w < processor.k1 * 2) {
        if (FU1[w] != NULL) {
            FU1[w]->clock--;
            if (FU1[w]->clock == 0) {
                anotherCDB[anotherCDBsize].tag = FU1[w]->destinationT;
                anotherCDB[anotherCDBsize].index = FU1[w]->index;
                anotherCDB[anotherCDBsize].FU = 1;
                anotherCDB[anotherCDBsize].linNum = FU1[w]->linNum;
                anotherCDB[anotherCDBsize++].RegisterEntry = FU1[w]->p_inst.dest_reg;
                FU1[w]->pipelineStage[4] = clock + 1;
                FU1[w]->clock = -4;
            }
        }

        w++;
    }
    w = 0;

    while (w < processor.k2 * 3) {
        if (FU2[w] != NULL) {
            FU2[w]->clock--;
            if (FU2[w]->clock == 0) {
                anotherCDB[anotherCDBsize].tag = FU2[w]->destinationT;
                anotherCDB[anotherCDBsize].index = FU2[w]->index;
                anotherCDB[anotherCDBsize].FU = 2;
                anotherCDB[anotherCDBsize].linNum = FU2[w]->linNum;
                anotherCDB[anotherCDBsize++].RegisterEntry = FU2[w]->p_inst.dest_reg;
                FU2[w]->pipelineStage[4] = clock + 1;
                FU2[w]->clock = -4;
            }
        }

        w++;
    }
    w = 0;

    while (w < anotherCDBsize) {
        if (RegisterFile[anotherCDB[w].RegisterEntry].tag == anotherCDB[w].tag) {
            RegisterFile[anotherCDB[w].RegisterEntry].tag = -3;
        }

        w++;
    }
    w = 0;

    while (w < processor.k0 * 1) {
        if (FU0[w] != NULL) {
            if (FU0[w]->clock == -4) {
                schdQ0.IsMemAvailable++;
                FU0[w] = NULL;
            }
        }
        w++;
    }
    w = 0;

    while (w < processor.k1 * 2) {
        if (FU1[w] != NULL) {
            if (FU1[w]->clock == -4) {
                schdQ1.IsMemAvailable++;
                FU1[w] = NULL;
            }
        }
        w++;
    }
    w = 0;

    while (w < processor.k2 * 3) {
        if (FU2[w] != NULL) {
            if (FU2[w]->clock == -4) {
                schdQ2.IsMemAvailable++;
                FU2[w] = NULL;
            }
        }
        w++;
    }
    
    CommonDB swapbus;
    for (int i = 0; i < anotherCDBsize; i++) {
        for (int j = i; j < anotherCDBsize; j++) {
            if (anotherCDB[i].linNum > anotherCDB[j].linNum) {
                swapbus = anotherCDB[i];
                anotherCDB[i] = anotherCDB[j];
                anotherCDB[j] = swapbus;
            }
        }
    }
}

void DISPSecondHalf() {
    //Set up all the registers by initializing them

    LLNode* dispTempN, *tempNode;
    dispTempN = dispQ.head;
    int temp = l0 + l1 + l2;

    for (int i = 0; i < temp && dispTempN != NULL; i++) {
        //Now create the node to insert into the schedule        
        if (dispTempN->p_inst.src_reg[0] != -1) {
            dispTempN->sourceT[0] = RegisterFile[dispTempN->p_inst.src_reg[0]].tag;
        } else {
            dispTempN->sourceT[0] = -3;
        }
        if (dispTempN->p_inst.src_reg[1] != -1) {
            dispTempN->sourceT[1] = RegisterFile[dispTempN->p_inst.src_reg[1]].tag;
        } else {
            dispTempN->sourceT[1] = -3;
        }
        if (dispTempN->p_inst.dest_reg != -1) {
            RegisterFile[dispTempN->p_inst.dest_reg].tag = dispTempN->destinationT;
        }

        tempNode = dispTempN;
        dispTempN = dispTempN->next;
        removeFromLinkedList(&dispQ, tempNode);
        if ((tempNode->p_inst.op_code == 0 || tempNode->p_inst.op_code == -1)) {
            addElementToLinkedList(&schdQ0, tempNode);
        }
        if (tempNode->p_inst.op_code == 1) {
            addElementToLinkedList(&schdQ1, tempNode);

        }
        if (tempNode->p_inst.op_code == 2) {
            addElementToLinkedList(&schdQ2, tempNode);
        }
    }
}

void SCHEDSecondHalf() {
    LLNode* lNode;
    lNode = schdQ0.head;
    int j = 0;
    while (lNode != NULL) {
        while (j < CDBsize) {
            if (CDB[j].tag == lNode->sourceT[0]) {
                lNode->sourceT[0] = -3;
            }
            if (CDB[j].tag == lNode->sourceT[1]) {
                lNode->sourceT[1] = -3;
            }
            j++;
        }
        j = 0;
        lNode = lNode->next;
    }
    lNode = schdQ1.head;
    while (lNode != NULL) {
        while (j < CDBsize) {
            if (CDB[j].tag == lNode->sourceT[0]) {
                lNode->sourceT[0] = -3;
            }
            if (CDB[j].tag == lNode->sourceT[1]) {
                lNode->sourceT[1] = -3;
            }
            j++;
        }
        j = 0;
        lNode = lNode->next;
    }
    lNode = schdQ2.head;
    while (lNode != NULL) {
        while (j < CDBsize) {
            if (CDB[j].tag == lNode->sourceT[0]) {
                lNode->sourceT[0] = -3;
            }
            if (CDB[j].tag == lNode->sourceT[1]) {
                lNode->sourceT[1] = -3;
            }
            j++;
        }
        j = 0;
        lNode = lNode->next;
    }
}

void EXECSecondHalf() {
    //create the new data bus
    for (int i = 0; i < anotherCDBsize; i++) {
        CDB[i] = anotherCDB[i];
    }
    CDBsize = anotherCDBsize;
}

void STATUSUPDATEFirstHalf() {
    int j = 0;
    while (j < CDBsize) {
        //update the reorder buffer entry
        robQ[CDB[j].index].COMPLETED = 1;
        j++;
    }
}

void RETIRE() {
    int indexROB;
    int initHead = ROBFIFO.head;

    for (int i = 0; i < processor.f; i++) {
        indexROB = (initHead + i) % processor.r;
        if (robQ[indexROB].COMPLETED == 1 && (clock - robQ[indexROB].pipelineStage[4]) > 0) { //change 2.2
            //remove the node from the reorder buffer to pipelineStage[5] the lineNumber
            robQ[ROBFIFO.head].pipelineStage[5] = clock;
            printROB(ROBFIFO.head);
            robQ[ROBFIFO.head].COMPLETED = 0;
            ROBFIFO.head = (ROBFIFO.head + 1) % processor.r;
            ROBFIFO.NumObjects = ROBFIFO.NumObjects - 1;
        } else {
            break;
        }
    }

    if (isReadComplete == false && isReorderBufferEmpty() == 3) {
        isPipelined = false;
    }
}

void STATUSUPDATESecondHalf() {
    //remove the instructions from the scheduler
    LLNode* lNode;

    for (int j = 0; j < CDBsize; j++) {
        if (CDB[j].FU == 0) {
            lNode = schdQ0.head;
            while (lNode != NULL) {
                if (CDB[j].tag == lNode->destinationT) {
                    lNode->pipelineStage[5] = clock;
                    robQ[lNode->index].linNum = lNode->linNum;
                    robQ[lNode->index].pipelineStage[0] = lNode->pipelineStage[0];
                    robQ[lNode->index].pipelineStage[1] = lNode->pipelineStage[1];
                    robQ[lNode->index].pipelineStage[2] = lNode->pipelineStage[2];
                    robQ[lNode->index].pipelineStage[3] = lNode->pipelineStage[3];
                    robQ[lNode->index].pipelineStage[4] = lNode->pipelineStage[4];
                    robQ[lNode->index].pipelineStage[5] = lNode->pipelineStage[5];

                    removeFromLinkedList(&schdQ0, lNode);
                    free(lNode);
                    break;
                }
                lNode = lNode->next;
            }
        } else if (CDB[j].FU == 1) {
            lNode = schdQ1.head;
            while (lNode != NULL) {
                if (CDB[j].tag == lNode->destinationT) {
                    lNode->pipelineStage[5] = clock;
                    robQ[lNode->index].linNum = lNode->linNum;
                    robQ[lNode->index].pipelineStage[0] = lNode->pipelineStage[0];
                    robQ[lNode->index].pipelineStage[1] = lNode->pipelineStage[1];
                    robQ[lNode->index].pipelineStage[2] = lNode->pipelineStage[2];
                    robQ[lNode->index].pipelineStage[3] = lNode->pipelineStage[3];
                    robQ[lNode->index].pipelineStage[4] = lNode->pipelineStage[4];
                    robQ[lNode->index].pipelineStage[5] = lNode->pipelineStage[5];

                    removeFromLinkedList(&schdQ1, lNode);
                    free(lNode);
                    break;
                }
                lNode = lNode->next;
            }
        } else if (CDB[j].FU == 2) {
            lNode = schdQ2.head;
            while (lNode != NULL) {
                if (CDB[j].tag == lNode->destinationT) {
                    lNode->pipelineStage[5] = clock;
                    robQ[lNode->index].linNum = lNode->linNum;
                    robQ[lNode->index].pipelineStage[0] = lNode->pipelineStage[0];
                    robQ[lNode->index].pipelineStage[1] = lNode->pipelineStage[1];
                    robQ[lNode->index].pipelineStage[2] = lNode->pipelineStage[2];
                    robQ[lNode->index].pipelineStage[3] = lNode->pipelineStage[3];
                    robQ[lNode->index].pipelineStage[4] = lNode->pipelineStage[4];
                    robQ[lNode->index].pipelineStage[5] = lNode->pipelineStage[5];

                    removeFromLinkedList(&schdQ2, lNode);
                    free(lNode);
                    break;
                }
                lNode = lNode->next;
            }
        }
    }
    RETIRE();
}

/**
 * Subroutine for cleaning up any outstanding instructions and calculating overall statistics
 * such as average IPC or branch prediction percentage
 * XXX: You're responsible for completing this routine
 *
 * @p_stats Pointer to the statistics structure
 */
void complete_proc(proc_stats_t *p_stats) {
    //stats and freeing of memory
    free(CDB);
    free(anotherCDB);
    free(robQ);
    free(FU0);
    free(FU1);
    free(FU2);
    printf("\n");
    p_stats->retired_instruction = lineNumber;
    p_stats->cycle_count = clock;
    p_stats->avg_inst_retired = ((double) lineNumber) / clock;
    printf("\n");
}