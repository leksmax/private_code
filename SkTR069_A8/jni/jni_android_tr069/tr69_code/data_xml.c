// $Id: data_xml.c 12 2011-02-18 04:05:43Z cedric.shih@gmail.com $
/*
 * Copyright (C) 2010 AXIM Communications Inc.
 * Copyright (C) 2010 Cedric Shih <cedric.shih@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <errno.h>
#include <stdlib.h>


#include "log.h"
#include "data_xml.h"


typedef struct xml_params_info_s 
{
    char*  node;
	 char* name;
	 char* Writable;	 
}xml_params_info;

#define SK_PARAM_INFO  "ParameterInfoStruct"


/**********************************************************************
sunjian:说明 :应该取XML文档模板生成初始化参数
************************************************************************/
// 浙江
#if 0
static xml_params_info m_xmlpara_table[] =
{
		{SK_PARAM_INFO, 	 "Device."   ,"fasle" },
		//{SK_PARAM_INFO, 	 "Device.DeviceSummary"   ,"fasle" },    //error
		{SK_PARAM_INFO, 	 "Device.DeviceInfo."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.Manufacturer"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.ManufacturerOUI"   ,"fasle" },                  //华为用OUI, 标准ManufacturerOUI 修改
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.ModelName"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.ModelID"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.Description"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.ProductClass"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.SerialNumber"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.HardwareVersion"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.SoftwareVersion"   ,"fasle" },
		//{SK_PARAM_INFO, 	 "Device.DeviceInfo.SafeVersion"   ,"fasle" },
		//{SK_PARAM_INFO, 	 "Device.DeviceInfo.EnabledOptions"   ,"fasle" },
		//{SK_PARAM_INFO, 	 "Device.DeviceInfo.AdditionalHardwareVersion"   ,"fasle" },
		//{SK_PARAM_INFO, 	 "Device.DeviceInfo.AdditionalSoftwareVersion"   ,"fasle" },
		//{SK_PARAM_INFO, 	 "Device.DeviceInfo.ProvisioningCode"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.DeviceStatus"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.UpTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.FirstUseDate"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.DeviceLog"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.URL"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.Username"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.Password"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.PeriodicInformEnable"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.PeriodicInformInterval"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.PeriodicInformTime"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.ParameterKey"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.ConnectionRequestURL"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.ConnectionRequestUsername"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.ConnectionRequestPassword"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.UpgradesManaged"   ,"true" },
		//{SK_PARAM_INFO, 	 "Device.ManagementServer.UDPConnectionRequestAddress"   ,"true" },
		//{SK_PARAM_INFO, 	 "Device.ManagementServer.UDPConnectionRequestURL"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.STUNEnable"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.STUNServerAddress"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.STUNServerPort"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.STUNUsername"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.STUNPassword"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.STUNMaximumKeepAlivePeriod"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.STUNMinimumKeepAlivePeriod"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.NATDetected"   ,"true" },
		//{SK_PARAM_INFO, 	 "Device.ManagementServer.UDPConnectionRequestAddressNotificationLimit"   ,"true" },
		//
		{SK_PARAM_INFO, 	 "Device.Time."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.Time.NTPServer1"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.Time.NTPServer2"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.Time.CurrentLocalTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.Time.LocalTimeZone"   ,"true" },
		/*
		{SK_PARAM_INFO, 	 "Device.LAN."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.AddressingType"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.LAN.IPAddress"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.LAN.SubnetMask"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.LAN.DefaultGateway"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.LAN.DNSServers"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.LAN.MACAddress"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.MACAddressOverride"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.LAN.DHCPOptionNumberOfEntries"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.Stats."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.Stats.ConnectionUpTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.Stats.TotalBytesSent"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.Stats.TotalBytesReceived"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.Stats.CurrentDayInterval"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.Stats.CurrentDayBytesSent"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.Stats.CurrentDayBytesReceived"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.Stats.CurrentDayPacketsSent"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.Stats.CurrentDayPacketsReceived"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.Stats.QuarterHourInterval"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.Stats.QuarterHourBytesSent"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.Stats.QuarterHourBytesReceived"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.Stats.QuarterHourPacketsSent"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.Stats.QuarterHourPacketsReceived"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.Stats.TotalPacketsSent"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.Stats.TotalPacketsReceived"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.IPPingDiagnostics."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.IPPingDiagnostics.DiagnosticsState"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.LAN.IPPingDiagnostics.Host"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.LAN.IPPingDiagnostics.NumberOfRepetitions"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.LAN.IPPingDiagnostics.Timeout"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.LAN.IPPingDiagnostics.DataBlockSize"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.LAN.IPPingDiagnostics.DSCP"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.LAN.IPPingDiagnostics.SuccessCount"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.IPPingDiagnostics.FailureCount"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.IPPingDiagnostics.AverageResponseTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.IPPingDiagnostics.MinimumResponseTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.IPPingDiagnostics.MaximumResponseTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.TraceRouteDiagnostics."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.TraceRouteDiagnostics.DiagnosticsState"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.LAN.TraceRouteDiagnostics.Host"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.LAN.TraceRouteDiagnostics.Timeout"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.TraceRouteDiagnostics.DataBlockSize"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.LAN.TraceRouteDiagnostics.MaxHopCount"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.LAN.TraceRouteDiagnostics.DSCP"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.TraceRouteDiagnostics.ResponseTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.TraceRouteDiagnostics.NumberOfRouteHops"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN.TraceRouteDiagnostics.RouteHops."   ,"fasle" },
		*/
		//
		{SK_PARAM_INFO, 	 "Device.STBService."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.STBService.StreamingControlProtocols"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.STBService.StreamingTransportProtocols"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.STBService.StreamingTransportControlProtocols"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.STBService.DownloadTransportProtocols"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.STBService.MultiplexTypes"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.STBService.MaxDejitteringBufferSize"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.STBService.AudioStandards"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.STBService.VideoStandards"   ,"fasle" },
		//
		{SK_PARAM_INFO, 	 "Device.GatewayInfo."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.GatewayInfo.ManufacturerOUI"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.GatewayInfo.ProductClass"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.GatewayInfo.SerialNumber"   ,"true" },
		//
		{SK_PARAM_INFO, 	 "Device.UserInterface."   ,"true" },
		{SK_PARAM_INFO, 	 "Device.UserInterface.PasswordRequired"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.UserInterface.AvailableLanguages"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.UserInterface.CurrentLanguage"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.UserInterface.AutoUpdateServer"   ,"true" },
		//
		{SK_PARAM_INFO, 	 "Device.X_00E0FC."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.STBID"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.PhyMemSize"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StorageSize"   ,"fasle" },
		/*
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.LogServerUrl"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.LogUploadInterval"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.LogRecordInterval"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.MonitoringInterval"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.PacketsLostR1From"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.PacketsLostR1Till"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.PacketsLostR2From"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.PacketsLostR2Till"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.PacketsLostR3From"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.PacketsLostR3Till"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.PacketsLostR4From"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.PacketsLostR4Till"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.PacketsLostR5From"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.PacketsLostR5Till."   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.BitRateR1From"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.BitRateR1Till"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.BitRateR2From"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.BitRateR2Till"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.BitRateR3From"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.BitRateR3Till"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.BitRateR4From"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.BitRateR4Till"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.BitRateR5From"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.StatisticConfiguration.BitRateR5Till"   ,"true" },
		*/
		/*
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.Startpoint"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.Endpoint"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.AuthNumbers"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.AuthFailNumbers"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.AuthFailInfo"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.MultiReqNumbers"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.MultiFailNumbers"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.MultiFailInfo"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.VodReqNumbers"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.VodFailNumbers"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.VodFailInfo"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.HTTPReqNumbers"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.HTTPFailNumbers"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.HTTPFailInfo"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.AbendNumbers"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.AbendInfo"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.MultiPacketsLostR1"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.MultiPacketsLostR2"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.MultiPacketsLostR3"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.MultiPacketsLostR4"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.MultiPacketsLostR5"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.VODPacketsLostR1"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.VODPacketsLostR2"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.VODPacketsLostR3"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.VODPacketsLostR4"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.VODPacketsLostR5"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.BitRateR1"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.BitRateR2"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.BitRateR3"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.BitRateR4"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.BitRateR5"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.PacketsLost"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.JitterMax"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.FrameRate"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceStatistics.FrameLoss"   ,"fasle" },
		*/
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceInfo."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceInfo.UserID"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceInfo.UserIDPassword"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceInfo.AuthURL"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceInfo.PPPoEID"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceInfo.PPPoEPassword"   ,"true" },		
		
		/*
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.SQMConfiguration.SQMLisenPort"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.SQMConfiguration.SQMServerPort"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.PlayDiagnostics.DiagnosticsState"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.PlayDiagnostics.PlayURL"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.PlayDiagnostics.PlayState"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.LogParaConfiguration.LogType"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.LogParaConfiguration.LogLevel"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.LogParaConfiguration.LogOutPutType"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.LogParaConfiguration.SyslogServer"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.LogParaConfiguration.SyslogStartTime"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.LogParaConfiguration.SyslogContinueTime"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.AutoOnOffConfiguration.IsAutoPowerOn"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.AutoOnOffConfiguration.AutoPowerOnTime"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.AutoOnOffConfiguration.AutoShutdownTime"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.AlarmSwitch"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.AlarmReportLevel"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.CPUAlarmValue"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.MemoryAlarmValue"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.DiskAlarmValue"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.PacketsLostAlarmValue"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.BandwidthAlarmValue"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.1."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.1.AlarmSN"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.1.AlarmObjectInstance"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.1.AlarmLocation"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.1.AlarmCode"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.1.AlarmRaisedTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.1.AlarmClearedTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.1.PerceivedSeverity"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.1.AdditionalInformation"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.2."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.2.AlarmSN"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.2.AlarmObjectInstance"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.2.AlarmLocation"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.2.AlarmCode"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.2.AlarmRaisedTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.2.AlarmClearedTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.2.PerceivedSeverity"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.2.AdditionalInformation"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.3."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.3.AlarmSN"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.3.AlarmObjectInstance"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.3.AlarmLocation"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.3.AlarmCode"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.3.AlarmRaisedTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.3.AlarmClearedTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.3.PerceivedSeverity"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.3.AdditionalInformation"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.4."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.4.AlarmSN"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.4.AlarmObjectInstance"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.4.AlarmLocation"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.4.AlarmCode"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.4.AlarmRaisedTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.4.AlarmClearedTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.4.PerceivedSeverity"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.4.AdditionalInformation"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.5."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.5.AlarmSN"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.5.AlarmObjectInstance"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.5.AlarmLocation"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.5.AlarmCode"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.5.AlarmRaisedTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.5.AlarmClearedTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.5.PerceivedSeverity"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.5.AdditionalInformation"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.6."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.6.AlarmSN"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.6.AlarmObjectInstance"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.6.AlarmLocation"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.6.AlarmCode"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.6.AlarmRaisedTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.6.AlarmClearedTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.6.PerceivedSeverity"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.6.AdditionalInformation"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.7."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.7.AlarmSN"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.7.AlarmObjectInstance"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.7.AlarmLocation"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.7.AlarmCode"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.7.AlarmRaisedTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.7.AlarmClearedTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.7.PerceivedSeverity"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.7.AdditionalInformation"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.8."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.8.AlarmSN"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.8.AlarmObjectInstance"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.8.AlarmLocation"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.8.AlarmCode"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.8.AlarmRaisedTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.8.AlarmClearedTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.8.PerceivedSeverity"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.Alarm.8.AdditionalInformation"   ,"fasle" },			
		*/			
};
#endif


/**************************************************************
四川电信
***************************************************************/
/*
   static xml_params_info m_xmlpara_table[] =
{
		{SK_PARAM_INFO, 	 "Device.DeviceSummary"   ,"fasle" },    //error
		{SK_PARAM_INFO, 	 "Device.STBService."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.GatewayInfo."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.Time."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.LAN."   ,"false" },
		{SK_PARAM_INFO, 	 "Device.X_CTC_IPTV."   ,"false" },
		
};
*/
/***************************************************************
广东移动
****************************************************************/
/*
static xml_params_info m_xmlpara_table[] =
{
		{SK_PARAM_INFO, 	 "Device."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceSummary"   ,"fasle" },    //error
		{SK_PARAM_INFO, 	 "Device.DeviceInfo."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.Manufacturer"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.ManufacturerOUI"   ,"fasle" },                  //华为用OUI, 标准ManufacturerOUI 修改
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.ModelName"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.ModelID"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.Description"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.ProductClass"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.SerialNumber"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.HardwareVersion"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.SoftwareVersion"   ,"fasle" },
	
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.DeviceStatus"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.UpTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo.FirstUseDate"   ,"fasle" },		
		{SK_PARAM_INFO, 	 "Device.ManagementServer."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.URL"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.Username"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.Password"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.PeriodicInformEnable"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.PeriodicInformInterval"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.PeriodicInformTime"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.ParameterKey"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.ConnectionRequestURL"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.ConnectionRequestUsername"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.ConnectionRequestPassword"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.UpgradesManaged"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.UDPConnectionRequestAddress"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.UDPConnectionRequestURL"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.STUNEnable"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.STUNServerAddress"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.STUNServerPort"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.STUNUsername"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.STUNPassword"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.STUNMaximumKeepAlivePeriod"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.STUNMinimumKeepAlivePeriod"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.ManagementServer.NATDetected"   ,"true" },
		//{SK_PARAM_INFO, 	 "Device.ManagementServer.UDPConnectionRequestAddressNotificationLimit"   ,"true" },
		//
		{SK_PARAM_INFO, 	 "Device.Time."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.Time.NTPServer1"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.Time.NTPServer2"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.Time.CurrentLocalTime"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.Time.LocalTimeZone"   ,"true" },
		
		//
		{SK_PARAM_INFO, 	 "Device.STBService."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.STBService.StreamingControlProtocols"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.STBService.StreamingTransportProtocols"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.STBService.StreamingTransportControlProtocols"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.STBService.DownloadTransportProtocols"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.STBService.MultiplexTypes"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.STBService.MaxDejitteringBufferSize"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.STBService.AudioStandards"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.STBService.VideoStandards"   ,"fasle" },
		
		//X_00E0FC:
		{SK_PARAM_INFO, 	 "Device.X_00E0FC."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.STBID"   ,"fasle" },		
		
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceInfo."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceInfo.UserID"   ,"true" },		
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceInfo.AuthURL"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceInfo.PPPoEID"   ,"true" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceInfo.PPPoEPassword"   ,"true" },	
		//sunjian:X_CMCC_OTV
		{SK_PARAM_INFO, 	 "Device.X_CMCC_OTV."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_CMCC_OTV.STBInfo.STBID"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_CMCC_OTV.STBInfo.PhyMemSize"   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_CMCC_OTV.STBInfo.StorageSize"   ,"fasle" },		
		
		{SK_PARAM_INFO, 	 "Device.X_CMCC_OTV.ServiceInfo."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_CMCC_OTV.ServiceInfo.UserID"   ,"true" },		
		{SK_PARAM_INFO, 	 "Device.X_CMCC_OTV.ServiceInfo.AuthURL"   ,"true" },
		//{SK_PARAM_INFO, 	 "Device.X_CMCC_OTV.ServiceInfo.PPPoEID"   ,"true" },
		//{SK_PARAM_INFO, 	 "Device.X_CMCC_OTV.ServiceInfo.PPPoEPassword"   ,"true" },	
	
};

*/

static xml_params_info m_xmlpara_table[] =
{
		{SK_PARAM_INFO, 	 "Device."   ,"fasle" },    //error
		{SK_PARAM_INFO, 	 "Device.STBService."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.DeviceInfo."   ,"fasle" },
		//{SK_PARAM_INFO, 	 "Device.ManagementServer."   ,"fasle" },
		//{SK_PARAM_INFO, 	 "Device.GatewayInfo."   ,"fasle" },
		//{SK_PARAM_INFO, 	 "Device.Time."   ,"fasle" },
		//{SK_PARAM_INFO, 	 "Device.LAN."   ,"false" },
		//{SK_PARAM_INFO, 	 "Device.X_CTC_IPTV."   ,"false" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_00E0FC.ServiceInfo."   ,"fasle" },
		{SK_PARAM_INFO, 	 "Device.X_CMCC_OTV."   ,"fasle" },
		
};


int evcpe_device_id_to_xml(struct evcpe_device_id *id,
		const char *node, struct evbuffer *buffer)
{
	int rc;

	if ((rc = evcpe_add_buffer(buffer, "<%s>\n", node)))
		goto finally;
	if ((rc = evcpe_xml_add_string(buffer, "Manufacturer", id->manufacturer)))
		goto finally;
	if ((rc = evcpe_xml_add_string(buffer, "OUI", id->oui)))
		goto finally;
	if ((rc = evcpe_xml_add_string(buffer, "ProductClass", id->product_class)))
		goto finally;
	if ((rc = evcpe_xml_add_string(buffer, "SerialNumber", id->serial_number)))
		goto finally;
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;

finally:
	return rc;
}

int evcpe_event_to_xml(struct evcpe_event *event,
		const char *node, struct evbuffer *buffer)
{
	int rc;
	//xdl 20120817
	char value[64]={0};
	sk_api_params_get("tr069_commandkey",value,64);
	//	printf("---xdl---tr069_commandkey:%s\n",value);
	if ((rc = evcpe_add_buffer(buffer, "<%s>\n", node)))
		goto finally;
	if ((rc = evcpe_xml_add_string(buffer,
			"EventCode", event->event_code)))
		goto finally;
//	if ((rc = evcpe_xml_add_string(buffer,	"CommandKey", event->command_key)))
	if ((rc = evcpe_xml_add_string(buffer,	"CommandKey", value)))
		goto finally;
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;

finally:
	return rc;
}

int evcpe_event_list_to_xml(struct evcpe_event_list *list,
		const char *node, struct evbuffer *buffer)
{
	int rc;
	struct evcpe_event *event;

	if ((rc = evcpe_add_buffer(buffer,
			"<%s "EVCPE_SOAP_ENC_XMLNS":arrayType=\""EVCPE_CWMP_XMLNS":EventStruct[%d]\">\n",
			node, evcpe_event_list_size(list))))
		goto finally;
	TAILQ_FOREACH(event, &list->head, entry) {
		if ((rc = evcpe_event_to_xml(event, "EventStruct",
				buffer)))
			goto finally;
	}
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;
	rc = 0;

finally:
	return rc;
}

int evcpe_param_value_to_xml(struct evcpe_param_value *value,
		const char *node, struct evbuffer *buffer)
{
	int rc;

	if ((rc = evcpe_add_buffer(buffer, "<%s>\n", node)))
		goto finally;
//	if ((rc = evcpe_add_buffer(buffer, "<%s xsi:type=\"cwmp:ParameterValueStruct\">\n", node)))
//		goto finally;
	if ((rc = evcpe_add_buffer(buffer, "<Name>%s</Name>\n",
			value->name)))
		goto finally;
//	if ((rc = evcpe_add_buffer(buffer, "<Name xsi:type=\"xsd:string\">%s</Name>\n",
//			value->name)))
//		goto finally;
	if ((rc = evcpe_add_buffer(buffer, "<Value xsi:type=\"xsd:%s\">%.*s</Value>\n",
			evcpe_type_to_str(value->type), value->len, value->data)))
		goto finally;
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;

finally:
	return rc;
}

int evcpe_param_value_list_to_xml(struct evcpe_param_value_list *list,
		const char *node, struct evbuffer *buffer)
{

	int rc;
	struct evcpe_param_value *value;
    
	if ((rc = evcpe_add_buffer(buffer,
        "<%s "EVCPE_SOAP_ENC_XMLNS":arrayType=\""EVCPE_CWMP_XMLNS":ParameterValueStruct[%d]\">\n",
        node, evcpe_param_value_list_size(list))))
    goto finally;
    	
	TAILQ_FOREACH(value, &list->head, entry) 
	{
		if ((rc = evcpe_param_value_to_xml(value, "ParameterValueStruct",
				buffer)))
			goto finally;
	}
	
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;
	rc = 0;

finally:
	return rc;
}


int evcpe_param_value_list_to_xml_for_value_change(struct evcpe_param_value_list *list,
		const char *node, struct evbuffer *buffer)
{

	int rc;
	int natdetectflag = -1;
	int  ulparams=0;
	struct evcpe_param_value *value;
	
 	//add by lijingchao
 	natdetectflag = sk_get_nat_stun_flag();
    
	ulparams = evcpe_param_value_list_size(list);
	
    if(1 == natdetectflag)
   	{
   		ulparams++;
        ulparams++;  //江苏移动项目，需要格外添加参数Device.ManagementServer.UDPConnectionRequestAddress
		evcpe_info(__func__ , "sevcpe_param_value_list_to_xml  natdetectflag = 1\n"); 
	    if ((rc = evcpe_add_buffer(buffer,
			"<%s "EVCPE_SOAP_ENC_XMLNS":arrayType=\""EVCPE_CWMP_XMLNS":ParameterValueStruct[%d]\">\n",
			node, ulparams)))
		goto finally;
   	}
    else if( 0 == natdetectflag)
    {
        ulparams++;
        evcpe_info(__func__ , "sevcpe_param_value_list_to_xml  natdetectflag = 0\n"); 
    	if ((rc = evcpe_add_buffer(buffer,
            "<%s "EVCPE_SOAP_ENC_XMLNS":arrayType=\""EVCPE_CWMP_XMLNS":ParameterValueStruct[%d]\">\n",
            node, ulparams)))
        goto finally;
    	
    }
    else
    {
        if ((rc = evcpe_add_buffer(buffer,
        "<%s "EVCPE_SOAP_ENC_XMLNS":arrayType=\""EVCPE_CWMP_XMLNS":ParameterValueStruct[%d]\">\n",
        node, evcpe_param_value_list_size(list))))
        goto finally;
    }
    
	TAILQ_FOREACH(value, &list->head, entry) 
	{
		if ((rc = evcpe_param_value_to_xml(value, "ParameterValueStruct",
				buffer)))
			goto finally;
	}
    
	/*****************************************************/
    if(1 == natdetectflag)
	{
		evcpe_info(__func__ , "add set_NATDetected \n");
		evcpe_add_buffer(buffer,"%s","<ParameterValueStruct>\n<Name>Device.ManagementServer.NATDetected</Name>\n<Value xsi:type=\"xsd:string\">true</Value>\n</ParameterValueStruct>\n");
        evcpe_add_buffer(buffer , "<ParameterValueStruct>\n<Name>Device.ManagementServer.UDPConnectionRequestAddress</Name>\n<Value xsi:type=\"xsd:string\">%s</Value>\n</ParameterValueStruct>\n",STUN_get_UDPConnectionRequestAddress());
    
	}
    else if(0 == natdetectflag)
    {
     
         evcpe_add_buffer(buffer,"%s","<ParameterValueStruct>\n<Name>Device.ManagementServer.NATDetected</Name>\n<Value xsi:type=\"xsd:string\">false</Value>\n</ParameterValueStruct>\n");
 
    }
	/*******************************************************/
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;
	rc = 0;

finally:
	return rc;
}

int evcpe_param_attr_to_xml(struct evcpe_param_attr *info,
		const char *node, struct evbuffer *buffer)
{
	int rc=0;

	if ((rc = evcpe_add_buffer(buffer, "<%s>\n", node)))
		goto finally;
	
	//if(!strcmp("Device.Time.CurrentLocalTime",info->name))
	//{
	//	if((rc=evcpe_xml_add_datetime(buffer,"Name", info->name)))
	//		goto finally;
	//}
	//else
	{
		if ((rc = evcpe_xml_add_string(buffer, "Name", info->name)))
		goto finally;
	}
	if ((rc = evcpe_xml_add_int(buffer, "Notification", 0)))
		goto finally;
//	if ((rc = evcpe_xml_add_xsd_boolean(buffer, "AccessListChange", 0)))
//		goto finally;
	
	if ((rc = evcpe_xml_add_str_array(buffer, "AccessList")))
		goto finally;
	
		
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;
	rc=0;

finally:
	return rc;
}

int evcpe_param_attr_list_to_xml(struct evcpe_param_attr_list *list,
		const char *node, struct evbuffer *buffer)
{
	int rc;
	struct evcpe_param_attr *info;

	if ((rc = evcpe_add_buffer(buffer,
			"<%s "EVCPE_SOAP_ENC_XMLNS":arrayType=\""EVCPE_CWMP_XMLNS":ParameterAttributeStruct[%d]\">\n",
			node, evcpe_param_attr_list_size(list))))
		goto finally;
	TAILQ_FOREACH(info, &list->head, entry) 
	{
		//printf("evcpe_param_attr_list_to_xml  ---------info->name=%s\n",info->name);
		if ((rc = evcpe_param_attr_to_xml(info, "ParameterAttributesStruct",
				buffer)))
		{
			//printf("evcpe_param_attr_list_to_xml  2 ---------info->name=%s\n",info->name);
			goto finally;
		}	
		//printf("evcpe_param_attr_list_to_xml 3 ---------info->name=%s\n",info->name);
	}
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;
	rc = 0;

finally:
	return rc;
}



int evcpe_param_info_to_xml(struct evcpe_param_info *info,
		const char *node, struct evbuffer *buffer)
{
	int rc;

	if ((rc = evcpe_add_buffer(buffer, "<%s>\n", node)))
		goto finally;
	if ((rc = evcpe_xml_add_xsd_string(buffer, "Name", info->name)))
		goto finally;
	if ((rc = evcpe_xml_add_xsd_boolean(buffer, "Writable", info->writable)))
		goto finally;
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;

finally:
	return rc;
}

int evcpe_param_info_list_to_xml(struct evcpe_param_info_list *list,
		const char *node, struct evbuffer *buffer)
{
	int rc;
	struct evcpe_param_info *info;

	if ((rc = evcpe_add_buffer(buffer,
			"<%s "EVCPE_SOAP_ENC_XMLNS":arrayType=\""EVCPE_CWMP_XMLNS":ParameterInfoStruct[%d]\">\n",
			node, evcpe_param_info_list_size(list))))
		goto finally;
	TAILQ_FOREACH(info, &list->head, entry) {
		if ((rc = evcpe_param_info_to_xml(info, "ParameterInfoStruct",
				buffer)))
			goto finally;
	}
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;
	rc = 0;

finally:
	return rc;
}

int evcpe_method_list_to_xml(struct evcpe_method_list *list, const char *node,
		struct evbuffer *buffer)
{
	int rc;
	struct evcpe_method_list_item *item;

	if ((rc = evcpe_add_buffer(buffer,
			"<%s "EVCPE_SOAP_ENC_XMLNS":arrayType=\"xsd:string[%d]\">\n",
			node, evcpe_method_list_size(list))))
		goto finally;
	TAILQ_FOREACH(item, &list->head, entry) {
		if ((rc = evcpe_xml_add_string(buffer, "string",
				item->name)))
			goto finally;
	}
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;
	rc = 0;

finally:
	return rc;

}


/************************************************************
孙健:浙江网管
*************************************************************/


int evcpe_param_name_info_list_to_xml(struct evcpe_param_info_list *list,
		const char *node, struct evbuffer *buffer)
{
	int i,rc;
	int count= sizeof(m_xmlpara_table) / sizeof(xml_params_info);
	
	struct evcpe_param_info *info;
    count=list->size;
	evcpe_info(__func__ , "[evcpe_param_name_info_list_to_xml ] count:%d,node:%s\n",count,node);
	printf("evcpe_param_name_info_list_to_xml node:%s, count:%d\n",node,list->size);
    //
	if ((rc = evcpe_add_buffer(buffer,
			"<%s "EVCPE_SOAP_ENC_XMLNS":arrayType=\""EVCPE_CWMP_XMLNS":ParameterInfoStruct[%d]\">\n",
			node, count)))
		goto finally;
#if 1
	TAILQ_FOREACH(info, &list->head, entry) {
		if ((rc = evcpe_param_info_to_xml(info, "ParameterInfoStruct",
				buffer)))
			goto finally;
	}
#else
	count= sizeof(m_xmlpara_table) / sizeof(xml_params_info);
 	LOGI("[evcpe_param_name_info_list_to_xml ] count:%d,node:%s\n",count,node);
	for(i=0;i<count;i++)
	{
		if ((rc = evcpe_add_buffer(buffer, "<%s>\n", m_xmlpara_table[i].node)))
		goto finally;
		if ((rc = evcpe_xml_add_xsd_string(buffer, "Name", m_xmlpara_table[i].name)))
		goto finally;
	if ((rc = evcpe_xml_add_xsd_boolean(buffer, "Writable", m_xmlpara_table[i].Writable)))
		goto finally;
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", m_xmlpara_table[i].node)))
		goto finally;
	}
#endif
	if ((rc = evcpe_add_buffer(buffer, "</%s>\n", node)))
		goto finally;
	rc = 0;

finally:
	return rc;
}



