#ifndef IndexValueListH
#define IndexValueListH

#include <Arduino.h>
#include <vector>

// this is used to make a list of slots which have non-zero seconds-field, entered
// from p2.html. That allows us not to have to read every time-slot every second!
// We populate the list when preferences are loaded in setup(). We then add to the
// list when a new time-entry is made, if it has non-zero seconds. We remove from
// the list as we delete entries from p2.html.
class ValueListClass{

  private:
  
    typedef struct{
      int index;
      uint16_t value;
    } t_indexValue;

    std::vector<t_indexValue> arr;

  public:
    int GetIndex(int idx);
    uint16_t GetValue(int idx);
    int GetCount(void);
    void Clear(void);
    void SetIndex(int idx, int val);
    void SetValue(int idx, uint16_t val);
    int FindIndex(int idx);
    bool RemoveIndexBySlot(int idx);
    bool SetSize(int newSize);
    bool Add(int index, uint16_t value);
};

#endif

extern ValueListClass IVL;
