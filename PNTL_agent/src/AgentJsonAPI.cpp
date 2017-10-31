//
// Created by zy on 17-9-15.
//
#include "AgentJsonAPI.h"
#include <boost/algorithm/string.hpp>
#include "Log.h"

using namespace std;
using namespace boost;
/*
{
"LogCfg" :
    {
                "LOG_DIR"       : "/opt/huawei/logs/ServerAntAgent"
    },
"ServerAntServer" :
    {
        "IP"    : "8.15.4.11",
        "Port"  : 8888
    },
"ServerAntAgent" :
    {
        "AgentIP"    : "0.0.0.0",
        "MgntIP"     : "0.0.0.0",
        "Hostname"      :       "SZV1000278559",
        "Port"  : 31001,

        "PollingTimerPeriod"    : 100000,
        "ReportPeriod"          : 300,
        "QueryPeriod"           : 120,
        "DetectPeriod"          : 60,
        "DetectTimeoutPeriod"   : 1,
        "DetectDropThresh"      : 2,

        "ProtocolUDP" :
            {
                "DestPort"  : 6000,
                "SrcPortMin": 32769,
                "SrcPortMax": 32868
            }
    }
}
*/
// 解析Agent本地配置文件, 完成初始化配置.
INT32 ParserLocalCfg(const char *pcJsonData, ServerAntAgentCfg_C *pcCfg) {
    INT32 iRet = AGENT_OK;
    UINT32 uiIp, uiPort, uiData;
    string strTemp;
    // boost::property_tree对象, 用于存储json格式数据.
    ptree ptDataRoot, ptDataTmp;

    // boost库中出现错误会抛出异常, 未被catch的异常会逐级上报, 最终导致进程abort退出.
    try {
        // pcData字符串转存stringstream格式, 方便后续boost::property_tree处理.
        stringstream ssStringData(pcJsonData);
        read_json(ssStringData, ptDataRoot);

        // 解析LogCfg数据.
        ptDataTmp.clear();
        ptDataTmp = ptDataRoot.get_child("LogCfg");
        strTemp = ptDataTmp.get<string>("LOG_DIR");
        iRet = SetNewLogDir(strTemp);
        if (iRet) {
            JSON_PARSER_ERROR("SetNewLogDir failed[%d]", iRet);
            return iRet;
        }

        // 解析ServerAntServer数据.
        ptDataTmp.clear();
        ptDataTmp = ptDataRoot.get_child("ServerAntServer");
        strTemp = ptDataTmp.get<string>("IP");
        uiIp = sal_inet_aton(strTemp.c_str());
        uiPort = ptDataTmp.get<UINT32>("Port");
        pcCfg->SetServerAddress(uiIp, uiPort);

        // 解析ServerAntAgent数据.
        ptDataTmp.clear();
        ptDataTmp = ptDataRoot.get_child("ServerAntAgent");
        strTemp = ptDataTmp.get<string>("MgntIP");
        uiIp = sal_inet_aton(strTemp.c_str());
        pcCfg->SetMgntIP(uiIp);


        strTemp = ptDataTmp.get<string>("AgentIP");
        uiIp = sal_inet_aton(strTemp.c_str());
        pcCfg->SetAgentAddress(uiIp);

        uiData = ptDataTmp.get<UINT32>("ReportPeriod");
        pcCfg->SetReportPeriod(uiData);

        uiData = ptDataTmp.get<UINT32>("QueryPeriod");
        pcCfg->SetQueryPeriod(uiData);

        uiData = ptDataTmp.get<UINT32>("DetectPeriod");
        pcCfg->SetDetectPeriod(uiData);

        uiData = ptDataTmp.get<UINT32>("DetectTimeoutPeriod");
        pcCfg->SetDetectTimeout(uiData);

        uiData = ptDataTmp.get<UINT32>("DetectDropThresh");
        pcCfg->SetDetectDropThresh(uiData);

        // 解析ServerAntAgent.ProtocolUDP数据.
        ptDataTmp.clear();
        ptDataTmp = ptDataRoot.get_child("ServerAntAgent.ProtocolUDP");
        UINT32 uiSrcPortMin = ptDataTmp.get<UINT32>("SrcPortMin");
        UINT32 uiSrcPortMax = ptDataTmp.get<UINT32>("SrcPortMax");
        UINT32 uiDestPort = ptDataTmp.get<UINT32>("DestPort");
        pcCfg->SetProtocolUDP(uiSrcPortMin, uiSrcPortMax, uiDestPort);

    }
    catch (std::exception const &e) {
        JSON_PARSER_ERROR("Caught exception [%s] when ParserLocalCfg. LocalCfg:[%s]", e.what(), pcJsonData);
        return AGENT_E_ERROR;
    }

    return iRet;
}

#define NormalFlowRequestSignature    "HuaweiDCAnts"
#define NormalFlowRequestAction       "RequestServerProbeList"

INT32 CreatAgentIPRequestPostData(ServerAntAgentCfg_C *pcCfg, stringstream *pssPostData) {
    INT32 iRet = AGENT_OK;

    stringstream ssJsonData;
    ptree ptDataRoot;
    UINT32 uiIp, uiMgntIp;
    // boost库中出现错误会抛出异常, 未被catch的异常会逐级上报, 最终导致进程abort退出.
    try {
        pcCfg->GetAgentAddress(&uiIp);
        if (iRet) {
            JSON_PARSER_ERROR("GetAgentAddress failed[%d]", iRet);
            return iRet;
        }

        pcCfg->GetMgntIP(&uiMgntIp);
        ptDataRoot.put("vbond_ip", sal_inet_ntoa(uiIp));    // 数据面IP
        ptDataRoot.put("agent_ip", sal_inet_ntoa(uiMgntIp));

        write_json(ssJsonData, ptDataRoot);
        (*pssPostData) << ssJsonData.str();
    }
    catch (std::exception const &e) {
        JSON_PARSER_ERROR("Caught exception [%s] when CreatAgentIPRequestPostData.", e.what());
        return AGENT_E_ERROR;
    }
    return AGENT_OK;
}

/*
{
    "flow":
    {

        "sip": "",
        "dip": "",
        "sport": "",
        "time":
        {
            "t1": "",
            "t2": "",
            "t3": "",
            "t4": ""
        },
        "statistics":
        {
            "packet-sent": "",
            "packet-drops": "",
            "50percentile": "",
            "99percentile": ""
            "standard-deviation": ""
            "drop-notices": "",
        },
    },
}
*/
#define LatencyReportSignature    "HuaweiDC3ServerAntsFull"

// 生成json格式的字符串, 用于向Analyzer上报延时信息.
INT32 CreateLatencyReportData(AgentFlowTableEntry_S *pstAgentFlowEntry, stringstream *pssReportData, UINT32 maxDelay,
                              UINT32 bigPkgSize) {
    INT64 max = pstAgentFlowEntry->stFlowDetectResult.lLatencyMax;
    if (-1 != max && 0 != maxDelay && maxDelay * 1000 > max) {
        JSON_PARSER_INFO("Max delay is [%d], less than threshold[%d], does not report.", max, maxDelay * 1000);
        return AGENT_FILTER_DELAY;
    }

    ptree ptDataRoot, ptDataFlowArray, ptDataFlowEntry;
    ptree ptDataFlowEntryTemp;
    char acCurTime[32] = {0};                      // 缓存时间戳

    stringstream ssJsonData;
    // boost库中出现错误会抛出异常, 未被catch的异常会逐级上报, 最终导致进程abort退出.
    try {
        ptDataRoot.clear();
        ptDataRoot.put("orgnizationSignature", LatencyReportSignature);

        // 清空Flow Entry Array
        ptDataFlowArray.clear();

        // 生成一个Flow Entry的数据
        {
            GetPrintTime(acCurTime);
            ptDataFlowEntry.clear();
            ptDataFlowEntry.put("sip", sal_inet_ntoa(pstAgentFlowEntry->stFlowKey.uiSrcIP));
            ptDataFlowEntry.put("dip", sal_inet_ntoa(pstAgentFlowEntry->stFlowKey.uiDestIP));
            ptDataFlowEntry.put("sport", pstAgentFlowEntry->stFlowKey.uiSrcPort);
            ptDataFlowEntry.put("time", acCurTime);

            // 处理time信息
            ptDataFlowEntryTemp.clear();
            ptDataFlowEntryTemp.put("t1", pstAgentFlowEntry->stFlowDetectResult.lT1);
            ptDataFlowEntryTemp.put("t2", pstAgentFlowEntry->stFlowDetectResult.lT2);
            ptDataFlowEntryTemp.put("t3", pstAgentFlowEntry->stFlowDetectResult.lT3);
            ptDataFlowEntryTemp.put("t4", pstAgentFlowEntry->stFlowDetectResult.lT4);
            ptDataFlowEntry.put_child("times", ptDataFlowEntryTemp);

            // 处理statistics信息
            ptDataFlowEntryTemp.clear();
            ptDataFlowEntryTemp.put("packet-sent", pstAgentFlowEntry->stFlowDetectResult.lPktSentCounter);
            ptDataFlowEntryTemp.put("packet-drops", pstAgentFlowEntry->stFlowDetectResult.lPktDropCounter);
            ptDataFlowEntryTemp.put("50percentile", pstAgentFlowEntry->stFlowDetectResult.lLatency50Percentile);
            ptDataFlowEntryTemp.put("99percentile", pstAgentFlowEntry->stFlowDetectResult.lLatency99Percentile);
            ptDataFlowEntryTemp.put("standard-deviation",
                                    pstAgentFlowEntry->stFlowDetectResult.lLatencyStandardDeviation);
            ptDataFlowEntryTemp.put("min", pstAgentFlowEntry->stFlowDetectResult.lLatencyMin);
            ptDataFlowEntryTemp.put("max", max);
            ptDataFlowEntryTemp.put("drop-notices", pstAgentFlowEntry->stFlowDetectResult.lDropNotesCounter);
            ptDataFlowEntry.put_child("statistics", ptDataFlowEntryTemp);
            if (pstAgentFlowEntry->stFlowKey.uiIsBigPkg) {
                if (bigPkgSize) {
                    ptDataFlowEntry.put("package-size", bigPkgSize);
                } else {
                    ptDataFlowEntry.put("package-size", BIG_PACKAGE_SIZE);
                }
            } else {
                ptDataFlowEntry.put("package-size", NORMAL_PACKAGE_SIZE);
            }

            // 加入json数组, 暂不使用数组, Collector不支持.
            ptDataFlowArray.push_back(make_pair("", ptDataFlowEntry));
        }

        ptDataRoot.put_child("flow", ptDataFlowArray);

        ssJsonData.clear();
        ssJsonData.str("");
        write_json(ssJsonData, ptDataRoot);
        (*pssReportData) << ssJsonData.str();
    }
    catch (std::exception const &e) {
        JSON_PARSER_ERROR("Caught exception [%s] when CreateLatencyReportData.", e.what());
        return AGENT_E_ERROR;
    }
    return AGENT_OK;
}


/*
{
    "orgnizationSignature": "HuaweiDC3ServerAntsDropNotice",
    "flow": [
        {
            "sip": "10.78.221.45",
            "dip": "10.78.221.46",
            "sport": "5002",
            "dport": "6000",
            "ip-protocol": "udp",
            "dscp": "20",
            "urgent-flag": "0",
            "topology-tag": {
                "level": "1",
                "svid": "0x00000064",
                "dvid": "0x000001f4"
            },
            "statistics": {
                "t": "1477538852",
                "packet-sent": "5",
                "packet-drops": "5"
            }
        }
    ]
}

{
    "flow": [
        {
            "sip": "10.78.221.45",
            "dip": "10.78.221.46",
            "sport": "5002",
            "packet-sent": "5",
            "packet-drops": "5"
        }
    ]
}


*/
#define DropReportSignature    "HuaweiDC3ServerAntsDropNotice"

// 生成json格式的字符串, 用于向Analyzer上报丢包信息.
INT32 CreateDropReportData(AgentFlowTableEntry_S *pstAgentFlowEntry, stringstream *pssReportData, UINT32 bigPkgSize) {
    // boost库中出现错误会抛出异常, 未被catch的异常会逐级上报, 最终导致进程abort退出.
    char acCurTime[32] = {0};                      // 缓存时间戳
    ptree ptDataRoot, ptDataFlowArray, ptDataFlowEntry;
    stringstream ssJsonData;
    try {
        ptDataRoot.clear();
        ptDataRoot.put("orgnizationSignature", DropReportSignature);

        // 清空Flow Entry Array
        ptDataFlowArray.clear();

        // 生成一个Flow Entry的数据
        {
            ptDataFlowEntry.clear();
            GetPrintTime(acCurTime);
            ptDataFlowEntry.put("sip", sal_inet_ntoa(pstAgentFlowEntry->stFlowKey.uiSrcIP));
            ptDataFlowEntry.put("dip", sal_inet_ntoa(pstAgentFlowEntry->stFlowKey.uiDestIP));
            ptDataFlowEntry.put("sport", pstAgentFlowEntry->stFlowKey.uiSrcPort);

            ptDataFlowEntry.put("time", acCurTime);
            ptDataFlowEntry.put("packet-sent", pstAgentFlowEntry->stFlowDetectResult.lPktSentCounter);
            ptDataFlowEntry.put("packet-drops", pstAgentFlowEntry->stFlowDetectResult.lPktDropCounter);

            if (pstAgentFlowEntry->stFlowKey.uiIsBigPkg) {
                if (bigPkgSize) {
                    ptDataFlowEntry.put("package-size", bigPkgSize);
                } else {
                    ptDataFlowEntry.put("package-size", BIG_PACKAGE_SIZE);
                }
            } else {
                ptDataFlowEntry.put("package-size", NORMAL_PACKAGE_SIZE);
            }


            // 加入json数组
            ptDataFlowArray.push_back(make_pair("", ptDataFlowEntry));
        }

        ptDataRoot.put_child("flow", ptDataFlowArray);

        ssJsonData.clear();
        ssJsonData.str("");
        write_json(ssJsonData, ptDataRoot);
        (*pssReportData) << ssJsonData.str();
    }
    catch (std::exception const &e) {
        JSON_PARSER_ERROR("Caught exception [%s] when CreatDropReportData.", e.what());
        return AGENT_E_ERROR;
    }

    return AGENT_OK;
}

// 解析json格式的flow array, 并下发到FlowManager,
INT32 IssueFlowFromConfigFile(vector<string> allIpList, FlowManager_C *pcFlowManager) {
    INT32 iRet = AGENT_OK;

    // boost库中出现错误会抛出异常, 未被catch的异常会逐级上报, 最终导致进程abort退出.
    try {
        ServerFlowKey_S stNewServerFlowKey;
        for (int index = 0; index < allIpList.size(); index++) {
            string aIp = allIpList[index];
            sal_memset(&stNewServerFlowKey, 0, sizeof(stNewServerFlowKey));
            iRet = GetFlowInfoFromConfigFile(aIp, &stNewServerFlowKey, pcFlowManager->pcAgentCfg);
            if (iRet) {
                JSON_PARSER_ERROR("Get Flow Info From Json failed [%d]", iRet);
                return iRet;
            }

            // 普通流程添加到配置表, 待配置倒换后生效.
            iRet = pcFlowManager->ServerWorkingFlowTableAdd(&stNewServerFlowKey);
            if (AGENT_OK != iRet) {
                JSON_PARSER_ERROR("Add New ServerWorkingFlowTable failed [%d]", iRet);
                return iRet;
            }
        }
    }
    catch (std::exception const &e) {
        JSON_PARSER_ERROR("Caught exception [%s] when IssueFlowFromJsonFlowArray.", e.what());
        return AGENT_E_ERROR;
    }

    return iRet;
}

// 解析Server下发的json格式flow数据,转换成ServerFlowKey_S格式
INT32 GetFlowInfoFromConfigFile(string dip, ServerFlowKey_S *pstNewServerFlowKey, ServerAntAgentCfg_C *pcAgentCfg) {
    // boost库中出现错误会抛出异常, 未被catch的异常会逐级上报, 最终导致进程abort退出.
    try {
        string strTemp;
        UINT32 uiDataTemp = 0;

        // 初始化
        sal_memset(pstNewServerFlowKey, 0, sizeof(ServerFlowKey_S));

        pstNewServerFlowKey->uiUrgentFlow = 0;
//        pstNewServerFlowKey->eProtocol = AGENT_DETECT_PROTOCOL_UDP;
        pstNewServerFlowKey->eProtocol = AGENT_DETECT_PROTOCOL_ICMP;
        pstNewServerFlowKey->uiSrcIP = pcAgentCfg->GetAgentIP();

        pstNewServerFlowKey->uiDestIP = sal_inet_aton(dip.c_str());

        pstNewServerFlowKey->uiDscp = pcAgentCfg->getDscp();
        pstNewServerFlowKey->uiSrcPortMin = 32769;
        pstNewServerFlowKey->uiSrcPortMax = 32868;
        pstNewServerFlowKey->uiSrcPortRange = pcAgentCfg->GetPortCount();

        pstNewServerFlowKey->stServerTopo.uiSvid = 0;
        pstNewServerFlowKey->stServerTopo.uiDvid = 0;
        pstNewServerFlowKey->stServerTopo.uiLevel = 1;
    }
    catch (std::exception const &e) {
        JSON_PARSER_ERROR("Caught exception [%s] when GetFlowInfoFromConfigFile.", e.what());
        return AGENT_E_ERROR;
    }

    return AGENT_OK;
}

INT32 ParseLocalAgentConfig(const char *pcJsonData, FlowManager_C *pcFlowManager) {
    INT32 iRet = AGENT_OK;
    string strTemp;
    UINT32 data;
    // boost::property_tree对象, 用于存储json格式数据.
    ptree ptDataRoot, ptDataTmp;

    // boost库中出现错误会抛出异常, 未被catch的异常会逐级上报, 最终导致进程abort退出.
    try {
        // pcData字符串转存stringstream格式, 方便后续boost::property_tree处理.
        stringstream ssStringData(pcJsonData);
        read_json(ssStringData, ptDataRoot);
        data = ptDataRoot.get<UINT32>("probe_period");
        pcFlowManager->pcAgentCfg->SetDetectPeriod(data);

        data = ptDataRoot.get<UINT32>("port_count");
        pcFlowManager->pcAgentCfg->SetPortCount(data);

        data = ptDataRoot.get<UINT32>("report_period");
        pcFlowManager->pcAgentCfg->SetReportPeriod(data);

        data = ptDataRoot.get<UINT32>("package_size");
        pcFlowManager->pcAgentCfg->SetBigPkgSize(data);

        data = ptDataRoot.get<UINT32>("pkg_count");
        pcFlowManager->pcAgentCfg->SetBigPkgRate(data);

        data = ptDataRoot.get<UINT32>("delay_threshold");
        pcFlowManager->pcAgentCfg->SetMaxDelay(data);

        data = ptDataRoot.get<UINT32>("dscp");
        pcFlowManager->pcAgentCfg->SetDscp(data);

        data = ptDataRoot.get<UINT32>("lossPkg_timeout");
        pcFlowManager->pcAgentCfg->SetDetectTimeout(data);

        strTemp = ptDataRoot.get<string>("kafka_ip");
        pcFlowManager->pcAgentCfg->SetKafkaIp(strTemp);

        strTemp = ptDataRoot.get<string>("basicToken");

        pcFlowManager->pcAgentCfg->SetKafkaBasicToken(strTemp);

        strTemp = ptDataRoot.get<string>("topic");
        pcFlowManager->pcAgentCfg->SetTopic(strTemp);

        data = ptDataRoot.get<UINT32>("vbondIp_flag");
        if (data) {
            JSON_PARSER_INFO("Set vbondIp_flag to [%u], will report agent ip in next interval.", data);
            SHOULD_REPORT_IP = 1;
        }

        data = ptDataRoot.get<UINT32>("dropPkgThresh");
        pcFlowManager->pcAgentCfg->SetDetectDropThresh(data);

        strTemp = ptDataRoot.get<string>("pingList");

        vector<string> ip_list_vector;
        if (!strTemp.empty()) {

            split(ip_list_vector, strTemp, is_any_of(","));
        }
        vector<string> ip_az_list_vector;
        strTemp = ptDataRoot.get<string>("pingList_az");

        if (!strTemp.empty()) {

            split(ip_az_list_vector, strTemp, is_any_of(","));
        }


        std::vector<string> all_ip_list;
        sort(ip_list_vector.begin(), ip_list_vector.end());
        sort(ip_az_list_vector.begin(), ip_az_list_vector.end());
        std::set_union(ip_list_vector.begin(), ip_list_vector.end(),
                       ip_az_list_vector.begin(), ip_az_list_vector.end(),
                       back_inserter(all_ip_list));


        pcFlowManager->ServerClearFlowTable();
        iRet = IssueFlowFromConfigFile(all_ip_list, pcFlowManager);
        if (iRet) {
            JSON_PARSER_ERROR("Issue Flow From Json Flow Array failed [%d]. Flow info[%s]", iRet, pcJsonData);
            return iRet;
        }
        pcFlowManager->RefreshAgentTable();
    }
    catch (std::exception const &e) {
        JSON_PARSER_ERROR("Caught exception [%s] when ParseLocalAgentConfig. LocalConfig:[%s]", e.what(), pcJsonData);
        return AGENT_E_ERROR;
    }
    return iRet;
}

UINT32 ParseProbePeriodConfig(const char *pcJsonData, FlowManager_C *pcFlowManager) {
    UINT32 iRet = 9999;
    string strTemp;
    UINT32 data;
    // boost::property_tree对象, 用于存储json格式数据.
    ptree ptDataRoot;

    // boost库中出现错误会抛出异常, 未被catch的异常会逐级上报, 最终导致进程abort退出.
    try {
        // pcData字符串转存stringstream格式, 方便后续boost::property_tree处理.
        stringstream ssStringData(pcJsonData);
        read_json(ssStringData, ptDataRoot);
        data = ptDataRoot.get<UINT32>("probe_period");
        return data;
    }
    catch (std::exception const &e) {
        JSON_PARSER_ERROR("Caught exception [%s] when ParseLocalAgentConfig. LocalConfig:[%s]", e.what(), pcJsonData);
        return iRet;
    }
    return iRet;
}

