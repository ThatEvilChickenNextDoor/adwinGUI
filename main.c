//IDEAS:
/*
Add a killswitch...to interupt the ADwin hardware.  Dangerous because it will leave the outputs in
			whatever state they had at the time the system was 'killed.  Could be fixed by adding a
			bit to the ADBasic code that recognizes the kill switch and sets channels low.

*/
/*
Caveats:
If making any changes to the program, be aware of several inconsistencies.
Adwin software is base 1 for arrays
C (LabWindows) is base 0 for arrays,
however some controls for LabWindows are base 1   : Tables
and some are base 0								  : Listboxes

2010:
Aug 2:  change to the "ADbasic binary file: "TransferData_August02_2010.TB1"
		purpose: activate DIO 1 and DIO 2 outputs for Aubin Lab (W&M) sequencer.

2006:
Mar 9:  Added the ability to loop between 2 columns of the panel, spanning pages if necessary.  Could be used
		for repetitive outputs.. e.g. cycling between MOT and an optical probe.
		(need to be careful with DDS programming)
Mar 9: 	Force DDS clock frequency to be set at run-time.
		Fixed bug that caused the DDS trap bottom to set to 0 after doing a Scan.

Feb 9   Noticed a bug.. probably been there for a VERY long time.  The 'Debug Window' has been slowly accumulating
		text, and not being cleared.  Change code to clear the debug text window before saving the panel


2005
July 29 Added option to output a history of the Scan values when running a scan.
		Adding option to stream the panel to text files;
JULY 20 commands for DDS2 are now generated, also sent to ADwin now (see new ADBasic code  TransferData_Jul20.BAS)
		Fixed ADBasic software bug.  Turns out that DIG_WRITELATCH1 and DIG_WRITELATCH2 (for lower and upper 16 bits respectively)
		are incompatible.  Use DIG_WRITELATCH32 instead.
July 18 Added 3rd DDS interface, simplified DDS control by reusing the existing DDS control panel
		DDS 2 and 3 clock settings displayed in DDSSettings.uir  Not modifiable. Set using a #define in vars.h
		Save DDS 2,3 info to file.
July4   Adding 2nd DDS (interface only, creating dual dds command structure still needs to be done.)
June7   Finalized scan programming.  Now scans in amplitude, time or DDS frequency. (Only DDS1 so far)
April 20 Changed the way that the table cells are coloured.  Now all cells are coloured based on
		the information in the cell.  No longer based on the history of that row.
		Sine wave output now relabels amplitude and frequency on analog control panel. Colours Cyan on table.
April 7	Fixed a bug where we didn't reach the final value on a ramp, but reached the value before.
		Cause: in calculating ramps, i.e determine slope=amplitude/number of steps
				but should be amplitude/number_steps-1
Mar 23	Added A Frequency OFfset to the DDS...so the same ramp can be continually used while changing the
		trap bottom.
		Fixed duration of ramps etc at column duration.
Mar 10	Fixed a problem where the frequency ramps generated by the DDS finished in half the expected time
		Can't find a reason for this, except that the DDS manual might be wrong.
		Added a Sinewave option to the list of possible functions.  Only accepts amplitude and frequency..no bias
		  -  bias could be 'worked around' using bias setting under Analog Channel Setup
Jan 18	Add menu option to turn off the DDS for all cells.
		Avoids a string of warnings created by the DDS command routines if a DDS command is written before the
		previous DDS command was done;
Jan 5   Fix bug in code where the timing isn't always copied into the DDS commands
		Fixed the last panel mobile ability


//Update History
2004
Apr 15:  Run button flashes with ADwin is operating.
Apr 29:  Fixed Digital line 16.  Loop counted to 15, should have been 16
May 04:  Adding code to delete/insert columns.
May 06:  Added Copy/Paste fcns for column.  Doesn't work 100% yet.... test that channels 16 work for dig. and analog.
May 10:  Fixed a bug where the arrays weren't properly initialized, causing strange values to be written to the adwin
		 Added flashing led's to notify when the ADwin is doing something
May 13:  Fixed 16th line of the panel, now is actually output.  Bug was result of inconsistency with arrays.
		 i.e. Base 0 vs base 1.
		 - fixed by increasing the internal array size to 17 and starting from 1. (dont read element 0)
May 18:  Improved performance.  Added a variable (ChangedVals) that is 0 when no changes have been made since
		the last run.  If ChangedVals==0, then don't recreate/resend the arrays, shortens delay between runs.
June24:	Add support for more analog output cards.  Change the #define NUMBEROFANALOGCHANNELS xx to reflect the number of channels.
		NOTE:  You need to change this in every .c file.
		Still need to update the code to use a different channel for digital.  (currently using 17, which will be overwritten
		if using more than 16 analog channels.
July26: Begin adding code to control the DDS (AD9854 from Analog Devices)
		Use an extra line on the analog table (17 or 25) as the DDS control interface
AUg		Include DDS control
Nov		DDS control re-written by Ian Leroux
Dec7	Add a compression routine for the NumberUpdates variable, to speed up communication with ADwin.
		Added Menu option to turn compression on/off
Dec16	Made the last panel mobile, such that it can be inserted into other pages.
*/

#define ALLOC_GLOBALS
#include <ansi_c.h>
#include <userint.h>
#include <cvirte.h>
#include "AnalogSettings.h"
#include "AnalogSettings2.h"
#include "DigitalSettings2.h"
#include "GUIDesign.h"
#include "GUIDesign2.h"
#include "main.h"
#include "Adwin.h"
#include <time.h>
#define VAR_DECLS 1
#include "vars.h"

int main(int argc, char *argv[])
{
	if (InitCVIRTE(0, argv, 0) == 0)
		return -1; /* out of memory */
	if ((panelHandle = LoadPanel(0, "GUIDesign.uir", PANEL)) < 0)
		return -1;
	if ((panelHandle_sub1 = LoadPanel(0, "GUIDesign.uir", SUBPANEL1)) < 0)
		return -1;
	if ((panelHandle_sub2 = LoadPanel(0, "GUIDesign.uir", SUBPANEL2)) < 0)
		return -1;

	if ((panelHandle2 = LoadPanel(0, "AnalogSettings.uir", PANEL)) < 0)
		return -1;
	if ((panelHandle3 = LoadPanel(0, "DigitalSettings.uir", PANEL)) < 0)
		return -1;
	if ((panelHandle4 = LoadPanel(0, "AnalogControl.uir", PANEL)) < 0)
		return -1;
	if ((panelHandle5 = LoadPanel(0, "DDSSettings.uir", PANEL)) < 0)
		return -1;
	if ((panelHandle6 = LoadPanel(0, "DDSControl.uir", PANEL)) < 0)
		return -1;
	if ((panelHandle7 = LoadPanel(0, "Scan.uir", PANEL)) < 0)
		return -1;
	if ((panelHandle8 = LoadPanel(0, "ScanTableLoader.uir", PANEL)) < 0)
		return -1;

	SetCtrlAttribute(panelHandle, PANEL_DEBUG, ATTR_VISIBLE, 0);
	// Initialize arrays (to avoid undefined elements causing -99 to be written)
	for (int j = 0; j <= NUMBERANALOGCHANNELS; j++)
	{
		//ramp over # of analog chanels
		AChName[j].tfcn = 1;
		AChName[j].tbias = 0;
		AChName[j].resettozero = 1;
		for (int i = 0; i <= NUMBEROFCOLUMNS; i++) // ramp over # of cells per page
		{
			for (int k = 0; k < NUMBEROFPAGES; k++) // ramp over pages
			{
				AnalogTable[i][j][k].fcn = 1;
				AnalogTable[i][j][k].fval = 0.0;
				AnalogTable[i][j][k].tscale = 1;
			}
		}
	}

	for (int j = 0; j <= NUMBERDIGITALCHANNELS; j++)
	{
		for (int i = 0; i <= NUMBEROFCOLUMNS; i++) // ramp over # of cells per page
		{
			for (int k = 0; k < NUMBEROFPAGES; k++) // ramp over pages
				DigTableValues[i][j][k] = 0;
		}
	}

	//initialize dds_tables, don't assume anything...
	for (int i = 0; i < NUMBEROFCOLUMNS; i++)
	{
		for (int j = 0; j < NUMBEROFPAGES; j++)
		{
			ddstable[i][j].start_frequency = 0.0;
			ddstable[i][j].end_frequency = 0.0;
			ddstable[i][j].amplitude = 0.0;
			ddstable[i][j].delta_time = 0.0;
			ddstable[i][j].is_stop = TRUE;

			dds2table[i][j].start_frequency = 0.0;
			dds2table[i][j].end_frequency = 0.0;
			dds2table[i][j].amplitude = 0.0;
			dds2table[i][j].delta_time = 0.0;
			dds2table[i][j].is_stop = TRUE;

			dds3table[i][j].start_frequency = 0.0;
			dds3table[i][j].end_frequency = 0.0;
			dds3table[i][j].amplitude = 0.0;
			dds3table[i][j].delta_time = 0.0;
			dds3table[i][j].is_stop = TRUE;
		}
	}

	// done initializing

	EventPeriod = DefaultEventPeriod;
	ClearListCtrl(panelHandle, PANEL_DEBUG);
	menuHandle = GetPanelMenuBar(panelHandle);

	SetMenuBarAttribute(menuHandle, MENU_UPDATEPERIOD_SETGD5, ATTR_CHECKED, 0);
	SetMenuBarAttribute(menuHandle, MENU_UPDATEPERIOD_SETGD10, ATTR_CHECKED, 1);
	SetMenuBarAttribute(menuHandle, MENU_UPDATEPERIOD_SETGD100, ATTR_CHECKED, 0);
	SetMenuBarAttribute(menuHandle, MENU_UPDATEPERIOD_SETGD1000, ATTR_CHECKED, 0);
	currentpage = 1;

	//LoadLastSettings(1); //This feature is not fully implemented

	//Sets the First Page as Active
	SetCtrlVal(panelHandle, PANEL_TB_SHOWPHASE1, 1);
	SetCtrlVal(panelHandle, PANEL_TB_SHOWPHASE2, 0);
	SetCtrlVal(panelHandle, PANEL_TB_SHOWPHASE3, 0);
	SetCtrlVal(panelHandle, PANEL_TB_SHOWPHASE4, 0);
	SetCtrlVal(panelHandle, PANEL_TB_SHOWPHASE5, 0);
	SetCtrlVal(panelHandle, PANEL_TB_SHOWPHASE6, 0);
	SetCtrlVal(panelHandle, PANEL_TB_SHOWPHASE7, 0);

	// autochange the size of the analog table on main panel
	//	DrawNewTable(0);

	Initialization();

	DisplayPanel(panelHandle);
	RunUserInterface();		   // start the GUI
	DiscardPanel(panelHandle); // returns here after the GUI shutsdown

	return 0;
}
//**********************************************************************************
void Initialization()
{
	//Changes:
	//Mar09, 2006:  Force DDS 1 frequency settings at loadtime.
	int i = 0;
	int x0, dx;
	PScan.Scan_Active = FALSE;
	PScan.Use_Scan_List = FALSE;
	//
	DDSFreq.extclock = 15.036;
	DDSFreq.PLLmult = 8;
	DDSFreq.clock = DDSFreq.extclock * (double)DDSFreq.PLLmult;
	//	SetCtrlAttribute (panelHandle, PANEL_LABNOTE_TXT, ATTR_VISIBLE, FALSE);

	//Add in any extra rows (if the number of channels increases)
	//July4, added another row for DDS2

	//Build display panels (text/channel num)
	SetTableColumnAttribute(panelHandle, PANEL_TBL_ANAMES, 1, ATTR_CELL_TYPE, VAL_CELL_STRING);
	SetTableColumnAttribute(panelHandle, PANEL_TBL_ANAMES, 2, ATTR_CELL_TYPE, VAL_CELL_NUMERIC);
	SetTableColumnAttribute(panelHandle, PANEL_TBL_ANAMES, 2, ATTR_DATA_TYPE, VAL_UNSIGNED_INTEGER);
	SetTableColumnAttribute(panelHandle, PANEL_TBL_DIGNAMES, 1, ATTR_CELL_TYPE, VAL_CELL_STRING);
	SetTableColumnAttribute(panelHandle, PANEL_TBL_DIGNAMES, 2, ATTR_CELL_TYPE, VAL_CELL_NUMERIC);
	SetTableColumnAttribute(panelHandle, PANEL_TBL_DIGNAMES, 2, ATTR_DATA_TYPE, VAL_UNSIGNED_INTEGER);
	SetCtrlAttribute(panelHandle, PANEL_TBL_ANAMES, ATTR_TABLE_MODE, VAL_COLUMN);
	SetCtrlAttribute(panelHandle, PANEL_TBL_DIGNAMES, ATTR_TABLE_MODE, VAL_COLUMN);

	// Build Digital Table
	NewCtrlMenuItem(panelHandle, PANEL_DIGTABLE, "Copy", -1, Dig_Cell_Copy, 0); //Adds Popup Menu Item "Copy"
	NewCtrlMenuItem(panelHandle, PANEL_DIGTABLE, "Paste", -1, Dig_Cell_Paste, 0);
	HideBuiltInCtrlMenuItem(panelHandle, PANEL_DIGTABLE, -4); //Hides Sort Command

	SetCtrlAttribute(panelHandle, PANEL_DIGTABLE, ATTR_TABLE_MODE, VAL_GRID);
	SetCtrlAttribute(panelHandle, PANEL_DIGTABLE, ATTR_ENABLE_COLUMN_SIZING, 0);
	SetCtrlAttribute(panelHandle, PANEL_DIGTABLE, ATTR_ENABLE_ROW_SIZING, 0);
	SetCtrlAttribute(panelHandle, PANEL_DIGTABLE, ATTR_HILITE_ONLY_WHEN_PANEL_ACTIVE, 1);
	InsertTableRows(panelHandle, PANEL_DIGTABLE, -1, NUMBERDIGITALCHANNELS - 1, VAL_USE_MASTER_CELL_TYPE);
	InsertTableRows(panelHandle, PANEL_TBL_DIGNAMES, -1, NUMBERDIGITALCHANNELS - 1, VAL_USE_MASTER_CELL_TYPE);

	for (i = 1; i <= NUMBERDIGITALCHANNELS; i++)
	{
		SetTableCellVal(panelHandle, PANEL_TBL_DIGNAMES, MakePoint(2, i), i);
	}

	//Build Analog Table
	NewCtrlMenuItem(panelHandle, PANEL_ANALOGTABLE, "Copy", -1, Analog_Cell_Copy, 0);
	NewCtrlMenuItem(panelHandle, PANEL_ANALOGTABLE, "Paste", -1, Analog_Cell_Paste, 0);
	HideBuiltInCtrlMenuItem(panelHandle, PANEL_ANALOGTABLE, -4); //Hides Sort Command

	SetCtrlAttribute(panelHandle, PANEL_ANALOGTABLE, ATTR_TABLE_MODE, VAL_GRID);
	SetCtrlAttribute(panelHandle, PANEL_ANALOGTABLE, ATTR_ENABLE_COLUMN_SIZING, 0);
	SetCtrlAttribute(panelHandle, PANEL_ANALOGTABLE, ATTR_ENABLE_ROW_SIZING, 0);
	SetTableColumnAttribute(panelHandle, PANEL_ANALOGTABLE, -1, ATTR_PRECISION, 3);

	InsertTableRows(panelHandle, PANEL_ANALOGTABLE, -1, NUMBERANALOGCHANNELS + NUMBERDDS - 1, VAL_CELL_NUMERIC);
	InsertTableRows(panelHandle, PANEL_TBL_ANAMES, -1, NUMBERANALOGCHANNELS + NUMBERDDS - 1, VAL_USE_MASTER_CELL_TYPE);
	InsertTableRows(panelHandle, PANEL_TBL_ANALOGUNITS, -1, NUMBERANALOGCHANNELS - 1, VAL_USE_MASTER_CELL_TYPE);
	SetAnalogChannels();

	//Scan Table RC Menu
	NewCtrlMenuItem(panelHandle, PANEL_SCAN_TABLE, "Load Values", -1, Scan_Table_Load, 0);
	HideBuiltInCtrlMenuItem(panelHandle, PANEL_SCAN_TABLE, -4);

	for (i = 1; i <= NUMBERANALOGCHANNELS; i++)
	{
		SetTableCellVal(panelHandle, PANEL_TBL_ANAMES, MakePoint(2, i), i);
	}

	SetTableRowAttribute(panelHandle, PANEL_ANALOGTABLE, -1, ATTR_PRECISION, 3);

	// Change Analog Settings window
	SetCtrlAttribute(panelHandle2, ANLGPANEL_NUM_ACH_LINE, ATTR_MAX_VALUE, NUMBERANALOGCHANNELS);
	SetCtrlAttribute(panelHandle2, ANLGPANEL_NUM_ACHANNEL, ATTR_MAX_VALUE, NUMBERANALOGCHANNELS);

	// change GUI
	SetCtrlAttribute(panelHandle, PANEL_ANALOGTABLE, ATTR_NUM_VISIBLE_ROWS, NUMBERANALOGCHANNELS + NUMBERDDS);
	SetCtrlAttribute(panelHandle, PANEL_DIGTABLE, ATTR_NUM_VISIBLE_ROWS, NUMBERDIGITALCHANNELS);

	// Generate labels
	for (int i = 0; i < NUMBEROFPAGES; i++)
	{
		int newTable = NewCtrl(panelHandle, CTRL_TABLE, "Desc.", 88, 165);
		SetCtrlAttribute(panelHandle, newTable, ATTR_HEIGHT, 25);
		SetCtrlAttribute(panelHandle, newTable, ATTR_WIDTH, 686);
		SetCtrlAttribute(panelHandle, newTable, ATTR_LABEL_TOP, 93);
		SetCtrlAttribute(panelHandle, newTable, ATTR_LABEL_LEFT, 130);
		SetCtrlAttribute(panelHandle, newTable, ATTR_TABLE_MODE, VAL_GRID);
		SetCtrlAttribute(panelHandle, newTable, ATTR_CELL_TYPE, VAL_CELL_STRING);
		SetCtrlAttribute(panelHandle, newTable, ATTR_COLUMN_LABELS_VISIBLE, 0);
		SetCtrlAttribute(panelHandle, newTable, ATTR_ROW_LABELS_VISIBLE, 0);
		SetCtrlAttribute(panelHandle, newTable, ATTR_SCROLL_BARS, VAL_NO_SCROLL_BARS);
		SetCtrlAttribute(panelHandle, newTable, ATTR_HORIZONTAL_GRID_COLOR, VAL_BLACK);
		SetCtrlAttribute(panelHandle, newTable, ATTR_VERTICAL_GRID_COLOR, VAL_BLACK);
		InsertTableRows(panelHandle, newTable, -1, 1, VAL_USE_MASTER_CELL_TYPE);
		InsertTableColumns(panelHandle, newTable, -1, 17, VAL_USE_MASTER_CELL_TYPE);
		SetTableColumnAttribute(panelHandle, newTable, -1, ATTR_COLUMN_WIDTH, 40);
		LabelArray[i] = newTable;
	}

	setVisibleLabel(1);

	// Reposition the page boxes and checkboxes
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE1, ATTR_TOP, 30);
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE2, ATTR_TOP, 30);
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE3, ATTR_TOP, 30);
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE4, ATTR_TOP, 30);
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE5, ATTR_TOP, 30);
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE6, ATTR_TOP, 30);
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE7, ATTR_TOP, 30);
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE8, ATTR_TOP, 30);
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE9, ATTR_TOP, 30);
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE10, ATTR_TOP, 30);
	x0 = 165;
	dx = 65;
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE1, ATTR_LEFT, x0);
	x0 = x0 + dx;
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE2, ATTR_LEFT, x0);
	x0 = x0 + dx;
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE3, ATTR_LEFT, x0);
	x0 = x0 + dx;
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE4, ATTR_LEFT, x0);
	x0 = x0 + dx;
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE5, ATTR_LEFT, x0);
	x0 = x0 + dx;
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE6, ATTR_LEFT, x0);
	x0 = x0 + dx;
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE7, ATTR_LEFT, x0);
	x0 = x0 + dx;
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE8, ATTR_LEFT, x0);
	x0 = x0 + dx;
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE9, ATTR_LEFT, x0);
	x0 = x0 + dx;
	SetCtrlAttribute(panelHandle, PANEL_TB_SHOWPHASE10, ATTR_LEFT, x0);
	x0 = x0 + dx;
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX, ATTR_TOP, 60);
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX_2, ATTR_TOP, 60);
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX_3, ATTR_TOP, 60);
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX_4, ATTR_TOP, 60);
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX_5, ATTR_TOP, 60);
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX_6, ATTR_TOP, 60);
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX_7, ATTR_TOP, 60);
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX_8, ATTR_TOP, 60);
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX_9, ATTR_TOP, 60);
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX_10, ATTR_TOP, 60);
	x0 = 165;
	dx = 65;
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX, ATTR_LEFT, x0);
	x0 = x0 + dx;
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX_2, ATTR_LEFT, x0);
	x0 = x0 + dx;
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX_3, ATTR_LEFT, x0);
	x0 = x0 + dx;
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX_4, ATTR_LEFT, x0);
	x0 = x0 + dx;
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX_5, ATTR_LEFT, x0);
	x0 = x0 + dx;
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX_6, ATTR_LEFT, x0);
	x0 = x0 + dx;
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX_7, ATTR_LEFT, x0);
	x0 = x0 + dx;
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX_8, ATTR_LEFT, x0);
	x0 = x0 + dx;
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX_9, ATTR_LEFT, x0);
	x0 = x0 + dx;
	SetCtrlAttribute(panelHandle, PANEL_CHECKBOX_10, ATTR_LEFT, x0);
	x0 = x0 + dx;

	SetCtrlAttribute(panelHandle_sub2, SUBPANEL2, ATTR_VISIBLE, 0);

	PScan.Analog.Scan_Step_Size = 1.0;
	PScan.Analog.Iterations_Per_Step = 1;
	PScan.Scan_Active = FALSE;
	//set to display both analog and digital channels
	SetChannelDisplayed(1);
	DrawCanvasArrows();
	//
	//set to graphical display
	//	SetDisplayType(VAL_CELL_NUMERIC);
	//	DrawNewTable(0);

	return;
}

void setVisibleLabel(int labelNum)
{
	for (int i = 0; i < NUMBEROFPAGES; i++)
	{
		SetCtrlAttribute(panelHandle, LabelArray[i], ATTR_VISIBLE, (i == labelNum) ? 1 : 0);
	}
}
//***************************************************************************************************
/*void ConvertIntToStr(int int_val, char *int_str)
{

	int i, j;

	for (i = j = floor(log10(int_val)); i >= 0; i--)
	{
		int_str[i] = (char)(((int)'0') + floor(((int)floor((int_val / pow(10, (j - i)))) % 10)));
	}

	int_str[j + 1] = '\0';

	return;
}
*/
//***************************************************************************************************
void DrawCanvasArrows(void)
{
	int loopx = 0, loopx0 = 0;
	// draws an arrow in each of 2 canvas's on the GUI.  These canvas's are moved around to indicate the location
	// of a loop in the Adwin output.
	SetCtrlAttribute(panelHandle, PANEL_CANVAS_START, ATTR_PEN_WIDTH, 1);
	SetCtrlAttribute(panelHandle, PANEL_CANVAS_START, ATTR_PEN_COLOR, VAL_DK_GREEN);
	SetCtrlAttribute(panelHandle, PANEL_CANVAS_START, ATTR_PICT_BGCOLOR, VAL_TRANSPARENT);

	CanvasDrawLine(panelHandle, PANEL_CANVAS_START, MakePoint(0, 0), MakePoint(15, 0));
	CanvasDrawLine(panelHandle, PANEL_CANVAS_START, MakePoint(15, 0), MakePoint(8, 12));
	CanvasDrawLine(panelHandle, PANEL_CANVAS_START, MakePoint(8, 12), MakePoint(0, 0));
	CanvasDrawLine(panelHandle, PANEL_CANVAS_START, MakePoint(8, 11), MakePoint(1, 1));
	CanvasDrawLine(panelHandle, PANEL_CANVAS_START, MakePoint(8, 10), MakePoint(2, 2));
	CanvasDrawLine(panelHandle, PANEL_CANVAS_START, MakePoint(0, 0), MakePoint(8, 4));
	SetCtrlAttribute(panelHandle, PANEL_CANVAS_START, ATTR_PEN_WIDTH, 4);
	CanvasDrawLine(panelHandle, PANEL_CANVAS_START, MakePoint(4, 2), MakePoint(11, 2));
	CanvasDrawLine(panelHandle, PANEL_CANVAS_START, MakePoint(11, 2), MakePoint(8, 8));
	CanvasDrawLine(panelHandle, PANEL_CANVAS_START, MakePoint(8, 8), MakePoint(4, 2));

	SetCtrlAttribute(panelHandle, PANEL_CANVAS_END, ATTR_PEN_WIDTH, 1);
	SetCtrlAttribute(panelHandle, PANEL_CANVAS_END, ATTR_PEN_COLOR, VAL_DK_RED);
	SetCtrlAttribute(panelHandle, PANEL_CANVAS_END, ATTR_PICT_BGCOLOR, VAL_TRANSPARENT);
	CanvasDrawLine(panelHandle, PANEL_CANVAS_END, MakePoint(0, 0), MakePoint(8, 4));

	CanvasDrawLine(panelHandle, PANEL_CANVAS_END, MakePoint(0, 0), MakePoint(15, 0));
	CanvasDrawLine(panelHandle, PANEL_CANVAS_END, MakePoint(15, 0), MakePoint(8, 12));
	CanvasDrawLine(panelHandle, PANEL_CANVAS_END, MakePoint(8, 12), MakePoint(0, 0));
	CanvasDrawLine(panelHandle, PANEL_CANVAS_END, MakePoint(8, 11), MakePoint(1, 1));
	CanvasDrawLine(panelHandle, PANEL_CANVAS_END, MakePoint(8, 10), MakePoint(2, 2));
	SetCtrlAttribute(panelHandle, PANEL_CANVAS_END, ATTR_PEN_WIDTH, 4);
	CanvasDrawLine(panelHandle, PANEL_CANVAS_END, MakePoint(4, 2), MakePoint(11, 2));
	CanvasDrawLine(panelHandle, PANEL_CANVAS_END, MakePoint(11, 2), MakePoint(8, 8));
	CanvasDrawLine(panelHandle, PANEL_CANVAS_END, MakePoint(8, 8), MakePoint(4, 2));

	loopx0 = 170; //horizontal offset
	loopx = 8;	  //x position...in horizontal table units
	SetCtrlAttribute(panelHandle, PANEL_CANVAS_START, ATTR_LEFT, loopx0 + loopx * 40);
	SetCtrlAttribute(panelHandle, PANEL_CANVAS_START, ATTR_TOP, 141);
	SetCtrlAttribute(panelHandle, PANEL_CANVAS_END, ATTR_LEFT, loopx0 + loopx * 40 + 25);
	SetCtrlAttribute(panelHandle, PANEL_CANVAS_END, ATTR_TOP, 141);

	SetCtrlAttribute(panelHandle, PANEL_CANVAS_LOOPLINE, ATTR_LEFT, 168);
	SetCtrlAttribute(panelHandle, PANEL_CANVAS_LOOPLINE, ATTR_TOP, 140);
	SetCtrlAttribute(panelHandle, PANEL_CANVAS_LOOPLINE, ATTR_WIDTH, 685);
}
