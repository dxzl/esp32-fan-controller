#ifndef ChangeH
#define ChangeH

#include <Arduino.h>
#include <vector>

// CMchangeData has two base64 encoded uint32_t separated by CM_SEP. The first is a command. In the command, the
// low FLAG_COUNT_CD bits are reserved for flag-bits. The second is a full 32-bits which can be used for a MAC or IP
// address.
// Commands (up to 28 bits worth!)
#define CD_CMD_EMPTY       0 // idle state
#define CD_CMD_TOKEN       1
#define CD_CMD_RECONNECT   2
#define CD_CMD_CYCLE_PARMS 3 // period, phase, duty-cycle A, duty-cycle B
// Flag-bit masks (0-3)
#define CD_FLAG_SAVE_IN_FLASH (1<<0)
#define CD_FLAG_unused1       (1<<1)
#define CD_FLAG_unused2       (1<<2)
#define CD_FLAG_unused3       (1<<3)
#define FLAG_COUNT_CD         4

#define CD_DATA_PLACE_HOLDER '0' // used when data string for AddCMchangeReq is empty!

#define CD_RECONNECT_STRING "jeLidhE" // magic-string for reconnect command (can be anything)

#define CD_MAX_DATA_STRING_LENGTH 1024 // max decoded characters that can follow any given command (arbitrary!)

class ChangeClass{

  private:
  
    typedef struct{
      int iCmd, iFlags;
      String sData;
    } t_change;
    
    std::vector<t_change> changes;

    bool SetSize(int newSize);
//    int FindChange(int iId);
    void DelIdx(int idx);

  public:
    int GetCount(void);
    void InitChanges();
//    bool CancelChange(int iId);
    bool QueueChange(int iCmd, int iFlags, String sData);
    bool RunChanges();
};

#endif

extern ChangeClass CNG;
