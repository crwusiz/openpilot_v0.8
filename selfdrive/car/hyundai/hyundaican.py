import crcmod
from common.params import Params
from selfdrive.car.hyundai.values import CAR, CHECKSUM, FEATURES

hyundai_checksum = crcmod.mkCrcFun(0x11D, initCrc=0xFD, rev=False, xorOut=0xdf)

def create_lkas11(packer, frame, car_fingerprint, apply_steer, steer_req,
                  lkas11, sys_warning, sys_state, enabled,
                  left_lane, right_lane,
                  left_lane_depart, right_lane_depart, bus):
  values = lkas11
  values["CF_Lkas_LdwsSysState"] = sys_state
  values["CF_Lkas_SysWarning"] = 3 if sys_warning else 0
  values["CF_Lkas_LdwsLHWarning"] = left_lane_depart
  values["CF_Lkas_LdwsRHWarning"] = right_lane_depart
  values["CR_Lkas_StrToqReq"] = apply_steer
  values["CF_Lkas_ActToi"] = steer_req
  values["CF_Lkas_ToiFlt"] = 0
  values["CF_Lkas_MsgCount"] = frame % 0x10
  values["CF_Lkas_Chksum"] = 0

  if car_fingerprint == CAR.GENESIS:
    values["CF_Lkas_LdwsActivemode"] = 2
    values["CF_Lkas_SysWarning"] = lkas11["CF_Lkas_SysWarning"]

  elif car_fingerprint in [CAR.OPTIMA, CAR.OPTIMA_HEV, CAR.CADENZA, CAR.CADENZA_HEV]:
    values["CF_Lkas_LdwsActivemode"] = 0  

  ldws_mfc = int(Params().get('MfcSelect')) == 1
  if ldws_mfc: # This field is LDWS Mfc car ( set is setup screen toggle )
    values["CF_Lkas_LdwsActivemode"] = 0
    values["CF_Lkas_LdwsOpt_USM"] = 3
    values["CF_Lkas_FcwOpt_USM"] = 2 if enabled else 1
#    values["CF_Lkas_SysWarning"] = 4 if sys_warning else 0

  lfa_mfc = int(Params().get('MfcSelect')) == 2
  if lfa_mfc: # This field is LFA Mfc car ( set is setup screen toggle )
    values["CF_Lkas_LdwsActivemode"] = int(left_lane) + (int(right_lane) << 1)
    values["CF_Lkas_LdwsOpt_USM"] = 2
    values["CF_Lkas_FcwOpt_USM"] = 2 if enabled else 1
    values["CF_Lkas_SysWarning"] = 4 if sys_warning else 0
    # ---------------------------------------------------------------------------------------
    # FcwOpt_USM 0 = No car + lanes
    #            1 = White car + lanes
    #            2 = Green car + lanes
    #            3 = Green blinking car + lanes
    #            4 = Orange car + lanes
    #            5 = Orange blinking car + lanes
    # SysWarning 4 = keep hands on wheel
    #            5 = keep hands on wheel (red)
    #            6 = keep hands on wheel (red) + beep
    # Note: the warning is hidden while the blinkers are on
    # ---------------------------------------------------------------------------------------

  dat = packer.make_can_msg("LKAS11", 0, values)[2]

  if car_fingerprint in CHECKSUM["crc8"]: # CRC Checksum
    dat = dat[:6] + dat[7:8]
    checksum = hyundai_checksum(dat)
  elif car_fingerprint in CHECKSUM["6B"]: # Checksum of first 6 Bytes
    checksum = sum(dat[:6]) % 256
  else:                                   # Checksum of first 6 Bytes and last Byte
    checksum = (sum(dat[:6]) + dat[7]) % 256

  values["CF_Lkas_Chksum"] = checksum

  return packer.make_can_msg("LKAS11", bus, values)

def create_clu11(packer, frame, bus, clu11, button, speed):
  values = clu11
  values["CF_Clu_CruiseSwState"] = button
  values["CF_Clu_Vanz"] = speed
  values["CF_Clu_AliveCnt1"] = frame % 0x10
  return packer.make_can_msg("CLU11", bus, values)

def create_lfahda_mfc(packer, enabled, hda_set_speed=0):
  values = {
#    "ACTIVE": enabled,
#    "HDA_USM": 2,
    "LFA_Icon_State": 2 if enabled else 0,
    "HDA_Active": 1 if hda_set_speed else 0,
    "HDA_Icon_State": 2 if hda_set_speed else 0,
    "HDA_VSetReq": hda_set_speed,
  }
  return packer.make_can_msg("LFAHDA_MFC", 0, values)

def create_mdps12(packer, frame, mdps12):
  values = mdps12
  values["CF_Mdps_ToiActive"] = 0
  values["CF_Mdps_ToiUnavail"] = 1
  values["CF_Mdps_MsgCount2"] = frame % 0x100
  values["CF_Mdps_Chksum2"] = 0

  dat = packer.make_can_msg("MDPS12", 2, values)[2]
  checksum = sum(dat) % 256
  values["CF_Mdps_Chksum2"] = checksum

  return packer.make_can_msg("MDPS12", 2, values)

def create_scc11(packer, frame, enabled, set_speed, lead_visible, scc_live, scc11):
  values = scc11
  values["AliveCounterACC"] = frame // 2 % 0x10
  if not scc_live:
    values["MainMode_ACC"] = 1
    values["VSetDis"] = set_speed
    values["ObjValid"] = 1 if enabled else 0
#    values["ACC_ObjStatus"] = lead_visible

  return packer.make_can_msg("SCC11", 0, values)

def create_scc12(packer, apply_accel, enabled, cnt, scc_live, scc12):
  values = scc12
  values["aReqRaw"] = apply_accel if enabled else 0 #aReqMax
  values["aReqValue"] = apply_accel if enabled else 0 #aReqMin
  values["CR_VSM_Alive"] = cnt
  values["CR_VSM_ChkSum"] = 0
  if not scc_live:
    values["ACCMode"] = 1 if enabled else 0 # 2 if gas padel pressed

  dat = packer.make_can_msg("SCC12", 0, values)[2]
  values["CR_VSM_ChkSum"] = 16 - sum([sum(divmod(i, 16)) for i in dat]) % 16

  return packer.make_can_msg("SCC12", 0, values)

def create_scc13(packer, scc13):
  values = scc13
  return packer.make_can_msg("SCC13", 0, values)

def create_scc14(packer, enabled, scc14):
  values = scc14
  if enabled:
    values["JerkUpperLimit"] = 3.2
    values["JerkLowerLimit"] = 0.1
    values["SCCMode"] = 1
    values["ComfortBandUpper"] = 0.24
    values["ComfortBandLower"] = 0.24

  return packer.make_can_msg("SCC14", 0, values)

def create_spas11(packer, car_fingerprint, frame, en_spas, apply_steer, bus):
  values = {
    "CF_Spas_Stat": en_spas,
    "CF_Spas_TestMode": 0,
    "CR_Spas_StrAngCmd": apply_steer,
    "CF_Spas_BeepAlarm": 0,
    "CF_Spas_Mode_Seq": 2,
    "CF_Spas_AliveCnt": frame % 0x200,
    "CF_Spas_Chksum": 0,
    "CF_Spas_PasVol": 0,
  }
  dat = packer.make_can_msg("SPAS11", 0, values)[2]
  if car_fingerprint in CHECKSUM["crc8"]:
    dat = dat[:6]
    values["CF_Spas_Chksum"] = hyundai_checksum(dat)
  else:
    values["CF_Spas_Chksum"] = sum(dat[:6]) % 256
  return packer.make_can_msg("SPAS11", bus, values)

def create_spas12(bus):
  return [1268, 0, b"\x00\x00\x00\x00\x00\x00\x00\x00", bus]

def create_ems11(packer, ems11, enabled):
  values = ems11
  if enabled:
    values["VS"] = 0
  return packer.make_can_msg("values", 1, ems11)

def create_acc_commands(packer, enabled, accel, idx, lead_visible, set_speed, stopping):
  commands = []

  scc11_values = {
    "MainMode_ACC": 1,
    "TauGapSet": 4,
    "VSetDis": set_speed if enabled else 0,
    "AliveCounterACC": idx % 0x10,
  }
  commands.append(packer.make_can_msg("SCC11", 0, scc11_values))

  scc12_values = {
    "ACCMode": 1 if enabled else 0,
    "StopReq": 1 if stopping else 0,
    "aReqRaw": accel,
    "aReqValue": accel, # stock ramps up at 1.0/s and down at 0.5/s until it reaches aReqRaw
    "CR_VSM_Alive": idx % 0xF,
  }
  scc12_dat = packer.make_can_msg("SCC12", 0, scc12_values)[2]
  scc12_values["CR_VSM_ChkSum"] = 0x10 - sum([sum(divmod(i, 16)) for i in scc12_dat]) % 0x10

  commands.append(packer.make_can_msg("SCC12", 0, scc12_values))

  scc14_values = {
    "ComfortBandUpper": 0.0, # stock usually is 0 but sometimes uses higher values
    "ComfortBandLower": 0.0, # stock usually is 0 but sometimes uses higher values
    "JerkUpperLimit": 1.0 if enabled else 0, # stock usually is 1.0 but sometimes uses higher values
    "JerkLowerLimit": 0.5 if enabled else 0, # stock usually is 0.5 but sometimes uses higher values
    "ACCMode": 1 if enabled else 4, # stock will always be 4 instead of 0 after first disengage
    "ObjGap": 3 if lead_visible else 0, # TODO: 1-5 based on distance to lead vehicle
  }
  commands.append(packer.make_can_msg("SCC14", 0, scc14_values))

  fca11_values = {
    # seems to count 2,1,0,3,2,1,0,3,2,1,0,3,2,1,0,repeat...
    # (where first value is aligned to Supplemental_Counter == 0)
    # test: [(idx % 0xF, -((idx % 0xF) + 2) % 4) for idx in range(0x14)]
    "CR_FCA_Alive": ((-((idx % 0xF) + 2) % 4) << 2) + 1,
    "Supplemental_Counter": idx % 0xF,
  }
  fca11_dat = packer.make_can_msg("FCA11", 0, fca11_values)[2]
  fca11_values["CR_FCA_ChkSum"] = 0x10 - sum([sum(divmod(i, 16)) for i in fca11_dat]) % 0x10
  commands.append(packer.make_can_msg("FCA11", 0, fca11_values))

  return commands

def create_acc_opt(packer):
  commands = []

  scc13_values = {
    "SCCDrvModeRValue": 2,
    "SCC_Equip": 1,
    "Lead_Veh_Dep_Alert_USM": 2,
  }
  commands.append(packer.make_can_msg("SCC13", 0, scc13_values))

  fca12_values = {
    # stock values may be needed if openpilot has vision based AEB some day
    # for now we are not setting these because there is no AEB for vision only
    # "FCA_USM": 3,
    # "FCA_DrvSetState": 2,
  }
  commands.append(packer.make_can_msg("FCA12", 0, fca12_values))

  return commands

def create_frt_radar_opt(packer):
  frt_radar11_values = {
    "CF_FCA_Equip_Front_Radar": 1,
  }
  return packer.make_can_msg("FRT_RADAR11", 0, frt_radar11_values)
