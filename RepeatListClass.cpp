// this file RepeatListClass.cpp
#include "FanController.h"

RepeatListClass IRL;

//-------------------------------------------------------------
// Functions for RepeatListClass (a C++ vector)
//-------------------------------------------------------------
int RepeatListClass::GetIndex(int idx) { return arr[idx].index; }
void RepeatListClass::SetIndex(int idx, int val) { arr[idx].index = val; }
uint16_t RepeatListClass::GetRptCount(int idx) { return arr[idx].rptCount; }
void RepeatListClass::SetRptCount(int idx, uint16_t val) { arr[idx].rptCount = val; }
uint16_t RepeatListClass::GetRptCounter(int idx) { return arr[idx].rptCounter; }
void RepeatListClass::SetRptCounter(int idx, uint16_t val) { arr[idx].rptCounter = val; }
uint16_t RepeatListClass::GetEveryCount(int idx) { return arr[idx].everyCount; }
void RepeatListClass::SetEveryCount(int idx, uint16_t val) { arr[idx].everyCount = val; }
uint16_t RepeatListClass::GetEveryCounter(int idx) { return arr[idx].everyCounter; }
void RepeatListClass::SetEveryCounter(int idx, uint16_t val) { arr[idx].everyCounter = val; }
bool RepeatListClass::GetStaleFlag(int idx) { return arr[idx].bStale; }
void RepeatListClass::SetStaleFlag(int idx, bool val) { arr[idx].bStale = val; }
int RepeatListClass::GetCount(void) { return arr.size(); }
void RepeatListClass::Clear(void) { arr.resize(0); }

//-------------------------------------------------------------------------------------------
bool RepeatListClass::IsStaleBySlot(int slotIndex){
  return IsStale(FindIndexBySlot(slotIndex));
}

bool RepeatListClass::IsStale(int idx){
  if (idx >= 0)
    return GetStaleFlag(idx);
  return false;
}

// return true if success
void RepeatListClass::ResetCountersBySlot(int slotIndex){
  ResetCounters(FindIndexBySlot(slotIndex));
}

void RepeatListClass::ResetCounters(int idx){
  if (idx >= 0){
    SetRptCounter(idx, 0);
    SetEveryCounter(idx, 0);
  }
}

// this returns true if we should call DoEvent()
bool RepeatListClass::IncRepeatCounterBySlot(int slotIndex){
  return IncRepeatCounter(FindIndexBySlot(slotIndex));
}

// this returns true if we should call DoEvent()
bool RepeatListClass::IncRepeatCounter(int idx){
  // if no repeat count is even set, return false
  if (idx < 0)
    return false;

  // if either everyCount or repeatCount is non-zero, there will
  // be a list-entry for that slot...
  //
  // NOTE: everyCount, if not present in the timeslot string, defaults to 0, "repeats off".
  
  int everyCount = GetEveryCount(idx);
  
  if (everyCount <= 0)
    return false;
  
  int everyCounter = GetEveryCounter(idx);

  bool bDoEvent;
  if (++everyCounter >= everyCount){
    bDoEvent = true;
    everyCounter = 0;
  }
  else
    bDoEvent = false;

  SetEveryCounter(idx, everyCounter);

  if (!bDoEvent)
    return false;

  int repeatCount = GetRptCount(idx);

  // if repeatCount is 0, repeat forever everyXYZ...
  if (repeatCount == 0)
    return true;

  int repeatCounter = GetRptCounter(idx);

  // finished repeatCount times
  if (repeatCounter >= repeatCount)
    return false;

  // incerment the repeat counter
  SetRptCounter(idx, ++repeatCounter);

  return true;
}

// return true if found and removed
bool RepeatListClass::RemoveIndexBySlot(int slotIndex){
  bool bFound = false;
  int count = GetCount();
  for (int ii = 0; ii < count; ii++){
    if (bFound)
      arr[ii-1] = arr[ii];
    else if (GetIndex(ii) == slotIndex)
      bFound = true;
  }
  if (bFound)
    SetSize(count-1);
  return bFound;
}

//-------------------------------------------------------------------------------------------
// return true if success
bool RepeatListClass::Add(int index, bool bStale, uint16_t repeatCount, uint16_t everyCount){
  int oldCount = GetCount();
  int newCount = oldCount+1;
  if (!SetSize(newCount))
    return false;
  SetIndex(oldCount, index);

  SetRptCount(oldCount, repeatCount);
  SetRptCounter(oldCount, 0);

  SetEveryCount(oldCount, everyCount);
  SetEveryCounter(oldCount, 0);

  SetStaleFlag(oldCount, bStale);
  return true;
}

// return -1 if not found
// or the 0-based index position in the list if found
int RepeatListClass::FindIndexBySlot(int slotIndex){
  int count = GetCount();
  for (int ii = 0; ii < count; ii++)
    if (GetIndex(ii) == slotIndex)
      return ii;
  return -1;
}

// return true if success
bool RepeatListClass::SetSize(int newSize){
  arr.resize(newSize);
  return (newSize == GetCount()) ? true : false;
}
