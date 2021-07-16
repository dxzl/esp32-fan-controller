#ifndef IndexRepeatListH
#define IndexRepeatListH

#include <Arduino.h>
#include <vector>

typedef struct {int index; bool bStale; uint16_t rptCount; uint16_t rptCounter; uint16_t everyCount; uint16_t everyCounter;} t_indexRepeat;

// this is used to make a list of slots which have non-zero seconds-field, entered
// from p2.html. That allows us not to have to read every time-slot every second!
// We populate the list when preferences are loaded in setup(). We then add to the
// list when a new time-entry is made, if it has non-zero seconds. We remove from
// the list as we delete entries from p2.html.
// functions prefaced with IV_ work on this dynamic array...
int IR_GetIndex(int ir_list_idx);
uint16_t IR_GetRptCount(int ir_list_idx);
uint16_t IR_GetRptCounter(int ir_list_idx);
uint16_t IR_GetEveryCount(int ir_list_idx);
uint16_t IR_GetEveryCounter(int ir_list_idx);
int IR_GetCount(void);
void IR_Clear(void);
void IR_SetIndex(int ir_list_idx, int val);
void IR_SetRptCount(int ir_list_idx, uint16_t val);
void IR_SetRptCounter(int ir_list_idx, uint16_t val);
void IR_SetEveryCount(int ir_list_idx, uint16_t val);
void IR_SetEveryCounter(int ir_list_idx, uint16_t val);
void IR_SetStaleFlag(int ir_list_idx, bool val);
bool IR_SetSize(int newSize);
bool IR_Add(int index, bool bStale, uint16_t repeatCount, uint16_t everyCount);

int IR_FindIndexBySlot(int slotIndex);

bool IR_RemoveIndexBySlot(int slotIndex);

void IR_ResetCountersBySlot(int slotIndex);
void IR_ResetCounters(int ir_list_idx);

bool IR_IncRepeatCounterBySlot(int slotIndex);
bool IR_IncRepeatCounter(int ir_list_idx);

bool IR_IsStaleBySlot(int slotIndex);
bool IR_IsStale(int ir_list_idx);

extern std::vector<t_indexRepeat> m_indexRepeatArray;

#endif
