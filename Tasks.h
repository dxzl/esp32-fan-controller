#ifndef TasksH
#define TasksH

#include <Arduino.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <ESPmDNS.h>

#define MAX_TASKS 20 // need enough to change all timing parameters at once for remote HTTP_ASYNCREQ_PARAM_COMMAND

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
#define TASK_CYCLE_THROUGH_MDNS_IPS 33
#define TASK_PULSEOFF_TIMING_CYCLE 34
#define TASK_STATS_MONITOR 35
#define TASK_SET_PULSEOFF_VARS 36
#define TASK_WRITE_PULSE_EE_VALUES 37
#define TASK_PRINT_PREFERENCES 38
#define TASK_HTTPCB_DECODE_MAC 39
#define TASK_SEND_CANRX_QUERY 40
#define TASK_MAX_POWER 41
#define TASK_NONE 255

// TASK_PARMS cycle-timing sub-tasks
#define SUBTASK_PERMAX    0
#define SUBTASK_PERUNITS  1
#define SUBTASK_PERVAL    2
#define SUBTASK_PHASE     3
#define SUBTASK_DCA       4
#define SUBTASK_DCB       5
#define SUBTASK_RELAY_A   6
#define SUBTASK_RELAY_B   7

typedef struct{
  int iTask, i1, i2;
  String s1, s2, s3;
} t_task;

void InitTasks();
bool CancelTask(int iTask);
int FindTask(int iTask);
bool QueueTask(int iTask, String s1);
bool QueueTask(int iTask, int i2=0, int i3=0, String s1="", String s2="", String s3="");
t_task* GetFirstFullTaskSlot();
void RunTasks();

void TaskSendCanRxQuery(String sIp);

void SubtaskProcessParams(int& subTask, int& iData);
void TaskWiFiApConnect();
void TaskWiFiStaConnect();
void TaskMainTimingCycle();

void TaskHttpCallback(int& iSpare, int& httpCode, String& sRsp, String& sMac, String& sTxIp);
void SubHttpCB_CanRxOk(int& ipIdx, int& httpCode, String& sRsp, String& sTxIp);
void SubHttpCB_CanRxDecodeFail(int& ipIdx, int& httpCode, String& sRsp, String& sTxIp);
void SubHttpCB_ParamOkTxtOkTokOkTimesetFail(int& ipIdx, int& httpCode, String& sRsp, String& sTxIp);
void SubHttpCB_DecFLDecPrevFLParamFLTxtFlCanRxFLNoTokenFL(int& ipIdx, int& httpCode, String& sRsp, String& sTxIp);

void TaskHttpCB_DecodeMac(String& sMac, String& sTxIp);
#endif
