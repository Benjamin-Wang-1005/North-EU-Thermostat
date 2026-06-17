//---------------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 E-Poly Technology Co., Ltd. All rights reserved.
//
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// E-Poly North EU Thermostat Project
// Author: Benjamin Wang
// Date: 2026/06/03
// Email: Benjamin@epoly-tech.com
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// File Function: Language string table definition
//---------------------------------------------------------------------------------------------------------

#include "Language.h"

const char* const LanguageTable[STR_MAX][LANG_MAX] = {
	// [STR_Save]
	{
		[LANG_ENGLISH] = "Save",
		[LANG_Norwegian] = "Lagre",
	},
	// [STR_Temperature_Read_From]
	{
		[LANG_ENGLISH] = "Temp.Read From",
		[LANG_Norwegian] = "Temp.lest fra",
	},
	// [STR_Room]
	{
		[LANG_ENGLISH] = "Room",
		[LANG_Norwegian] = "Rom",
	},
	// [STR_Floor]
	{
		[LANG_ENGLISH] = "Floor",
		[LANG_Norwegian] = "Gulv",
	},
	// [STR_Sensor_Calibrate]
	{
		[LANG_ENGLISH] = "Sensor Calibrate",
		[LANG_Norwegian] = "Sensorkalibrering",
	},
	// [STR_Temperature_Limit]
	{
		[LANG_ENGLISH] = "Temp.Limit",
		[LANG_Norwegian] = "Temperaturgrense",
	},
	// [STR_Max]
	{
		[LANG_ENGLISH] = "Max",
		[LANG_Norwegian] = "Maks",
	},
	// [STR_Min]
	{
		[LANG_ENGLISH] = "Min",
		[LANG_Norwegian] = "Min",
	},
	// [STR_Max_Temperature_Protect]
	{
		[LANG_ENGLISH] = "Max Temp.Protect",
		[LANG_Norwegian] = "Maks temperaturbeskyttelse",
	},
	// [STR_Min_Temperature_Protect]
	{
		[LANG_ENGLISH] = "Min Temp.Protect",
		[LANG_Norwegian] = "Min temperaturbeskyttelse",
	},
	// [STR_Power_On_State]
	{
		[LANG_ENGLISH] = "Power On State",
		[LANG_Norwegian] = "Status ved oppstart",
	},
	// [STR_Keep_State]
	{
		[LANG_ENGLISH] = "Keep State",
		[LANG_Norwegian] = "Behold status",
	},
	// [STR_Device_Close]
	{
		[LANG_ENGLISH] = "Device Close",
		[LANG_Norwegian] = "Enhet lukket",
	},
	// [STR_Device_Open]
	{
		[LANG_ENGLISH] = "Device Open",
		[LANG_Norwegian] = "Enhet \xE5pen",
	},
	// [STR_Power_Limit]
	{
		[LANG_ENGLISH] = "Power Limit",
		[LANG_Norwegian] = "Str\xF8mgrense",
	},
	// [STR_OFF]
	{
		[LANG_ENGLISH] = "OFF",
		[LANG_Norwegian] = "AV",
	},
	// [STR_ON]
	{
		[LANG_ENGLISH] = "ON",
		[LANG_Norwegian] = "P\xC5",
	},
	// [STR_Set_Floor_Material]
	{
		[LANG_ENGLISH] = "Set Floor Material",
		[LANG_Norwegian] = "Velg gulvmateriale",
	},
	// [STR_Language]
	{
		[LANG_ENGLISH] = "Language",
		[LANG_Norwegian] = "Spr\xE5k"
	},
	// [STR_Menu_Language]
	{
		[LANG_ENGLISH] = "Menu Language",
		[LANG_Norwegian] = "Menyspr\xE5k",
	},
	// [STR_English]
	{
		[LANG_ENGLISH] = "English",
		[LANG_Norwegian] = "Engelsk"
	},
	// [STR_Norwegian]
	{
		[LANG_ENGLISH] = "Norwegian",
		[LANG_Norwegian] = "norsk",
	},
	// [STR_Comfort_Mode]
	{
		[LANG_ENGLISH] = "Comfort Mode",
		[LANG_Norwegian] = "Komfortmodus",
	},
	// [STR_Wood_Laminate]
	{
		[LANG_ENGLISH] = "Wood/Laminate",
		[LANG_Norwegian] = "Tre/Laminat",
	},
	// [STR_Tile_Concrete]
	{
		[LANG_ENGLISH] = "Tile/Concrete",
		[LANG_Norwegian] = "Flis/Betong",
	},
	// [STR_Fast_Response]
	{
		[LANG_ENGLISH] = "Fast Response",
		[LANG_Norwegian] = "Rask respons",
	},
	// [STR_Floor_Type]
	{
		[LANG_ENGLISH] = "Floor Type",
		[LANG_Norwegian] = "Gulvtype",
	},
	// [STR_Temperature_Swing]
	{
		[LANG_ENGLISH] = "Temp.Swing",
		[LANG_Norwegian] = "Temperaturavvik",
	},
	// [STR_Program_Type]
	{
		[LANG_ENGLISH] = "Program Type",
		[LANG_Norwegian] = "Programtype",
	},
	// [STR_Workday_Setting]
	{
		[LANG_ENGLISH] = "Workday Setting",
		[LANG_Norwegian] = "Innstilling virkedag",
	},
	// [STR_Restday_Setting]
	{
		[LANG_ENGLISH] = "Restday Setting",
		[LANG_Norwegian] = "Innstilling fridag",
	},
	// [STR_Current_Type]
	{
		[LANG_ENGLISH] = "Current Type:",
		[LANG_Norwegian] = "Gjeldende type:",
	},
	// [STR_Finish]
	{
		[LANG_ENGLISH] = "Finish",
		[LANG_Norwegian] = "Fullf\xF8r",
	},
	// [STR_Period_1_On]
	{
		[LANG_ENGLISH] = "Period 1 On",
		[LANG_Norwegian] = "Periode 1 P\xE5",
	},
	// [STR_Period_1_Off]
	{
		[LANG_ENGLISH] = "Period 1 Off",
		[LANG_Norwegian] = "Periode 1 Av",
	},
	// [STR_Leave_Schedule]
	{
		[LANG_ENGLISH] = "Leave Schedule",
		[LANG_Norwegian] = "Forlat tidsplan",
	},
	// [STR_Mode]
	{
		[LANG_ENGLISH] = "Mode",
		[LANG_Norwegian] = "Modus",
	},
	// [STR_Cancel]
	{
		[LANG_ENGLISH] = "Cancel",
		[LANG_Norwegian] = "Avbryt",
	},
	// [STR_Yes]
	{
		[LANG_ENGLISH] = "Yes",
		[LANG_Norwegian] = "Ja",
	},
	// [STR_Window_Open]
	{
		[LANG_ENGLISH] = "Window Open",
		[LANG_Norwegian] = "\xC5pent vindu",
	},
	// [STR_Detected]
	{
		[LANG_ENGLISH] = "Detected!",
		[LANG_Norwegian] = "Detektert!",
	},
	// [STR_Cancel_Detect]
	{
		[LANG_ENGLISH] = "Cancel Detect",
		[LANG_Norwegian] = "Avbryt deteksjon",
	},
	// [STR_Reset_Detect]
	{
		[LANG_ENGLISH] = "Reset Detect",
		[LANG_Norwegian] = "Nullstill deteksjon",
	},
	// [STR_FRI]
	//{
	//	[LANG_ENGLISH] = "FRI",
	//	[LANG_Norwegian] = "FRE",
	//},
	// [STR_MON]
	//{
	//	[LANG_ENGLISH] = "MON",
	//	[LANG_Norwegian] = "MAN",
	//},
	// [STR_SAT]
	//{
	//	[LANG_ENGLISH] = "SAT",
	//	[LANG_Norwegian] = "L\xD8R",
	//},
	// [STR_SUN]
	//{
	//	[LANG_ENGLISH] = "SUN",
	//	[LANG_Norwegian] = "S\xD8N",
	//},
	// [STR_THU]
	//{
	//	[LANG_ENGLISH] = "THU",
	//	[LANG_Norwegian] = "TOR",
	//},
	// [STR_TUE]
	//{
	//	[LANG_ENGLISH] = "TUE",
	//	[LANG_Norwegian] = "TIR",
	//},
	// [STR_WED]
	//{
	//	[LANG_ENGLISH] = "WED",
	//	[LANG_Norwegian] = "ONS",
	//},
	// [STR_Sensor]
	{
		[LANG_ENGLISH] = "Sensor",
		[LANG_Norwegian] = "Sensor",
	},
	// [STR_Input_Temperature_Limit]
	{
		[LANG_ENGLISH] = "Input Temp.Limit",
		[LANG_Norwegian] = "Inndata temperaturgrense",
	},
	// [STR_Protect_Temperature]
	{
		[LANG_ENGLISH] = "Protect Temp.",
		[LANG_Norwegian] = "Beskyttelsestemperatur",
	},
	// [STR_Child_lock]
	{
		[LANG_ENGLISH] = "Child Lock",
		[LANG_Norwegian] = "Barnesikring",
	},
	// [STR_Window_Function]
	{
		[LANG_ENGLISH] = "Window Function",
		[LANG_Norwegian] = "Vindusfunksjon",
	},
	// [STR_Set_Time]
	{
		[LANG_ENGLISH] = "Set Time",
		[LANG_Norwegian] = "Still inn tid",
	},
	// [STR_Set_Backlight]
	{
		[LANG_ENGLISH] = "Set Backlight",
		[LANG_Norwegian] = "Still inn bakgrunnsbelysning",
	},
	// [STR_Factory_Reset]
	{
		[LANG_ENGLISH] = "Factory Reset",
		[LANG_Norwegian] = "Fabrikkinnstilling",
	},
	// [STR_Operation_Mode]
	{
		[LANG_ENGLISH] = "Operation Mode",
		[LANG_Norwegian] = "Driftsmodus",
	},
	// [STR_Heating_Schedule]
	{
		[LANG_ENGLISH] = "Heating Schedule",
		[LANG_Norwegian] = "Varmeplan",
	},
	// [STR_Control_Adjustment]
	{
		[LANG_ENGLISH] = "Control Adj.",
		[LANG_Norwegian] = "Reguleringsjustering",
	},
	// [STR_User_Settings]
	{
		[LANG_ENGLISH] = "User Settings",
		[LANG_Norwegian] = "Brukerinnstillinger",
	},
	// [STR_Manual_Mode]
	{
		[LANG_ENGLISH] = "Manual Mode",
		[LANG_Norwegian] = "Manuell modus",
	},
	// [STR_Schedule_Mode]
	{
		[LANG_ENGLISH] = "Schedule Mode",
		[LANG_Norwegian] = "Tidsplanmodus",
	},
	// [STR_Enter_Pin_Code]
	{
		[LANG_ENGLISH] = "Enter Pin Code",
		[LANG_Norwegian] = "Tast inn PIN-kode",
	},
	// [STR_User_Setting]
	{
		[LANG_ENGLISH] = "User Setting",
		[LANG_Norwegian] = "Brukerinnstilling",
	},
	// [STR_Set_Clock]
	{
		[LANG_ENGLISH] = "Set Clock",
		[LANG_Norwegian] = "Still inn klokke",
	},
	// [STR_Trigger_Temperature_Time]
	{
		[LANG_ENGLISH] = "Trigger Temp / Time",
		[LANG_Norwegian] = "Utl\xF8ser temperatur/tid",
	},
	// [STR_Temperature]
	{
		[LANG_ENGLISH] = "Temp.",
		[LANG_Norwegian] = "Temp.",
	},
	// [STR_Time]
	{
		[LANG_ENGLISH] = "Time",
		[LANG_Norwegian] = "Tid",
	},
	// [STR_Year_Month_Date]
	{
		[LANG_ENGLISH] = "  Year Month Date",
		[LANG_Norwegian] = "  \xC5r M\xE5ned Dato",
	},
	// [STR_Hour_Minute]
	{
		[LANG_ENGLISH] = " Hour  Minute",
		[LANG_Norwegian] = " Time  Minutt",
	},
	// [STR_Reset]
	{
		[LANG_ENGLISH] = "Reset",
		[LANG_Norwegian] = "Nullstill",
	},
	// [STR_Reset_to_Default]
	{
		[LANG_ENGLISH] = "Reset to Default?",
		[LANG_Norwegian] = "Nullstill til fabrikkstandard?",
	},
	// [STR_This_will_erase]
	{
		[LANG_ENGLISH] = "This will erase",
		[LANG_Norwegian] = "Dette vil slette alle",
	},
	// [STR_all_saved_settings]
	{
		[LANG_ENGLISH] = "all saved settings.",
		[LANG_Norwegian] = "lagrede innstillinger.",
	},
	// [STR_Please_Setup_Pin]
	{
		[LANG_ENGLISH] = "Please Setup Pin",
		[LANG_Norwegian] = "Vennligst opprett PIN",
	},
	// [STR_Please_Confirm_Pin]
	{
		[LANG_ENGLISH] = "Please Confirm Pin",
		[LANG_Norwegian] = "Vennligst bekreft PIN",
	}
};
