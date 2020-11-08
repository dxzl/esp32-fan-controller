#include "IndexValueList.h"

std::vector<t_indexValue> m_indexValueArray(0);

//-------------------------------------------------------------
// Functions for IndexValue Array (a C++ vector)
// Note: I had this implemented as a C++ Class but the
// object was being placed in RAM - this way, it's in flash.
//-------------------------------------------------------------
int IV_GetIndex(int idx) { return m_indexValueArray[idx].index; }
uint16_t IV_GetValue(int idx) { return m_indexValueArray[idx].value; }
int IV_GetCount(void) { return m_indexValueArray.size(); }
void IV_Clear(void) { m_indexValueArray.resize(0); }
void IV_SetIndex(int idx, int val) { m_indexValueArray[idx].index = val; }
void IV_SetValue(int idx, uint16_t val) { m_indexValueArray[idx].value = val; }

// return true if success
bool IV_Add(int index, uint16_t value)
{
  int oldCount = IV_GetCount();
  int newCount = oldCount+1;
  if (!IV_SetSize(newCount))
    return false;
  IV_SetIndex(oldCount, index);
  IV_SetValue(oldCount, value);
  return true;
}

// return -1 if not found
// or the 0-based index position in the list if found
int IV_FindIndex(int idx)
{
  int count = IV_GetCount();
  for (int ii = 0; ii < count; ii++)
    if (idx == IV_GetIndex(ii))
      return ii;
  return -1;
}

// return true if found and removed
bool IV_RemoveIndexBySlot(int idx)
{
  bool bFound = false;
  int count = IV_GetCount();
  for (int ii = 0; ii < count; ii++)
  {
    if (bFound)
      m_indexValueArray[ii-1] = m_indexValueArray[ii];
    else if (idx == IV_GetIndex(ii))
      bFound = true;
  }
  if (bFound)
    IV_SetSize(count-1);
  return bFound;
}

// return true if success
bool IV_SetSize(int newSize)
{
  m_indexValueArray.resize(newSize);
  return (newSize == IV_GetCount()) ? true : false;
}
