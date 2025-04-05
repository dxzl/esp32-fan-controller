// this file Tasks.cpp
#include "Gpc.h"

ChangeClass CNG;

// call at setup()
void ChangeClass::InitChanges(){
  changes.resize(0);
}

// returns false if queue full
bool ChangeClass::QueueChange(int iCmd, int iFlags, String sData){
  // add new...
  int oldCount = GetCount();
  int newCount = oldCount+1;
  if (!SetSize(newCount)){
    prtln("ERROR: CHANGE QUEUE FULL!!!!");
    return false;
  }
  changes[oldCount].iCmd = iCmd;
  changes[oldCount].iFlags = iFlags;
  changes[oldCount].sData = sData;
  return true;
}

// return -1 if not found, index if found
//int ChangeClass::FindChange(int iId){
//  int iCount = GetCount();
//  for (int i=0; i<iCount; i++)
//    if (changes[i].iId == iId)
//      return i;
//  return -1;
//}

// remove all copies of iCmd from task queue
// return true if any cancelled
//bool ChangeClass::CancelChange(int iCmd){
//  bool bRet = false;
//  for (int i=0; i < GetCount(); i++){
//    if (changes[i].iCmd == iCmd){
//      DelIdx(i--);
//      if (!bRet)
//        bRet = true;
//    }
//  }
//  return bRet;
//}

void ChangeClass::DelIdx(int idx){
  changes.erase(changes.begin() + idx);
}

// return true if success
bool ChangeClass::SetSize(int newSize)
{
  changes.resize(newSize);
  return (newSize == GetCount()) ? true : false;
}

int ChangeClass::GetCount(void) { return changes.size(); }

bool ChangeClass::RunChanges(){
  if (!GetCount()) return false;
  t_change* p = &changes[0]; // get pointer to oldest task...
  
  int iCmd = p->iCmd;
  int iFlags = p->iFlags;
  String sData = p->sData;

  DelIdx(0); // delete oldest task...
    
  bool bSaveInFlash = (iFlags & CD_FLAG_SAVE_IN_FLASH);
  
  switch(iCmd){
    case CD_CMD_TOKEN:
      if (alldigits(sData)){
        prtln("");
        int iToken = sData.toInt();
        if (iToken != NO_TOKEN && iToken >= MIN_TOKEN && iToken <= MAX_TOKEN){
          g_defToken = iToken;
          if (bSaveInFlash)
            TSK.QueueTask(TASK_SETTOKEN);
          prtln("ChangeClass::RunChanges(): CM_CMD_CMD_RECONNECT: New default token auto-set (it will restore to user's setting on powerup): " + String(g_defToken));
        }
        else
          prtln("ChangeClass::RunChanges(): CM_CMD_CMD_RECONNECT: range problem with iToken: " + String(iToken));
      }
    break;
    
    case CD_CMD_RECONNECT:
      if (sData == CD_RECONNECT_STRING){
        WiFiMonitorConnection(true); // disconnect (reconnect) (set in-motion globally after a MAC change)
        IML.Clear(); // erase all mDNS entries
        prtln("ChangeClass::RunChanges(): CM_CMD_CMD_RECONNECT: disconnecting/reconnecting...");
      }
    break;
    
    case CD_CMD_CYCLE_PARMS:
      if (!sData.isEmpty()){
        PerVals pv = {0};
        int iErr = StringToPerVals(sData, pv); 
        if (!iErr){
          g_perVals = pv;
          if (g8_ssr1ModeFromWeb != SSR_MODE_ON)
            SetSSR(GPOUT_SSR1, false); // turn off A
          if (g8_ssr2ModeFromWeb != SSR_MODE_ON)
            SetSSR(GPOUT_SSR2, false); // turn off B
          ResetPeriod();
          prtln("ChangeClass::RunChanges(): CD_CMD_CYCLE_PARMS: g_perVals changed, cycle reset!");
        }
      }
    break;
    
    default:
    break;
  };
  return true;
}

// ------------------------ non-class-functions --------------------------------
