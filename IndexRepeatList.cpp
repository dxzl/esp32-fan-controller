#include "IndexRepeatList.h"

std::vector<t_indexRepeat> m_indexRepeatArray(0);

//-------------------------------------------------------------
// Functions for IndexValue Array (a C++ vector)
// Note: I had this implemented as a C++ Class but the
// object was being placed in RAM - this way, it's in flash.
//-------------------------------------------------------------
int IR_GetIndex(int ir_list_idx) { return m_indexRepeatArray[ir_list_idx].index; }
uint16_t IR_GetRptCount(int ir_list_idx) { return m_indexRepeatArray[ir_list_idx].rptCount; }
uint16_t IR_GetRptCounter(int ir_list_idx) { return m_indexRepeatArray[ir_list_idx].rptCounter; }
uint16_t IR_GetEveryCount(int ir_list_idx) { return m_indexRepeatArray[ir_list_idx].everyCount; }
uint16_t IR_GetEveryCounter(int ir_list_idx) { return m_indexRepeatArray[ir_list_idx].everyCounter; }
int IR_GetCount(void) { return m_indexRepeatArray.size(); }
void IR_Clear(void) { m_indexRepeatArray.resize(0); }
void IR_SetIndex(int ir_list_idx, int val) { m_indexRepeatArray[ir_list_idx].index = val; }
void IR_SetRptCount(int ir_list_idx, uint16_t val) { m_indexRepeatArray[ir_list_idx].rptCount = val; }
void IR_SetRptCounter(int ir_list_idx, uint16_t val) { m_indexRepeatArray[ir_list_idx].rptCounter = val; }
void IR_SetEveryCount(int ir_list_idx, uint16_t val) { m_indexRepeatArray[ir_list_idx].everyCount = val; }
void IR_SetEveryCounter(int ir_list_idx, uint16_t val) { m_indexRepeatArray[ir_list_idx].everyCounter = val; }
void IR_SetStaleFlag(int ir_list_idx, bool val) { m_indexRepeatArray[ir_list_idx].bStale = val; }


//-------------------------------------------------------------------------------------------
bool IR_IsStaleBySlot(int slotIndex)
{
  return IR_IsStale(IR_FindIndexBySlot(slotIndex));
}

bool IR_IsStale(int ir_list_idx)
{
  if (ir_list_idx >= 0)
    return m_indexRepeatArray[ir_list_idx].bStale;
  return false;
}

// return true if success
void IR_ResetCountersBySlot(int slotIndex)
{
  IR_ResetCounters(IR_FindIndexBySlot(slotIndex));
}

void IR_ResetCounters(int ir_list_idx)
{
  if (ir_list_idx >= 0)
  {
    IR_SetRptCounter(ir_list_idx, 0);
    IR_SetEveryCounter(ir_list_idx, 0);
  }
}

// this returns true if we should call DoEvent()
bool IR_IncRepeatCounterBySlot(int slotIndex)
{
  return IR_IncRepeatCounter(IR_FindIndexBySlot(slotIndex));
}

// this returns true if we should call DoEvent()
bool IR_IncRepeatCounter(int ir_list_idx)
{
  // if no repeat count is even set, return false
  if (ir_list_idx < 0)
    return false;

  // if either everyCount or repeatCount is non-zero, there will
  // be a list-entry for that slot...
  int everyCount = IR_GetEveryCount(ir_list_idx);
  if (everyCount > 0)
  {
    int everyCounter = IR_GetEveryCounter(ir_list_idx);
    
    bool bDoEvent;
    if (++everyCounter >= everyCount)
    {
      bDoEvent = true;
      everyCounter = 0;
    }
    else
      bDoEvent = false;
    
    IR_SetEveryCounter(ir_list_idx, everyCounter);

    if (!bDoEvent)
      return false;
  }
  
  int repeatCount = IR_GetRptCount(ir_list_idx);
  
  // if repeatCount is 0, repeat forever everyXYZ...
  if (repeatCount == 0)
    return true;
    
  int repeatCounter = IR_GetRptCounter(ir_list_idx);

  // finished repeatCount times
  if (repeatCounter >= repeatCount)
    return false;

  // incerment the repeat counter
  IR_SetRptCounter(ir_list_idx, ++repeatCounter);
  
  return true;
}

// return true if found and removed
bool IR_RemoveIndexBySlot(int slotIndex)
{
  bool bFound = false;
  int count = IR_GetCount();
  for (int ii = 0; ii < count; ii++)
  {
    if (bFound)
      m_indexRepeatArray[ii-1] = m_indexRepeatArray[ii];
    else if (IR_GetIndex(ii) == slotIndex)
      bFound = true;
  }
  if (bFound)
    IR_SetSize(count-1);
  return bFound;
}

//-------------------------------------------------------------------------------------------
// return true if success
bool IR_Add(int index, bool bStale, uint16_t repeatCount, uint16_t everyCount)
{
  int oldCount = IR_GetCount();
  int newCount = oldCount+1;
  if (!IR_SetSize(newCount))
    return false;
  IR_SetIndex(oldCount, index);
  
  IR_SetRptCount(oldCount, repeatCount);
  IR_SetRptCounter(oldCount, 0);

  IR_SetEveryCount(oldCount, everyCount);
  IR_SetEveryCounter(oldCount, 0);

  IR_SetStaleFlag(oldCount, bStale);
  return true;
}

// return -1 if not found
// or the 0-based index position in the list if found
int IR_FindIndexBySlot(int slotIndex)
{
  int count = IR_GetCount();
  for (int ii = 0; ii < count; ii++)
    if (IR_GetIndex(ii) == slotIndex)
      return ii;
  return -1;
}

// return true if success
bool IR_SetSize(int newSize)
{
  m_indexRepeatArray.resize(newSize);
  return (newSize == IR_GetCount()) ? true : false;
}
