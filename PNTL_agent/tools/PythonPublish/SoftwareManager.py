#!/usr/bin/env python
# -*- coding: utf-8 -*-

#!/usr/bin/env python

# ------------------------------------------
# Python 鍑芥暟瀹氫箟

# 鍒涘缓涓�涓狶ogger瀵硅薄,鐢ㄤ簬璁板綍鏃ュ織.
# 涓�涓狶ogger瀵硅薄瀵瑰簲涓�涓狶og妯″潡鍜屼竴涓枃浠�, 鍙�夋棩蹇楁槸鍚﹀悓鏃惰緭鍑哄埌缁堢
def CreateLogger(Name, LogFile, LogToFile, LogToStream):
    try:
        
        NewLogger = logging.getLogger(Name)
        NewLogger.setLevel(logging.DEBUG)
        
        Formatter = logging.Formatter("[%(asctime)s][%(name)s][%(levelname)s]\t[%(filename)s][%(lineno)3d]\t%(message)s")
        
        # log闇�瑕佽褰曞埌鏃ュ織鏂囦欢
        if True == LogToFile:
            # mkdir for LogFile
            Path = os.path.dirname(LogFile)
            if False == os.path.exists(Path):
                os.makedirs(Path)
        
            FileHandler = logging.FileHandler(LogFile)
            FileHandler.setLevel(logging.DEBUG)
            FileHandler.setFormatter(Formatter)
            NewLogger.addHandler(FileHandler)
            
        # log闇�瑕佹墦鍗板埌缁堢
        if True == LogToStream:
            StreamHandler = logging.StreamHandler()
            StreamHandler.setLevel(logging.DEBUG)
            StreamHandler.setFormatter(Formatter)
            NewLogger.addHandler(StreamHandler)
        
        NewLogger.info("")
        NewLogger.info("================= Logger[%s] Start Working Now ====================" % Name)
        NewLogger.info("")
        
    # Exception
    except Exception: 
        print "[%s] Exception in CreateLogger. Name:[%s], Path:[%s], ToFile[%s], ToStream[%s]. " % (datetime.datetime.now(), Name, LogFile, LogToFile, LogToStream)
        traceback.print_exc()
        return -1
    # No Exception    
    else:
        return NewLogger

# 鏍规嵁Logger鍚嶇О杩斿洖Logger瀵硅薄.
def GetLogger(Name):
    try:
        # name涓庡璞℃槸涓�瀵逛竴鐨�, 涓嶄細閲嶅鍒涘缓.
        Logger = logging.getLogger(Name)
        
    # Exception
    except Exception: 
        print "[%s] Exception in GetLogger. Name:[%s] " % (datetime.datetime.now(), Name)
        traceback.print_exc()
        return -1
    # No Exception    
    else:
        return Logger
        
# 鏍规嵁Logger鍚嶇О閿�姣丩ogger瀵硅薄, 棰勭暀, 灏氭湭瀹炵幇
def DestroyLogger(Name):
    try:
        # name涓庡璞℃槸涓�瀵逛竴鐨�, 涓嶄細閲嶅鍒涘缓.
        Logger = logging.getLogger(Name)
        Logger.info("")
        Logger.info("================= Logger[%s] Stop Working Now ====================" % Name)
        Logger.info("")
        #logging.shutdown()
        
    # Exception
    except Exception: 
        print "[%s] Exception in DestroyLogger. Name:[%s] " % (datetime.datetime.now(), Name)
        traceback.print_exc()
        return -1
    # No Exception    
    else:
        return 0

# 浣跨敤ssh鐧婚檰杩滅鏈嶅姟鍣�, 骞舵墽琛屼竴涓懡浠�.
# 鏍囧噯閿欒杈撳嚭鏈夊唴瀹规椂璁や负鍛戒护鎵ц鍑洪敊.
# 寰呬紭鍖栧紓甯稿鐞�,纰板埌杩囬潪娉旾P瀵艰嚧鐨勬寕姝�?
# 浣跨敤鏂规硶鍙傝��: http://docs.paramiko.org/en/2.0/
def ProcessAgentSSHCmd(IP, Port, User, Password, Path, Cmd, Logger):
    try:
        CmdFailed = False
        Ret = 0
        
        # 瀵煎叆鐜鍙橀噺:Agent绠＄悊IP
        # 鍒囨崲鐩綍:
        # 鍓嶉潰涓や釜鍛戒护鎵ц鎴愬姛鐨勬儏鍐典笅鎵ц鐢ㄦ埛杈撳叆鐨勫懡浠�
        RealCmd ="LANG= && export Agent_ConnectInfo_IP="+ IP + " && cd " + Path + " && (" + Cmd +")"
        
        Logger.info("")
        Logger.info("\t+++++++++++++++++++++++++++++++++++++++++")
        Logger.info("\tConnecting to Agent[%s]" % (IP))
        
        # 寤虹珛ssh 瀹㈡埛绔璞�
        Client = paramiko.SSHClient()
        Client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        # 寤虹珛杩炴帴, 璁剧疆瓒呮椂鏃堕棿, 瓒呮椂鎶涘紓甯�, 閬垮厤鎸傛. 
        Client.connect(IP, Port, User, Password, timeout=15)
        
        Logger.info("\tConnecting to Agent[%s] success." % (IP))
        Logger.info("\tBeginning to exec cmd[%s : %s] in Agent[%s]" % (Path, Cmd, IP))
        
        # stdin, stdout, stderr = Client.exec_command(RealCmd, timeout=300)
        # 閫氳繃ssh瀵硅薄鎻愪氦鍛戒护, 骞惰褰曟墽琛岀粨鏋�, 鏆傛椂鏈坊鍔犺秴鏃舵娴�(timeout 鍗曚綅涓簊)
        stdin, stdout, stderr = Client.exec_command(RealCmd)
        # 鎵撳嵃鎵ц缁撴灉
        Logger.info("\tResult: ")
        
        # 濡傛灉鍦ㄨ繙绋嬪惎鍔╯ervice, 閮ㄥ垎鐗规畩鏍煎紡瀛楃浼氬鑷� stdout.readlines()鎺ユ敹涓嶅埌EOF鑰屾寕姝�.
        out = stdout.readlines()
        for str in out:
            str = str.replace("\n", "")
            Logger.info("\t%s" % str)
        out = stderr.readlines()
        for str in out:
            str = str.replace("\n", "")
            # WARNING鍛婅涓嶅綋鍋氭墽琛屽け璐�(蹇界暐澶у皬鍐欑殑warning)
            if -1 == str.upper().find("WARNING:"):
                CmdFailed = True
                Logger.error("\t%s" % str)
            else:
                Logger.warning("\t%s" % str)
                
        # 鍒ゆ柇鎵ц鏄惁鍑洪敊.
        if False == CmdFailed :
            Logger.info("\tExec cmd[%s : %s] in Agent[%s] success" % (Path, Cmd, IP))
            Ret = 0
        else:
            Logger.error("\tExec cmd[%s : %s] in Agent[%s] failed" % (Path, Cmd, IP))
            Ret = -1        
        Logger.info("\t+++++++++++++++++++++++++++++++++++++++++")
        #Logger.info("")
        Client.close()
        return Ret
        
    # Exception
    except Exception: 
        Logger.error("\tException in SSHCmd. Exec cmd[%s : %s] in Agent[%s]" % (Path, Cmd, IP))
        traceback.print_exc()
        return -1
    # No Exception    
    else:
        return 0
    
# 閫氳繃sftp涓嬭浇鏂囦欢
# 浼氬皾璇曡嚜鍔ㄥ垱寤烘湰鍦扮洰褰�.
def ProcessAgentSSHDownload(IP, Port, User, Password, LocalFile, RemoteFile, Logger):
    try:
        Logger.info("")
        Logger.info("\t#########################################")
        
        # 鍒涘缓鏈湴鏂囦欢鐩綍
        Path = os.path.dirname(LocalFile)
        if False == os.path.exists(Path):
                os.makedirs(Path)
                
        Logger.info("\tConnecting to Agent[%s]" % (IP))
        # 鍒涘缓sftp浼犺緭瀵硅薄
        Transport = paramiko.Transport((IP, Port))
        Transport.connect(username = User, password = Password)
        sftp = paramiko.SFTPClient.from_transport(Transport)
        Logger.info("\tConnecting to Agent[%s] success" % (IP))
        
        # 鍚姩sftp涓嬭浇
        Logger.info("\tBeginning to download file")
        Logger.info("\tLocal: [%s] << Remote: [%s]:[%s]" %(LocalFile, IP, RemoteFile))
        sftp.get(RemoteFile, LocalFile)
        Logger.info("\tDownload file success")
        Logger.info("\t#########################################")
        #Logger.info("")
        
        Transport.close()
        return 0
        
    # Exception
    except Exception: 
        Logger.error("\tException in sftp download. Local: [%s] << Remote: [%s]:[%s]." % (LocalFile, IP, RemoteFile))
        traceback.print_exc()
        return -1
        
    # No Exception    
    else:
        return 0
        
# upload file through sftp
def ProcessAgentSSHUpload(IP, Port, User, Password, LocalFile, RemoteFile, Logger):
    try:
        Logger.info("")
        Logger.info("\t#########################################")
        
        # 鍒涘缓杩滅鏂囦欢鐩綍
        RemotPath = os.path.dirname(RemoteFile)
        Cmd = "mkdir -p " + RemotPath
        Ret = ProcessAgentSSHCmd(IP, Port, User, Password, "/", Cmd, Logger)
        if 0 > Ret:
            Logger.error("\tMake remote dir[%s] for upload file[%s] failed[%d] in agent[%s]" % (RemotPath, RemoteFile, Ret, IP))
            return -1
        
        Logger.info("\tConnecting to Agent[%s]" % (IP))
        # 鍒涘缓sftp浼犺緭瀵硅薄
        Transport = paramiko.Transport((IP, Port))
        Transport.connect(username = User, password = Password)
        sftp = paramiko.SFTPClient.from_transport(Transport)
        Logger.info("\tConnecting to Agent[%s] success" % (IP))
        
        # 鍚姩涓婁紶
        Logger.info("\tBeginning to upload file")
        Logger.info("\tLocal: [%s] >> Remote: [%s]:[%s]" %(LocalFile, IP, RemoteFile))
        sftp.put(LocalFile, RemoteFile)
        Logger.info("\tUpload file success" )
        Logger.info("\t#########################################")
        #Logger.info("")
        
        Transport.close()
        return 0
        
    # Exception
    except Exception: 
        Logger.error("\tException in sftp upload. Local: [%s] << Remote: [%s]:[%s]." % (LocalFile, IP, RemoteFile))
        traceback.print_exc()
        return -1
        
    # No Exception    
    else:
        return 0

        
# 杩炴帴Agent, 骞堕獙璇丄gent鍙敤. 褰撳墠鍙槸璁板綍Agent鏈嶅姟鍣ㄦ椂闂�
def ProcessAgentPrepareEnv(ConnectInfo, Logger):
    return ProcessAgentSSHCmd(ConnectInfo["IP"], ConnectInfo["Port"], ConnectInfo["User"], ConnectInfo["Password"], "/", "date", Logger)
            
# 瀹屾垚鎵�鏈夋枃浠朵笂浼�, 浠讳綍涓�涓枃浠朵笂浼犲け璐ヨ繑鍥為敊璇�.
def ProcessAgentUploadFileList(ConnectInfo, UploadFileList, Logger):
    Ret = 0
    for UploadFile in UploadFileList:
        Ret = ProcessAgentSSHUpload(ConnectInfo["IP"], ConnectInfo["Port"], ConnectInfo["User"], ConnectInfo["Password"], UploadFile["LocalFile"], UploadFile["RemotFile"], Logger)
        if 0 > Ret:
            Logger.error("\tProcessAgentSSHUpload[%s] in Agent[%s] failed[%d]" % (UploadFile["LocalFile"], ConnectInfo["IP"], Ret))
            break
    return Ret

# 瀹屾垚鎵�鏈夋枃浠朵笅杞�, 浠讳綍涓�涓枃浠朵笅杞藉け璐ヨ繑鍥為敊璇�.
def ProcessAgentDownloadFileList(ConnectInfo, DownloadFileList, Logger):
    Ret = 0
    for DownloadFile in DownloadFileList:
        Ret = ProcessAgentSSHDownload(ConnectInfo["IP"], ConnectInfo["Port"], ConnectInfo["User"], ConnectInfo["Password"], DownloadFile["LocalFile"], DownloadFile["RemotFile"], Logger)
        if 0 > Ret:
            Logger.error("\tProcessAgentSSHDownload[%s] in Agent[%s] failed[%d]" % (DownloadFile["RemotFile"], ConnectInfo["IP"], Ret))
            break
    return Ret
    
# 瀹屾垚鎵�鏈夊懡浠ゆ墽琛�, 浠讳綍涓�涓懡浠ゆ墽琛屽け璐ヨ繑鍥為敊璇�.
def ProcessAgentExecCmdList(ConnectInfo, CmdList, Logger):
    Ret = 0
    for Cmd in CmdList:
        Ret = ProcessAgentSSHCmd(ConnectInfo["IP"], ConnectInfo["Port"], ConnectInfo["User"], ConnectInfo["Password"], Cmd["Path"], Cmd["Cmd"], Logger)
        if 0 > Ret:
            Logger.error("\tProcessAgentSSHCmd[%s/%s] in Agent[%s] failed[%d]" % (Cmd["Path"], Cmd["Cmd"], ConnectInfo["IP"], Ret))
            break
    return Ret
    
# 澶勭悊涓�涓狝gent鐨勬墍鏈夋搷浣� , 骞惰妯″紡浼氬奖鍝嶆棩蹇楄褰曠殑琛屼负
def ProcessAgent(ConnectInfo, UploadFileList, CmdList, DownloadFileList, ManagerLogger, ParallelMode):
    try:
        AgentLoggerIsValid = False
        
        # 鍒涘缓Agent瀵瑰簲鐨凩og瀵硅薄, 鐢盤rocessAgent鍑芥暟浣跨敤澶勭悊瀹屾垚鍚�
        # 涓茶妯″紡涓嬫棩蹇楄褰曞埌鏂囦欢, 鍚屾椂鎵撳嵃鍒扮粓绔�
        if False == ParallelMode:
            LogToFile   = True
            LogToStream = True
        else:
            LogToFile   = True
            LogToStream = False
        
        LogFileName = "./Log/" + ConnectInfo["IP"] + "/" + ConnectInfo["IP"] +".log"
        Logger = CreateLogger(ConnectInfo["IP"], LogFileName, LogToFile, LogToStream)
        if -1 == Logger:
            ManagerLogger.warning("CreateLogger for Agent[%s] failed" % (ConnectInfo["IP"]))
            return -1
            
        AgentLoggerIsValid = True
        
        # 鍑嗗Agent鐜.
        Ret = ProcessAgentPrepareEnv(ConnectInfo, Logger)
        if 0 > Ret:
            Logger.error("ProcessAgentPrepareEnv in Agent[%s] failed[%d]" % (ConnectInfo["IP"], Ret))
            DestroyLogger(ConnectInfo["IP"])
            return Ret
        
        # 澶勭悊Agent鐨勪笂浼犱换鍔�.
        Ret = ProcessAgentUploadFileList(ConnectInfo, UploadFileList, Logger)
        if 0 > Ret:
            Logger.error("ProcessAgentUploadFileList in Agent[%s] failed[%d]" % (ConnectInfo["IP"], Ret))
            DestroyLogger(ConnectInfo["IP"])
            return Ret
        
        # 澶勭悊Agent鐨勫懡浠ゅ垪琛�.
        Ret = ProcessAgentExecCmdList(ConnectInfo, CmdList, Logger)
        if 0 > Ret:
            Logger.error("ProcessAgentExecCmdList in Agent[%s] failed[%d]" % (ConnectInfo["IP"], Ret))
            DestroyLogger(ConnectInfo["IP"])
            return Ret
        
        # 澶勭悊Agent鐨勪笅杞戒换鍔�
        Ret = ProcessAgentDownloadFileList(ConnectInfo, DownloadFileList, Logger)
        if 0 > Ret:
            Logger.error("ProcessAgentDownloadFileList in Agent[%s] failed[%d]" % (ConnectInfo["IP"], Ret))
            DestroyLogger(ConnectInfo["IP"])
            return Ret
            
    # Exception
    except Exception:
        # Agent寮傚父浼樺厛璁板綍鍒癆gent Logger, 褰揂gent Logger涓嶅彲鐢ㄦ椂璁板綍鍒癕anagerLogger
        if True == AgentLoggerIsValid :
            Logger.error("Exception in ProcessAgent %s" % (ConnectInfo["IP"]))
        else:
            ManagerLogger.error("Exception in ProcessAgent %s" % (ConnectInfo["IP"]))
        traceback.print_exc()
        DestroyLogger(ConnectInfo["IP"])
        return -1
        
    # No Exception    
    else:
        DestroyLogger(ConnectInfo["IP"])
        return 0
        
    


# 閬嶅巻Agent鍒楄〃, 涓茶澶勭悊姣忎釜Agent鐨勬墍鏈夋搷浣�.
def TraverseAgent(AgentList, Logger):
    try:
        # 褰撳墠澶勭悊鐨凙gent绱㈠紩
        CurrentAgentIndex = 0
        # 澶勭悊鎴愬姛鐨凙gent涓暟
        ProcessAgentSucessCnt = 0
        # 閬嶅巻Agent鍒楄〃
        for Agent in AgentList:
            CurrentAgentIndex = CurrentAgentIndex + 1
            Logger.info("-------------------------------")
            Logger.info("Processing Agent: %d / %d" % (CurrentAgentIndex, len(AgentList)))
                
            # 璋冪敤Agent澶勭悊鍑芥暟, 涓茶妯″紡鏃ュ織榛樿鎵撳嵃鍒扮粓绔�, 鍚屾椂涔熶細璁板綍鍒版枃浠�
            Ret = ProcessAgent(Agent["ConnectInfo"], Agent["UploadFileList"] ,Agent["CmdList"] ,Agent["DownloadFileList"], Logger, False)
            if 0 > Ret:
                Logger.warning("ProcessAgent[%s] failed [%d]" % (Agent["ConnectInfo"]["IP"], Ret))
                Logger.info("-------------------------------")
                # return [Ret, CurrentAgentIndex, Agent["ConnectInfo"]["IP"]]
                continue
                
            # Agent澶勭悊鎴愬姛
            Logger.info("Process Agent: %d / %d success" % (CurrentAgentIndex, len(AgentList)))
            Logger.info("-------------------------------")
            ProcessAgentSucessCnt = ProcessAgentSucessCnt + 1
            
        return ProcessAgentSucessCnt
            
    # Exception
    except Exception: 
        Logger.error("Exception in TraverseAgent index [%d]" % (CurrentAgentIndex))
        traceback.print_exc()
        
    # No Exception    
    #else:
        
    finally:
        return ProcessAgentSucessCnt
    
# 澶氱嚎绋嬫ā寮忓苟琛屽鐞嗘墍鏈堿gent鎿嶄綔
# 寰呬紭鍖�, 鍥炴敹澶氫釜浠诲姟鐨勫鐞嗙粨鏋滃苟姹囨��. 閬垮厤agent浠诲姟鏈鐞嗗畬鏃朵富浠诲姟閫�鍑�.
def TraverseAgentThread(AgentList, Logger):
    try:
        # 褰撳墠澶勭悊鐨凙gent绱㈠紩
        CurrentAgentIndex = 0
        # 澶勭悊鎴愬姛鐨凙gent涓暟
        ProcessAgentSucessCnt = 0
        
        # thread list
        #threads = []
            
        # 閬嶅巻Agent鍒楄〃
        for Agent in AgentList:
            CurrentAgentIndex = CurrentAgentIndex + 1
            Logger.info("-------------------------------")
            Logger.info("Processing Agent: %d / %d" % (CurrentAgentIndex, len(AgentList)))
                
            # 璋冪敤Agent澶勭悊鍑芥暟, 澶氱嚎绋嬫ā寮忔棩蹇椾笉鎵撳嵃鍒扮粓绔�, 鍚﹀垯缁堢杈撳嚭浼氬緢娣蜂贡. 鏃ュ織浼氳褰曞埌鏂囦欢
            thread = threading.Thread(target=ProcessAgent, args=(Agent["ConnectInfo"], Agent["UploadFileList"] ,Agent["CmdList"] ,Agent["DownloadFileList"], Logger, True))
            #threads.append(thread)
            Ret = thread.start()
            if Ret:
                Logger.warning("Start ProcessAgent thread for [%s] failed [%d]" % (Agent["ConnectInfo"]["IP"], Ret))
                Logger.info("-------------------------------")
                # return [Ret, CurrentAgentIndex, Agent["ConnectInfo"]["IP"]]
                continue
                
            # Agent澶勭悊鎴愬姛
            Logger.info("Process Agent: %d / %d success" % (CurrentAgentIndex, len(AgentList)))
            Logger.info("-------------------------------")
            ProcessAgentSucessCnt = ProcessAgentSucessCnt + 1
        
        thread.join()
        return ProcessAgentSucessCnt
            
    # Exception
    except Exception: 
        Logger.error("Exception in TraverseAgentThread index [%d]" % (CurrentAgentIndex))
        traceback.print_exc()
        
    # No Exception    
    #else:
        
    finally:
        return ProcessAgentSucessCnt

# 璇诲彇鏈湴閰嶇疆鏂囦欢
def GetAgentCfg(CfgFile, Logger):
    try:
        # 鎵撳紑閰嶇疆鏂囦欢
        fp = file(CfgFile)
        # 璇诲彇鏂囦欢鍐呭, 骞舵寜鐓son鏍煎紡瑙ｆ瀽
        JsonData = json.load(fp)
        # 鍏抽棴閰嶇疆鏂囦欢
        fp.close
        
        return JsonData
        
    # Exception
    except Exception: 
        Logger.error("Exception in GetAgentCfg [%s]" % (CfgFile))
        traceback.print_exc()
        return -1
        
    # No Exception    
    # else: 

# 涓诲叆鍙ｅ嚱鏁�
def main(AgentCfgFile):

    # 1. 鍒涘缓Log瀵硅薄,璁板綍绠＄悊淇℃伅, 鍚屾椂璁板綍鍒版枃浠跺拰缁堢
    ManagerLogger = CreateLogger("Manager", "./Log/ManagerLog.log", True, True)
    if -1 == ManagerLogger:
        print "CreateLogger for Manager failed"
        return -1
    
    # 2. 璇诲彇Agent閰嶇疆鏂囦欢
    AgentCfg = GetAgentCfg(AgentCfgFile, ManagerLogger)
    if -1 == AgentCfg: 
        ManagerLogger.error("GetAgentCfg failed")
        return -1
    
    # 3. 鎵撳嵃鍩烘湰閰嶇疆淇℃伅
    # 閰嶇疆鏂囦欢鐨勭増鏈俊鎭�
    ManagerLogger.info("AgentCfg Ver: %s" % AgentCfg["ver"])
    # 鎵撳嵃Agent 鎬绘暟
    ManagerLogger.info("Total Agents Number: [%d]" % len(AgentCfg['AgentList']))
    # 浠son鏍煎紡鎵撳嵃鎵�鏈夐厤缃俊鎭�, 璋冭瘯浣跨敤
    # ManagerLogger.info("AgentCfg Details: \n%s" % json.dumps(AgentCfg, indent=2))
    
    # 4. 澶勭悊鎵�鏈堿gent鎿嶄綔
    if "true" != AgentCfg["ParallelMode"]: 
        ManagerLogger.info("Process Agent In Sequence Mode")
        # 浼犵粺鐨勪覆琛屽鐞�, 鍒嗘瀽鎵�鏈堿gent鎵ц缁撴灉.
        Ret = TraverseAgent(AgentCfg["AgentList"], ManagerLogger)
        if Ret != len(AgentCfg['AgentList']): 
            ManagerLogger.error("Process [%d]/[%d] Agents Finished" % (Ret, len(AgentCfg['AgentList'])))
        else:
            ManagerLogger.info("Process [%d]/[%d] Agents Finished" % (Ret, len(AgentCfg['AgentList'])))
    else:
        ManagerLogger.info("Process Agent In Parallel Mode")
        # 姣忎釜Agent涓�涓嚎绋�, 骞惰鎵ц, 姣忎釜Agent鎵ц缁撴灉璇锋煡鐪嬪搴擜gent鐨勬棩蹇楁枃浠�.
        Ret = TraverseAgentThread(AgentCfg["AgentList"], ManagerLogger)
        if Ret != len(AgentCfg['AgentList']): 
            ManagerLogger.error("Process [%d]/[%d] Agents Finished" % (Ret, len(AgentCfg['AgentList'])))
        else:
            ManagerLogger.info("Process [%d]/[%d] Agents Finished" % (Ret, len(AgentCfg['AgentList'])))
    
    # 閿�姣丮anagerLogger瀵硅薄
    DestroyLogger("Manager")
    return 0
    
# ------------------------------------------
# prepare python
# load sys
import sys
reload(sys)
# 璁惧畾榛樿缂栫爜涓簎tf8
sys.setdefaultencoding("utf8")
# add path
#sys.path.append('./')

# 浣跨敤璺緞鍙婃枃浠跺す鐩稿叧澶勭悊
import os
# 鏀寔寮傚父鏃舵墦鍗拌皟鐢ㄦ爤
import traceback
# 鑾峰彇绯荤粺鏃堕棿,Logging妯″潡寮傚父澶勭悊鏃朵娇鐢�
import datetime
# 浣跨敤鏃ュ織澶勭悊鍔熻兘
import logging

# 浣跨敤ssh瀹㈡埛绔強sftp浼犺緭鍔熻兘
import paramiko
# 浣跨敤澶氫换鍔℃敮鎸佸苟琛屽鐞�
import threading
# 鏀寔json鏍煎紡鏁版嵁瑙ｆ瀽
import json

# 涓诲嚱鏁板叆鍙�, 鍏ュ弬涓篈gent閰嶇疆鏂囦欢, 鍙互娣诲姞璺緞, 榛樿鍦ㄥ綋鍓嶇洰褰曟煡鎵�.
main("AgentList.cfg")
