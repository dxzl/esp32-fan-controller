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
#define TASK_RESTART      16
#define TASK_FIRMWARE_RESTART 17
#define TASK_WIFI_ON      18
#define TASK_WIFI_OFF     19
#define TASK_SYNC         20
#define TASK_LABEL_A      21
#define TASK_LABEL_B      22
#define TASK_SETTOKEN     23
#define TASK_SETCIPKEY    24
#define TASK_HTTPCALLBACK 25 // TaskHttpCallback()
#define TASK_MAIN_TIMING_CYCLE 26
#define TASK_PROCESS_ONE_SECOND_TIME_SLOTS 27
#define TASK_PROCESS_ONE_MINUTE_TIME_SLOTS 28
#define TASK_QUERY_MDNS_SERVICE 29
#define TASK_ENCODE_CHANGED_PARAMETERS 30
#define TASK_POLL_WIFI_SWITCH 31
#define TASK_CHECK_MDNS_SEARCH_RESULT 32
#define TASK_SEND_HTTP_REQ 33
#define TASK_SEND_CANRX_REQ 34
#define TASK_SHOW_PREVIOUS_SEND_RESULTS 35
#define TASK_PROCESS_RECEIVE_STRING 36 // might add subtasks to this...
#define TASK_PULSEOFF_TIMING_CYCLE 37
#define TASK_STATS_MONITOR 38
#define TASK_SET_PULSEOFF_VARS 39
#define TASK_WRITE_PULSE_EE_VALUES 40
#define TASK_PRINT_PREFERENCES 41
#define TASK_HTTPCB_DECODE_MAC 42
#define TASK_MAX_POWER 43
#define TASK_SET_TIMEDATE 44
#define TASK_SET_SNTP_TIMEZONE 45
#define TASK_SET_SNTP_INTERVAL 46
#define TASK_PROCESS_COMMANDS 47
#define TASK_PRINT_MDNS_INFO 48

// TASK_PARMS cycle-timing sub-tasks
#define SUBTASK_PERMAX    0
#define SUBTASK_PERUNITS  1
#define SUBTASK_PERVAL    2
#define SUBTASK_PHASE     3
#define SUBTASK_DCA       4
#define SUBTASK_DCB       5
#define SUBTASK_RELAY_A   6
#define SUBTASK_RELAY_B   7

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
