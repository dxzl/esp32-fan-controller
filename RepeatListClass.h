#ifndef IndexRepeatListH
#define IndexRepeatListH

#include <Arduino.h>
#include <vector>

// this is used to make a list of slots which have non-zero seconds-field, entered
// from p2.html. That allows us not to have to read every time-slot every second!
// We populate the list when preferences are loaded in setup(). We then add to the
// list when a new time-entry is made, if it has non-zero seconds. We remove from
// the list as we delete entries from p2.html.
class RepeatListClass{
  private:

    typedef struct{
      int index;
      bool bStale;
      uint16_t rptCount;
      uint16_t rptCounter;
      uint16_t everyCount;
      uint16_t everyCounter;
    } t_indexRepeat;
    
    std::vector<t_indexRepeat> arr;

  public:
    int GetIndex(int idx);
    void SetIndex(int idx, int val);
    uint16_t GetRptCount(int idx);
    void SetRptCount(int idx, uint16_t val);
    uint16_t GetRptCounter(int idx);
    void SetRptCounter(int idx, uint16_t val);
    uint16_t GetEveryCount(int idx);
    void SetEveryCount(int idx, uint16_t val);
    uint16_t GetEveryCounter(int idx);
    void SetEveryCounter(int idx, uint16_t val);
    bool GetStaleFlag(int idx);
    void SetStaleFlag(int idx, bool val);
    int GetCount(void);
    void Clear(void);
    bool SetSize(int newSize);
    bool Add(int index, bool bStale, uint16_t repeatCount, uint16_t everyCount);
    
    int FindIndexBySlot(int slotIndex);
    
    bool RemoveIndexBySlot(int slotIndex);
    
    void ResetCountersBySlot(int slotIndex);
    void ResetCounters(int idx);
    
    bool IncRepeatCounterBySlot(int slotIndex);
    bool IncRepeatCounter(int idx);
    
    bool IsStaleBySlot(int slotIndex);
    bool IsStale(int idx);
};

#endif

extern RepeatListClass IRL;
