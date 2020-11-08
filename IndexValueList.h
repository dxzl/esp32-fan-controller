#ifndef IndexValueListH
#define IndexValueListH

#include <Arduino.h>

typedef struct {int index; uint16_t value;} t_indexValue;

// this is used to make a list of slots which have non-zero seconds-field, entered
// from p2.html. That allows us not to have to read every time-slot every second!
// We populate the list when preferences are loaded in setup(). We then add to the
// list when a new time-entry is made, if it has non-zero seconds. We remove from
// the list as we delete entries from p2.html.
// functions prefaced with IV_ work on this dynamic array...
int IV_GetIndex(int idx);
uint16_t IV_GetValue(int idx);
int IV_GetCount(void);
void IV_Clear(void);
void IV_SetIndex(int idx, int val);
void IV_SetValue(int idx, uint16_t val);
int IV_FindIndex(int idx);
bool IV_RemoveIndexBySlot(int idx);
bool IV_SetSize(int newSize);
bool IV_Add(int index, uint16_t value);

extern std::vector<t_indexValue> m_indexValueArray;

#endif
