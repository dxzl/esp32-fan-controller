#ifndef TasksH
#define TasksH

#include <Arduino.h>
#include <vector>

#define TASK_PARMS        0 // this task has sub-tasks (below)
#define TASK_HOSTNAME     1
#define TASK_RECONNECT    2
#define TASK_MAC          3
#define TASK_RESET_PREFS  4
#define TASK_RESET_SLOTS  5
#define TASK_RESET_WIFI   6
#define TASK_WIFI_TOGGLE  7
#define TASK_WIFI_RESTORE 8
#define TASK_WIFI_AP_RESTORE 9
#define TASK_PAGE_REFRESH_REQUEST 10
#define TASK_WIFI_STA_CONNECT 11
#define TASK_WIFI_AP_CONNECT 12
#define TASK_MIDICHAN     13
#define TASK_MIDINOTE_A   14
#define TASK_MIDINOTE_B   15
#define TASK_MIDINOTE_C   16
#define TASK_MIDINOTE_D   17
#define TASK_RESTART      18
#define TASK_FIRMWARE_RESTART 19
#define TASK_WIFI_ON      20
#define TASK_WIFI_OFF     21
#define TASK_SYNC         22
#define TASK_LABEL_A      23
#define TASK_LABEL_B      24
#define TASK_LABEL_C      25
#define TASK_LABEL_D      26
#define TASK_SETTOKEN     27
#define TASK_SETCIPKEY    28
#define TASK_HTTPCALLBACK 29 // TaskHttpCallback()
#define TASK_MAIN_TIMING_CYCLE 30
#define TASK_PROCESS_ONE_SECOND_TIME_SLOTS 31
#define TASK_PROCESS_ONE_MINUTE_TIME_SLOTS 32
#define TASK_QUERY_MDNS_SERVICE 33
#define TASK_ENCODE_CHANGED_PARAMETERS 34
#define TASK_POLL_WIFI_SWITCH 35
#define TASK_CHECK_MDNS_SEARCH_RESULT 36
#define TASK_SEND_HTTP_REQ 37
#define TASK_SEND_CANRX_REQ 38
#define TASK_SHOW_PREVIOUS_SEND_RESULTS 39
#define TASK_PROCESS_RECEIVE_STRING 40 // might add subtasks to this...
#define TASK_PULSEOFF_TIMING_CYCLE 41
#define TASK_STATS_MONITOR 42
#define TASK_SET_PULSEOFF_VARS 43
#define TASK_WRITE_PULSE_EE_VALUES 44
#define TASK_PRINT_PREFERENCES 45
#define TASK_HTTPCB_DECODE_MAC 46
#define TASK_MAX_POWER 47
#define TASK_SET_TIMEDATE 48
#define TASK_SET_SNTP_TIMEZONE 49
#define TASK_SET_SNTP_INTERVAL 50
#define TASK_PROCESS_COMMANDS 51
#define TASK_PRINT_MDNS_INFO 52
#define TASK_POTCHANGE 53

// TASK_PARMS cycle-timing sub-tasks
#define SUBTASK_PERMAX    0
#define SUBTASK_PERUNITS  1
#define SUBTASK_PERVAL    2
#define SUBTASK_PHASEB    3
#define SUBTASK_PHASEC    4
#define SUBTASK_PHASED    5
#define SUBTASK_DCA       6
#define SUBTASK_DCB       7
#define SUBTASK_DCC       8
#define SUBTASK_DCD       9
#define SUBTASK_RELAY_A   10
#define SUBTASK_RELAY_B   11
#define SUBTASK_RELAY_C   12
#define SUBTASK_RELAY_D   13

class TasksClass{

  private:
  
    typedef struct{
      int iTask, i1, i2;
      String s1, s2, s3;
    } t_task;
    
    std::vector<t_task> tasks;

    int GetCount(void);
    bool SetSize(int newSize);
    int FindTask(int iTask);
    void DelIdx(int idx);

  public:
    void InitTasks();
    bool CancelTask(int iTask);
    bool QueueTask(int iTask, String s1);
    bool QueueTask(int iTask, String s1, String s2);
    bool QueueTask(int iTask, int i1=0, int i2=0, String s1="", String s2="", String s3="");
    bool RunTasks();
};

  void SubtaskProcessParams(int& subTask, int& iData);
  void TaskWiFiApConnect();
  void TaskWiFiStaConnect();
  void TaskMainTimingCycle();
  
  void TaskProcessMsgCommands(String& sInOut, String& sIp);
  void TaskProcessReceiveString(int& iIp, int& rxToken, String& sParam1, String& sParam2);
  void TaskResultsFromPreviousSend(int& iResultCode, String& sIp);
  void TaskEchoCommandToRemotes(char cCmd, String sData, String sRxIp);
  
  void TaskHttpCallback(int& iSpare, int& httpCode, String& sRsp, String& sMac, String& sTxIp);
  void SubHttpCB_CanRxOk(int& ipIdx, int& httpCode, String& sRsp, String& sTxIp);
  void SubHttpCB_CanRxDecodeFail(int& ipIdx, int& httpCode, String& sRsp, String& sTxIp);
  void SubHttpCB_CanRxFLNoTokenFL(int& ipIdx, int& httpCode, String& sRsp, String& sTxIp);

  void TaskHttpCB_DecodeMac(String& sMac, String& sTxIp);
#endif

extern TasksClass TSK;
