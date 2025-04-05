// this file MdnsListClass.cpp
#include "Gpc.h"

MdnsListClass IML;

// https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/protocols/mdns.html
//-------------------------------------------------------------
// Functions for MdnsListClass (a C++ vector)
//-------------------------------------------------------------
int MdnsListClass::GetToken(int idx, int iToken) { return arr[idx].tokens[iToken]; }
void MdnsListClass::SetToken(int idx, int iToken, int val) { arr[idx].tokens[iToken] = val; }

uint16_t MdnsListClass::GetMdnsMAClastTwo(int idx) { return arr[idx].mac_last_two_octets; }
void MdnsListClass::SetMdnsMAClastTwo(int idx, uint16_t val) { arr[idx].mac_last_two_octets = val; }
String MdnsListClass::GetStr(int idx, int iString) { return arr[idx].strings[iString]; }
void MdnsListClass::SetStr(int idx, int iString, String val) { arr[idx].strings[iString] = val; }

bool MdnsListClass::GetFlag(int idx, int iFlag){ return arr[idx].flags[iFlag]; }
void MdnsListClass::SetFlag(int idx, int iFlag, bool val){ arr[idx].flags[iFlag] = val; }
void MdnsListClass::SetFlag(IPAddress ipFind, int iFlag, bool val){
  int idx = FindMdnsIp(ipFind);
  if (idx >= 0)
    SetFlag(idx, iFlag, val);
}
int MdnsListClass::GetSendCount(int idx) { return arr[idx].sendCount; }
void MdnsListClass::SetSendCount(int idx, int val) { arr[idx].sendCount = val; }
int MdnsListClass::GetStatus(int idx) { return arr[idx].devStatus; }
void MdnsListClass::SetStatus(int idx, int val) { arr[idx].devStatus = val; }
int MdnsListClass::GetMinute(int idx) { return arr[idx].iMin; }
void MdnsListClass::SetMinute(int idx, int val) { arr[idx].iMin = val; }
IPAddress MdnsListClass::GetIP(int idx) { return arr[idx].ip; }
void MdnsListClass::SetIP(int idx, IPAddress val) { arr[idx].ip = val; }
int MdnsListClass::GetCount(void) { return arr.size(); }
void MdnsListClass::Clear(void) { arr.resize(0); }
//-------------------------------------------------------------

bool MdnsListClass::IsGoodIndex(int idx){
  int count = GetCount();
  if (!count || idx < 0 || idx >= count)
    return false;
  return true;
}

bool MdnsListClass::IsFlagSetForAnyIP(int iFlag){
  int count = GetCount();
  for (int ii=0; ii<count; ii++){
    if (GetFlag(ii, iFlag))
      return true;
  }
  return false;  
}

bool MdnsListClass::IsFlagSetForAllIP(int iFlag){
  int count = GetCount();
  for (int ii=0; ii<count; ii++){
    if (!GetFlag(ii, iFlag))
      return false;
  }
  return true;  
}

void MdnsListClass::SetFlagForAllIP(int iFlag, bool val){
  int count = GetCount();
  for (int ii=0; ii<count; ii++)
    SetFlag(ii, iFlag, val);
}

void MdnsListClass::XferRxTokens(int idx){
  if (arr[idx].tokens[MDNS_TOKEN_RX] != NO_TOKEN)
    arr[idx].tokens[MDNS_TOKEN_RXPREV] = arr[idx].tokens[MDNS_TOKEN_RX];
  if (arr[idx].tokens[MDNS_TOKEN_RXNEXT] != NO_TOKEN){
    arr[idx].tokens[MDNS_TOKEN_RX] = arr[idx].tokens[MDNS_TOKEN_RXNEXT];
    arr[idx].tokens[MDNS_TOKEN_RXNEXT] = NO_TOKEN;
  }
}

void MdnsListClass::XferTxTokens(int idx){
  if (arr[idx].tokens[MDNS_TOKEN_TX] != NO_TOKEN)
    arr[idx].tokens[MDNS_TOKEN_TXPREV] = arr[idx].tokens[MDNS_TOKEN_TX];
  if (arr[idx].tokens[MDNS_TOKEN_TXNEXT] != NO_TOKEN){
    arr[idx].tokens[MDNS_TOKEN_TX] = arr[idx].tokens[MDNS_TOKEN_TXNEXT];
    arr[idx].tokens[MDNS_TOKEN_TXNEXT] = NO_TOKEN;
  }
}

void MdnsListClass::InitTokens(int idx){
  for (int ii=0; ii<TOKEN_COUNT_MDNS; ii++)
    arr[idx].tokens[ii] = NO_TOKEN;
}

void MdnsListClass::InitFlags(int idx){
  for (int ii=0; ii<FLAG_COUNT_MDNS; ii++)
    arr[idx].flags[ii] = false;
}

void MdnsListClass::InitStrings(int idx){
  for (int ii=0; ii<STRING_COUNT_MDNS; ii++)
    arr[idx].strings[ii] = "";
}

// returns index of newly added IP or negative if error
int MdnsListClass::AddMdnsIp(String sIP){
  IPAddress ipNew;
  ipNew.fromString(sIP);
  if ((uint32_t)ipNew == 0)
    return -1;
  return AddMdnsIp(ipNew);
}

// returns index of newly added IP or negative if error
int MdnsListClass::AddMdnsIp(IPAddress ipNew){
  
  int iMin = millis()/60000;
  int ii = 0;
  int count = GetCount();

  for (; ii<count; ii++){
    if (GetIP(ii) == ipNew){
      // found pre-existing ip? update time, bLinkOk Etc. and return
      SetMinute(ii, iMin);
      if (!GetFlag(ii, MDNS_FLAG_LINK_OK)){
        SetFlag(ii, MDNS_FLAG_LINK_OK, true);
        RestorePrevTokens(ii);
      }
      return ii;
    }
  }

  // full buffer? Delete first entry that has the bLinkOk flag clear...
  if (count >= MDNS_ASYNC_SEARCH_MAX_COUNT){
    count = DeleteOldestNoLinkMdnsEntry();
    if (count >= MDNS_ASYNC_SEARCH_MAX_COUNT)
      return -2; // full
  }

  // ipNew not in list? add it!
  if (!SetSize(count+1))
    return -3;
    
  arr[count].ip = ipNew;
  
  // we store current minute and delete this mDNS IP entry if not refreshed within 10 min.
  arr[count].iMin = iMin;
  
  arr[count].mac_last_two_octets = 0; // set this later via IML.SetMdnsMAClastTwo()
  arr[count].devStatus = 0; // up to 32 bits device status per unit
  
  InitTokens(count);
  InitFlags(count);
  InitStrings(count);
  
  // initialize with a request for this IP's last-two MAC address octets
  // (NOTE: Two MACs are now exchanged via the SendHttpCanRxReq() query initiated by NO_TOKEN in Rx/Tx mDNS
  // slot and passed to the callback as an added header!)
  //arr[count].sSendAll = HMC.AddRangeCommand(CMreqMacMin, CMreqMacMax);

  SetFlag(count, MDNS_FLAG_LINK_OK, true);

  if (g_bMaster){
    g_bMaster = false; // master-status needs re-evaluation with addition of new remote device!
    prtln("temporarily revoking master-status - new remote detected...");
  }

#if GPC_BOARD_3B || GPC_BOARD_2C || GPC_BOARD_3C
  g_oldDevStatus = ~g_actualStatus;
#else
  g_oldDevStatus = ~g_devStatus;
#endif
  
  // to force additional parameters to be sent the first time...
  //g_oldPerVals.dutyCycleA = 0xff;
  //g_oldPerVals.dutyCycleB = 0xff;
  //g_oldPerVals.dutyCycleC = 0xff;
  //g_oldPerVals.dutyCycleD = 0xff;
  //g_oldPerVals.phaseB = 0xff;
  //g_oldPerVals.phaseC = 0xff;
  //g_oldPerVals.phaseD = 0xff;
  //g_oldPerVals.perVal = 0xff;
  //g_oldPerVals.perUnits = 0xff;
  //g_oldPerVals.perMax = 0xffff;

  g16_sendDefTokenTimer = 0;
  
  return count;
}

void MdnsListClass::RestorePrevTokens(int idx){
  arr[idx].tokens[MDNS_TOKEN_RX] = arr[idx].tokens[MDNS_TOKEN_RXPREV];
  arr[idx].tokens[MDNS_TOKEN_TX] = arr[idx].tokens[MDNS_TOKEN_TXPREV];
}

bool MdnsListClass::ClearMdnsSendInfo(IPAddress ipClear){
  return ClearMdnsSendInfo(FindMdnsIp(ipClear));
}
bool MdnsListClass::ClearMdnsSendInfo(int idx){
  if (idx < 0 || idx >= GetCount())
    return false;
  if (g_bMaster){
    SetStr(idx, MDNS_STRING_SEND_ALL, "");
    SetStr(idx, MDNS_STRING_SEND_SPECIFIC, "");
  }
  else{
    // if we are not the master then we only SendHttpReq() to the master. In doing this,
    // we combined all sSendSpecific into CMto bundled commands and sent them together with
    // sSendAll for the master's mDNS entry. So we need to clear ALL sSend strings!
    int iCount = GetCount();
    for (int ii=0; ii < iCount; ii++){
      SetStr(ii, MDNS_STRING_SEND_ALL, "");
      SetStr(ii, MDNS_STRING_SEND_SPECIFIC, "");
    }
  }
  SetSendCount(idx, 0);
  return true;
}

// returns true if found and removed
bool MdnsListClass::DelMdnsIp(IPAddress ipDel){
  int iFound = FindMdnsIp(ipDel);
  if (iFound >= 0)
    if (DelMdnsIp(iFound))
      return true;
  return false;
}

bool MdnsListClass::DelMdnsIp(int idx){
  int count = GetCount();
  if (!count || idx < 0 || idx >= count)
    return false;
  for (int ii=idx; ii+1<count; ii++)
    arr[ii] = arr[ii+1];
  count--;
  SetSize(count);
  RefreshGlobalMasterFlagAndIp();
  g16_sendDefTokenTimer = 0;
  g_prevMdnsCount = 0;
  return true;
}

// returns index if found, -1 if not found
int MdnsListClass::FindMdnsIp(uint32_t iIp){
  if (!iIp)
    return -1;
  return FindMdnsIp(IPAddress(iIp));
}

// returns index if found, -1 if not found
int MdnsListClass::FindMdnsIp(String sIP){
  IPAddress ip;
  ip.fromString(sIP);
  if ((uint32_t)ip == 0)
    return -1;
  return FindMdnsIp(ip);
}

int MdnsListClass::FindMdnsIp(IPAddress ipFind){
  int count = GetCount();
  for (int ii=0; ii<count; ii++)
    if (ipFind == arr[ii].ip)
      return ii;
  return -1;
}

// if mDNS not detected for a long time, delete the entry for that unit...
void MdnsListClass::DeleteExpiredMdnsIps(){
  int iMin = millis()/60000;
  int count = GetCount();
  for (int ii=0; ii<count; ii++){
    int delta = iMin - GetMinute(ii);
    if (delta < 0)
      delta = -delta;
    // we link timeout in minutes to (count/2)+2
    // this is because the more units there are, the longer it takes to recheck for mDNS on a particular unit
    // we add/refresh a unit every 30-sec, so for 10 units it might take 5 minutes to refresh our unit and update u8min
    if (delta > (count/2)+2)
      DelMdnsIp(ii);
  }
}

// delete oldest mDNS entry with link down...
// returns the new count of remote ESP32 units
int MdnsListClass::DeleteOldestNoLinkMdnsEntry(){
  int count = GetCount();
  for (int ii=0; ii<count; ii++)
    if (!GetFlag(ii, MDNS_FLAG_LINK_OK))
      if (DelMdnsIp(ii))
        break;
  return GetCount();
}

// return true if success
bool MdnsListClass::SetSize(int newSize)
{
  arr.resize(newSize);
  return (newSize == GetCount()) ? true : false;
}

void MdnsListClass::PrintMdnsInfo(){
  int count = GetCount();
  if (count){
    prtln("Count of other ESP32s found: " + String(count));
    int iMaster = -1;
    if (g_bMaster)
      prtln("!!!THIS UNIT IS THE MASTER!!!");
    else
      GetHighestMac(iMaster);
    for (int ii=0; ii < count; ii++){
      String sTemp = GetFlag(ii, MDNS_FLAG_LINK_OK) ? "UP" : "DOWN";
      prtln("--- index: " + String(ii) + ", link is " + sTemp);
      sTemp = "";
      String sSendAll = GetStr(ii, MDNS_STRING_SEND_ALL);
      if (!sSendAll.isEmpty())
        sTemp += " sSendAll=\"" + CommandStrToPrintable(sSendAll) + "\"";
      String sSendSpecific = GetStr(ii, MDNS_STRING_SEND_SPECIFIC);
      if (!sSendSpecific.isEmpty())
        sTemp += " sSendSpecific=\"" + CommandStrToPrintable(sSendSpecific) + "\"";
      if (!sTemp.isEmpty())
        prtln(sTemp);
      sTemp = (ii == iMaster) ? ", !!!MASTER!!!" : "";
      prtln("--- ip: " + GetIP(ii).toString() + ", macLastTwo: " + String(GetMdnsMAClastTwo(ii)) + sTemp);
      prtln("--- rxToken: " + String(GetToken(ii, MDNS_TOKEN_RX)) + ", txToken: " + String(GetToken(ii, MDNS_TOKEN_TX)));
      prtln("--- rxPrevToken: " + String(GetToken(ii, MDNS_TOKEN_RXPREV)) + ", txPrevToken: " + String(GetToken(ii, MDNS_TOKEN_TXPREV)));
      prtln("--- status flags: 0x" + String(GetStatus(ii), HEX));
    }
  }
  else
    prtln("no other ESP32s in mDNS list!");
}

/**
 * @brief Get results from search pointer. Results available as a pointer to the output parameter.
 *        Pointer to search object has to be deleted via `mdns_query_async_delete` once the query has finished.
 *        The results although have to be freed manually.
 *
 * @param search pointer to search object
 * @param timeout time in milliseconds to wait for answers
 * @param results pointer to the results of the query
 * @param num_results pointer to the number of the actual result items (set to NULL to ignore this return value)
 *
 * @return
 *      True if search has finished before or at timeout
 *      False if search timeout is over
 */
bool MdnsListClass::CheckMdnsSearchResult(){
  // pSearch is a reference to a pointer...
  if (!g_pMdnsSearch)
    return false;

  // Check if any result is available (Note: mdns_result_t and mdns_result_s are typedefed the same at present!)
  // struct mdns_result_s *next - next result, or NULL for the last result in the list
  // mdns_if_ttcpip_if - interface index
  // mdns_ip_protocol_tip_protocol - ip_protocol type of the interface (v4/v6)
  // char *instance_name - instance name
  // char *hostname - hostname
  // uint16_t port - service port
  // mdns_txt_item_t *txt - txt record
  // size_t txt_count - number of txt items
  // mdns_ip_addr_t *addr - linked list of IP addresses found
  mdns_result_t* result = NULL;
  //bool mdns_query_async_get_results(mdns_search_once_t* search, uint32_t timeout, mdns_result_t ** results, uint8_t * num_results);
  if (!mdns_query_async_get_results(g_pMdnsSearch, 0, &result, NULL)) // 0 is no timeout.
    return false;

  if (result){
    // If yes, store ip addresses in array by reference...
    mdns_ip_addr_t* a = result->addr;
    while (a){
      //The IP2STR macro expands to 4 integers that represent the octets of the IP address, separated by commas; not a string.
      //The IPSTR macro expands to "%d.%d.%d.%d".
      if(a->addr.type == ESP_IPADDR_TYPE_V4)
        if (AddMdnsIp(IPAddress(IP2STR(&(a->addr.u_addr.ip4)))) < 0)
          break; // stop adding entries if array full
      
//      if (r->txt_count){
//        printf("  TXT : [%zu] ", r->txt_count);
//        for (t = 0; t < r->txt_count; t++)
//          printf("%s=%s(%d); ", r->txt[t].key, r->txt[t].value ? r->txt[t].value : "NULL", r->txt_value_len[t]);
//        printf("\n");
//      }
        
//      if(a->addr.type == ESP_IPADDR_TYPE_V6)
//        printf("  AAAA: " IPV6STR "\n", IPV62STR(a->addr.u_addr.ip6));
//      else
//        printf("  A   : " IPSTR "\n", IP2STR(&(a->addr.u_addr.ip4)));

      a = a->next;
    }
    // and free the result
    mdns_query_results_free(result);
//    ESP_LOGI(TAG, "Query A %s.local finished", host_name);
  }

  mdns_query_async_delete(g_pMdnsSearch);
  g_pMdnsSearch = NULL; // set to NULL by reference!

  return true; // also returns true if timeout (result was NULL)
}

/**
 * @brief  Remove and free all services from mDNS server
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
//esp_err_t mdns_service_remove_all(void);
/**
 * @brief Deletes the finished query. Call this only after the search has ended!
 *
 * @param search pointer to search object
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_STATE  search has not finished
 *     - ESP_ERR_INVALID_ARG    pointer to search object is NULL
 */
//esp_err_t mdns_query_async_delete(mdns_search_once_t* search);
/**
 * @brief  Query mDNS for host or service asynchronousely.
 *         Search has to be tested for progress and deleted manually!
 *
 * @param  name         service instance or host name (NULL for PTR queries)
 * @param  service_type service type (_http, _arduino, _ftp etc.) (NULL for host queries)
 * @param  proto        service protocol (_tcp, _udp, etc.) (NULL for host queries)
 * @param  type         type of query (MDNS_TYPE_*)
 * @param  timeout      time in milliseconds during which mDNS query is active
 * @param  max_results  maximum results to be collected
 * @param  notifier     Notification function to be called when the result is ready, can be NULL
 *
 * @return mdns_search_once_s pointer to new search object if query initiated successfully.
 *         NULL otherwise.
 */
//mdns_search_once_t *mdns_query_async_new(const char* name, const char* service_type, const char* proto, uint16_t type,
//                                         uint32_t timeout, size_t max_results, mdns_query_notify_t notifier);

// don't forget to prepend an underscore char to service name!
// pSearch is a reference to a pointer...
void MdnsListClass::QueryMdnsServiceAsync(const char* service_type){
  if (g_pMdnsSearch) // return if previous search not completed
    return;

  //ESP_LOGI(TAG, "Query A service: %s", service_type);
  //MDNS_TYPE_A
  //MDNS_TYPE_PTR
  //MDNS_TYPE_TXT
  //MDNS_TYPE_AAAA
  //MDNS_TYPE_SRV
  //MDNS_TYPE_OPT
  //MDNS_TYPE_NSEC
  //MDNS_TYPE_ANY
  g_pMdnsSearch = mdns_query_async_new(NULL, service_type, "_tcp", MDNS_TYPE_PTR, MDNS_ASYNC_SEARCH_TIMEOUT, MDNS_ASYNC_SEARCH_MAX_COUNT, NULL);
//  g_pMdnsSearch = mdns_query_async_new(NULL, service_type, "_tcp", MDNS_TYPE_A, MDNS_ASYNC_SEARCH_TIMEOUT, MDNS_ASYNC_SEARCH_MAX_COUNT, NULL);
  //ESP_LOGI(TAG, "Query AAAA service: %s", service_type);
  //g_pMdnsSearch = mdns_query_async_new(NULL, service_type, "_tcp", MDNS_TYPE_AAAA, MDNS_ASYNC_SEARCH_TIMEOUT, MDNS_ASYNC_SEARCH_MAX_COUNT, NULL);

// put this in .25 sec handler to poll
//  if (check_and_print_result(g_pMdnsSearch))
//    prtln("Query " + String(service_type) + " finished");
}

//void QueryHostServiceAsync(const char* host_name){
//  if (g_pMdnsSearch) // return if previous search not completed
//    return;
//
//  //ESP_LOGI(TAG, "Query A hosts: %s", host_name);
//  //MDNS_TYPE_A
//  //MDNS_TYPE_PTR
//  //MDNS_TYPE_TXT
//  //MDNS_TYPE_AAAA
//  //MDNS_TYPE_SRV
//  //MDNS_TYPE_OPT
//  //MDNS_TYPE_NSEC
//  //MDNS_TYPE_ANY
//  g_pMdnsSearch = mdns_query_async_new(host_name, NULL, NULL, MDNS_TYPE_A, MDNS_ASYNC_SEARCH_TIMEOUT, MDNS_ASYNC_SEARCH_MAX_COUNT, NULL);
//  //ESP_LOGI(TAG, "Query AAAA hosts: %s", host_name);
//  //g_pMdnsSearch = mdns_query_async_new(host_name, NULL, NULL, MDNS_TYPE_AAAA, MDNS_ASYNC_SEARCH_TIMEOUT, MDNS_ASYNC_SEARCH_MAX_COUNT, NULL);
//
//// put this in .25 sec handler to poll
////  if (check_and_print_result(g_pMdnsSearch))
////    prtln("Query " + String(host_name) + " finished");
//}


// FYI, Example
//static void query_mdns_hosts_async(const char * host_name)
//{
//    ESP_LOGI(TAG, "Query both A and AAA: %s.local", host_name);
//
//    mdns_search_once_t *s_a = mdns_query_async_new(host_name, NULL, NULL, MDNS_TYPE_A, 1000, 1, NULL);
//    mdns_search_once_t *s_aaaa = mdns_query_async_new(host_name, NULL, NULL, MDNS_TYPE_AAAA, 1000, 1, NULL);
//    while (s_a || s_aaaa) {
//        if (s_a && check_and_print_result(s_a)) {
//            ESP_LOGI(TAG, "Query A %s.local finished", host_name);
//            mdns_query_async_delete(s_a);
//            s_a = NULL;
//        }
//        if (s_aaaa && check_and_print_result(s_aaaa)) {
//            ESP_LOGI(TAG, "Query AAAA %s.local finished", host_name);
//            mdns_query_async_delete(s_aaaa);
//            s_aaaa = NULL;
//        }
//        vTaskDelay(50 / portTICK_PERIOD_MS);
//    }
//}
