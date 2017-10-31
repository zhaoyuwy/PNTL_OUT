//
// Created by zy on 17-9-15.
//
#include <boost/algorithm/string.hpp>
#include "FileNotifier.h"
#include "AgentCommon.h"
#include "GetLocalCfg.h"
#include "Log.h"

using namespace std;
using namespace boost;

FileNotifier_C::FileNotifier_C() {
    notifierId = -1;
    wdSendFile = -1;
    wdIncreaseFile = -1;
    lastAction = 0;
}

FileNotifier_C::~FileNotifier_C() {
    INT32 iRet = inotify_rm_watch(notifierId, wdSendFile);
    if (0 > iRet) {
        FILE_NOTIFIER_ERROR("Remove watch Send File fail[%d]", iRet);
    }

    iRet = inotify_rm_watch(notifierId, wdIncreaseFile);
    if (0 > iRet) {
        FILE_NOTIFIER_ERROR("Remove watch increase file fail[%d]", iRet);
    }

    if (-1 != notifierId) {
        iRet = close(notifierId);
        if (0 > iRet) {
            FILE_NOTIFIER_ERROR("close watch fail[%d]", iRet);
        }
    }
}

INT32 FileNotifier_C::Init(FlowManager_C *pcFlowManager) {
    manager = pcFlowManager;
    notifierId = inotify_init();
    if (0 > notifierId) {
        FILE_NOTIFIER_ERROR("Create a file notifier fail[%d]", notifierId);
        return AGENT_E_MEMORY;
    }

    wdSendFile = inotify_add_watch(notifierId, filePath.c_str(), IN_MODIFY);
    wdIncreaseFile = inotify_add_watch(notifierId, changefilePath.c_str(), IN_MODIFY);
    if (0 > wdSendFile && 0 > wdIncreaseFile) {
        FILE_NOTIFIER_ERROR("Create a watch Item fail[%d]", wdSendFile);
        return AGENT_E_ERROR;
    }
    INT32 iRet = StartThread();
    if (iRet) {
        FILE_NOTIFIER_ERROR("StartFileNotifierThread failed[%d]", iRet);
        return iRet;
    }
    return AGENT_OK;
}

INT32 FileNotifier_C::HandleEvent(struct inotify_event *event) {
    if (event->mask & IN_MODIFY) {
        HandleProbePeriod();
    } else if (event->mask & IN_IGNORED) {
        wdSendFile = inotify_add_watch(notifierId, filePath.c_str(), IN_MODIFY);
        wdIncreaseFile = inotify_add_watch(notifierId, changefilePath.c_str(), IN_MODIFY);
        if (0 > wdIncreaseFile) {
        } else {
            ChangeFileFun(changefilePath.c_str());
        }
        if (0 > wdSendFile) {
            FILE_NOTIFIER_ERROR("Create a watch Item fail[%d]", wdSendFile);
            return AGENT_E_ERROR;
        } else {

            HandleProbePeriod();
        }
    }
}

INT32 FileNotifier_C::PreStopHandler() {
    return 0;
}

INT32 FileNotifier_C::PreStartHandler() {
    return 0;
}

INT32 FileNotifier_C::ThreadHandler() {
    INT32 sizeRead = 0;
    CHAR *pBuf;
    while (GetCurrentInterval()) {
        sizeRead = read(notifierId, buf, BUF_LEN);
        if (0 > sizeRead) {
            continue;
        }
        for (pBuf = buf; pBuf < buf + sizeRead;) {
            event = (struct inotify_event *) pBuf;
            HandleEvent(event);
            pBuf += sizeof(struct inotify_event) + event->len;
        }
    }
}

void FileNotifier_C::HandleProbePeriod() {
    UINT32 probePeriod = GetProbePeriod(manager);
    if (0 > probePeriod || 120 < probePeriod) {
        FILE_NOTIFIER_ERROR("Parse local config file error, probePeriod is [%u], return.", probePeriod);
        return;
    } else if (0 == probePeriod) {
        FILE_NOTIFIER_INFO("Probe_period is [%u], will stop flowmanger.", probePeriod);
        manager->FlowManagerAction(STOP_AGENT);
        lastAction = 1;
    } else {
        if (lastAction) {
            FILE_NOTIFIER_INFO("Probe_period is [%u], will start flowmanger.", probePeriod);
            manager->FlowManagerAction(START_AGENT);
            lastAction = 0;
        }
        SHOULD_REFRESH_CONF = 1;
    }
}

string vectorToString(const std::vector<string> &vec) {
    string rst = "";
    typedef typename std::vector<string>::size_type size_type;
    for (size_type i = 0; i < vec.size(); i++)
        rst.append(vec[i]).append(i == vec.size() - 1 ? "" : ",");
    return rst;
}

INT32 FileNotifier_C::ChangeFileFun(string sChangeFileName) {
    printf("this is chang file name = %s \n", sChangeFileName.c_str());
    INT32 iRet = AGENT_OK;
    string strTemp;
    UINT32 data;
    stringstream ssCfgFileName;

    string strChangeCfgJsonData = readJsonCfg(sChangeFileName);
    string strCfgJsonData = readJsonCfg(filePath);

    // boost::property_tree对象, 用于存储json格式数据.
    ptree ptDataChange, ptDataOld, ptDataTmp;
    string spiltIpList = ",";
    string rstIpList = "";
    string spiltErrorIpList = ",,";
    string rstList = "";

    // boost库中出现错误会抛出异常, 未被catch的异常会逐级上报, 最终导致进程abort退出.

    // pcData字符串转存stringstream格式, 方便后续boost::property_tree处理.
    stringstream ssStringData(strCfgJsonData);
    stringstream ssStringChangeData(strChangeCfgJsonData);
    read_json(ssStringData, ptDataOld);
    read_json(ssStringChangeData, ptDataChange);

    string del_string = ptDataChange.get<string>("del");


    string add_string = ptDataChange.get<string>("add");

    add_string.append(add_string);

//    string del_az_string = ptDataChange.get<string>("del_az");
//    string add_az_string = ptDataChange.get<string>("add_az");

    vector<string> old_az_ip_vector;


    string list2[] = {"pingList", "pingList_az"};
    for (int index = 0; index < 2; index++) {
        try {
            string add_az_string;
            string del_az_string;
            string old_string_pingList_az;
            if (0 == list2[index].compare("pingList")) {
                add_az_string = ptDataChange.get<string>("add");
                del_az_string = ptDataChange.get<string>("del");
                old_string_pingList_az = ptDataOld.get<string>("pingList");
            } else if (0 == list2[index].compare("pingList_az")) {
                del_az_string = ptDataChange.get<string>("del_az");
                add_az_string = ptDataChange.get<string>("add_az");
                old_string_pingList_az = ptDataOld.get<string>("pingList_az");
            } else {
                continue;
            }
            split(old_az_ip_vector, old_string_pingList_az, is_any_of(","));
            if (!add_az_string.empty() || !del_az_string.empty()) {

                vector<string> add_ip_vector;
                vector<string> del_ip_vector;

                split(add_ip_vector, add_az_string, is_any_of(","));
                split(del_ip_vector, del_az_string, is_any_of(","));
//            old_string_pingList_az.append(add_az_string);

//                std::cout << vectorToString(old_az_ip_vector) << endl;
                JSON_PARSER_INFO("this is old %s  iplist = %s", list2[index].c_str(),
                                 vectorToString(old_az_ip_vector).c_str());
                std::vector<string> add_vec_rst;
                sort(old_az_ip_vector.begin(), old_az_ip_vector.end());
                sort(add_ip_vector.begin(), add_ip_vector.end());
                std::set_union(old_az_ip_vector.begin(), old_az_ip_vector.end(),
                               add_ip_vector.begin(), add_ip_vector.end(),
                               back_inserter(add_vec_rst));

                std::cout << vectorToString(add_vec_rst) << endl;
                JSON_PARSER_INFO("this is old %s  iplist add vect  = %s", list2[index].c_str(),
                                 vectorToString(add_ip_vector).c_str());
                JSON_PARSER_INFO("this is old %s  iplist after add vect  = %s", list2[index].c_str(),
                                 vectorToString(add_vec_rst).c_str());
                JSON_PARSER_INFO("this is old %s  iplist delete vect  = %s", list2[index].c_str(),
                                 vectorToString(del_ip_vector).c_str());

//                std::cout << vectorToString(del_ip_vector) << endl;
                std::vector<string> del_vec_rst;

                sort(old_az_ip_vector.begin(), old_az_ip_vector.end());
                sort(del_ip_vector.begin(), del_ip_vector.end());
                std::set_difference(add_vec_rst.begin(), add_vec_rst.end(),
                                    del_ip_vector.begin(), del_ip_vector.end(),
//                                std::insert_iterator<std::vector<string> >(del_vec_rst, del_vec_rst.begin()), std::greater<string>());
                                    back_inserter(del_vec_rst));

                JSON_PARSER_INFO("this is old %s  iplist after delete vect  = %s", list2[index].c_str(),
                                 vectorToString(del_vec_rst).c_str());
//                std::cout << vectorToString(del_vec_rst) << endl;
                rstIpList = vectorToString(del_vec_rst);
            }

        } catch (std::exception const &e) {
            JSON_PARSER_ERROR("Caught exception [%s] when ParseLocalAgentConfig. LocalConfig:[%s]", e.what(),
                              strCfgJsonData.c_str());
            return AGENT_E_ERROR;
        }

//        string ipListOldUse = ptDataOld.get<std::string>(list2[index]);
        ptDataOld.put(list2[index].c_str(), rstIpList.c_str());
    }


    boost::property_tree::write_json("agentConfig.cfg", ptDataOld);
    return 0;
}

string FileNotifier_C::readJsonCfg(string stringFile) {

    ifstream ifsAgentChangeCfg;
    ifsAgentChangeCfg.open(stringFile.c_str());


    if (ifsAgentChangeCfg.fail()) {
        INIT_ERROR("Can't open cfg file[%s]", stringFile.c_str());
        return "";
    }
    string strChangeCfgJsonData((istreambuf_iterator<char>(ifsAgentChangeCfg)), (istreambuf_iterator<char>()));

    ifsAgentChangeCfg.close();
    return strChangeCfgJsonData;
}
