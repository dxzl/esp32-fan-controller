// this file ValueListClass.cpp
#include "Gpc.h"

ValueListClass IVL;

//-------------------------------------------------------------
// Functions for ValueListClass (a C++ vector)
//-------------------------------------------------------------
int ValueListClass::GetIndex(int idx) { return arr[idx].index; }
void ValueListClass::SetIndex(int idx, int val) { arr[idx].index = val; }
uint16_t ValueListClass::GetValue(int idx) { return arr[idx].value; }
void ValueListClass::SetValue(int idx, uint16_t val) { arr[idx].value = val; }
int ValueListClass::GetCount(void) { return arr.size(); }
void ValueListClass::Clear(void) { arr.resize(0); }

// return true if success
bool ValueListClass::Add(int index, uint16_t value)
{
  int oldCount = GetCount();
  int newCount = oldCount+1;
  if (!SetSize(newCount))
    return false;
  SetIndex(oldCount, index);
  SetValue(oldCount, value);
  return true;
}

// return -1 if not found
// or the 0-based index position in the list if found
int ValueListClass::FindIndex(int idx)
{
  int count = GetCount();
  for (int ii = 0; ii < count; ii++)
    if (idx == GetIndex(ii))
      return ii;
  return -1;
}

// return true if found and removed
bool ValueListClass::RemoveIndexBySlot(int idx)
{
  bool bFound = false;
  int count = GetCount();
  for (int ii = 0; ii < count; ii++)
  {
    if (bFound)
      arr[ii-1] = arr[ii];
    else if (idx == GetIndex(ii))
      bFound = true;
  }
  if (bFound)
    SetSize(count-1);
  return bFound;
}

// return true if success
bool ValueListClass::SetSize(int newSize)
{
  arr.resize(newSize);
  return (newSize == GetCount()) ? true : false;
}
