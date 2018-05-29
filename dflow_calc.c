/* 046267 Computer Architecture - Spring 2017 - HW #3 */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <stdbool.h>
#include <stdlib.h>

// Defines

#define NO_DEP -1
#define NOT_USED -1

// Structs

typedef struct {
	bool is_last_dep; // Whether a different cmd is dep on it.
	int dep1; // Up to 2 dependencies
	int dep2;
	unsigned int depth; // not including command itself
} CmdStats;

typedef struct {
	CmdStats *inst_array;
	unsigned int arr_len;
	unsigned int overall_depth;
} ProgData;

// Declarations

ProgData* InitProg(unsigned int numOfInsts);
void CheckDep(ProgData *data, InstInfo progTrace[], int cmd_num, int dep_num,
	unsigned int opsLatency[]);
unsigned int findProgDepth(ProgData *data, unsigned int opsLatency[], InstInfo progTrace[]);


int max(int a, int b) {
	return (a>b) ? a : b;
}

// Actual Code

ProgData* InitProg(unsigned int numOfInsts) {

	// Allocation
	ProgData *data = malloc(sizeof(ProgData));
	if (!data)
		return PROG_CTX_NULL;

	data->inst_array = malloc(numOfInsts * sizeof(CmdStats));
	if (!(data->inst_array)) {
		free(data);
		return PROG_CTX_NULL;
	}

	// Init phase
	data->arr_len = numOfInsts;
	for (int i = 0; i < numOfInsts; i++) {
		(data->inst_array[i]).is_last_dep = true;
		(data->inst_array[i]).dep1 = NO_DEP;
		(data->inst_array[i]).dep2 = NO_DEP;
		(data->inst_array[i]).depth = 0;
	}

	return data;
}

void CheckDep(ProgData *data, InstInfo progTrace[], int cmd_num, int dep_num,
	unsigned int opsLatency[]) {

	bool is_updated = false;
	InstInfo cmd = progTrace[cmd_num];
	InstInfo dep_cmd = progTrace[dep_num];
	int dep_depth;
	int cmd_depth;

	// Update dep1  (Src1)
	if (cmd.src1Idx != NOT_USED && cmd.src1Idx == dep_cmd.dstIdx) {
		if (data->inst_array[cmd_num].dep1 < dep_num)
			data->inst_array[cmd_num].dep1 = dep_num;
		is_updated = true;
	}
	// Update dep2  (Src2)
	if (cmd.src2Idx != NOT_USED && cmd.src2Idx == dep_cmd.dstIdx) {
		if (data->inst_array[cmd_num].dep1 < dep_num)
			data->inst_array[cmd_num].dep1 = dep_num;
		is_updated = true;
	}
	if (is_updated)
		goto updated;
	return;

updated:
	// Update the dep command, that it is depended upon.
	data->inst_array[dep_num].is_last_dep = false;
	
	// Update depth
	dep_depth = data->inst_array[dep_num].depth;
	cmd_depth = data->inst_array[cmd_num].depth;
	data->inst_array[cmd_num].depth = 
		max(dep_depth + opsLatency[dep_cmd.opcode], cmd_depth);
}

unsigned int findProgDepth(ProgData *data, unsigned int opsLatency[], InstInfo progTrace[]) {
	unsigned int overall_depth = 0;
	for (int i = 0; i < data->arr_len; i++) {
		if (data->inst_array[i].is_last_dep == true){
			overall_depth = max(overall_depth,
				data->inst_array[i].depth + opsLatency[progTrace[i].opcode]);
	}
	return overall_depth;
}

ProgCtx analyzeProg(const unsigned int opsLatency[],  InstInfo progTrace[], unsigned int numOfInsts) {

	// Alloc & Init
	ProgData *data = InitProg(numOfInsts);
	if (!data)
		return PROG_CTX_NULL;

	// The Fun Stuff
/*
 *CheckDep(ProgData *data, InstInfo progTrace[], int cmd_num, int dep_num,
 *                unsigned int opsLatency[])
 */
	// We start from 1 because cmd no.0 has no dependencies, with a depth of 0.
	for (int i = 1; i < numOfInsts; i++) {
		CheckDep(data, progTrace, i, i-1);
		if (i != 1)
			CheckDep(data, progTrace, i, i-2);
		}
	}

	data->overall_depth = findProgDepth(data, opsLatency);

	return (ProgCtx)data;
}

void freeProgCtx(ProgCtx ctx) {
	if (ctx) {
		ProgData *data = (ProgData)ctx;
		free(data->inst_array);
		free(data);
	}
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
	ProgData *data = (progData*)ctx;
	if (theInst >= data->arr_len)
		return -1;
	return data->inst_array[theInst].depth;
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
	ProgData *data = (ProgData*)ctx;
	*src1DepInst = data->inst_array[theInst].dep1;
	*src2DepInst = data->inst_array[theInst].dep2;
    return -1;
}

int getProgDepth(ProgCtx ctx) {
	ProgData *data = (ProgData*)ctx;
	return data->overall_depth;
}


