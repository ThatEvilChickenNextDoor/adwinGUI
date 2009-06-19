#include <userint.h>

#include "AnalogSettings.h"
#include "AnalogSettings2.h"
/*  Sets the properties for each analog line.
    Properties include: 
    	chnum  :  Hardware channel... i.e. 1-32
    	tbias  :  voltage offset from zero.
    	tfcn   :  proportionality constant between the onscreen value and the output from the card.
		chname :  String to identify the channel on screen
		units  :  Another string, appears on rhs.  Can be used as units, transfer function, notes...
		minvolts: Minimum allowed voltage generated by card.    To protect instruments
		maxvolts :Maximum allowed voltage generated by card.  
		resettozero:  if true, forces the line to go low at end of cycle

*/
//************************************************************************
int CVICALLBACK CMD_ALLOWCHANGE_CALLBACK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
/*
Undim textboxes and numboxes so changes can be made to them.
Changes them from indicator to 'HOT' value
*/
{
	SetCtrlAttribute (panelHandle2, ANLGPANEL_CMD_SETCHANGES,   ATTR_VISIBLE, 1);

	switch (event)
		{
		case EVENT_COMMIT:
			
			SetCtrlAttribute (panelHandle2, ANLGPANEL_NUM_ACHANNEL, ATTR_CTRL_MODE, VAL_HOT);
			SetCtrlAttribute (panelHandle2, ANLGPANEL_STR_CHANNELNAME, ATTR_CTRL_MODE, VAL_HOT);
			SetCtrlAttribute (panelHandle2, ANLGPANEL_STRING_UNITS, ATTR_CTRL_MODE, VAL_HOT);
			SetCtrlAttribute (panelHandle2, ANLGPANEL_NUM_CHANNELPROP, ATTR_CTRL_MODE, VAL_HOT);
			SetCtrlAttribute (panelHandle2, ANLGPANEL_NUM_CHANNELBIAS, ATTR_CTRL_MODE, VAL_HOT);
			SetCtrlAttribute (panelHandle2, ANLGPANEL_CHKBOX_RESET, ATTR_CTRL_MODE, VAL_HOT);
			SetCtrlAttribute (panelHandle2, ANLGPANEL_NUM_MINV, ATTR_CTRL_MODE, VAL_HOT);
			SetCtrlAttribute (panelHandle2, ANLGPANEL_NUM_MAXV, ATTR_CTRL_MODE, VAL_HOT);
			ChangedVals=1;
			break;
		}
	return 0;
}
//************************************************************************

int CVICALLBACK CMD_SETCHANGES_CALLBACK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
/*
Change information in the AnalogChannel Properties based on the text and number boxes.
*/
{   
	char buff[51]="",buff2[51]="";
	int channel=0,line=0,val=0;
	double prop=0,bias=0,vmin=0,vmax=0;
	switch (event)
		{
		case EVENT_COMMIT:
		
			GetCtrlVal (panelHandle2, ANLGPANEL_NUM_ACHANNEL, &channel);
			GetCtrlVal (panelHandle2, ANLGPANEL_NUM_ACH_LINE, &line);
			GetCtrlVal (panelHandle2, ANLGPANEL_NUM_CHANNELPROP, &prop);
			GetCtrlVal (panelHandle2, ANLGPANEL_NUM_CHANNELBIAS, &bias);
			GetCtrlVal (panelHandle2, ANLGPANEL_STR_CHANNELNAME, buff);
			GetCtrlVal (panelHandle2, ANLGPANEL_STRING_UNITS, buff2);
			GetCtrlVal (panelHandle2, ANLGPANEL_CHKBOX_RESET, &val);
			GetCtrlVal (panelHandle2, ANLGPANEL_NUM_MINV, &vmin);
			GetCtrlVal (panelHandle2, ANLGPANEL_NUM_MAXV, &vmax);
			AChName[line].resettozero=val;
		
			AChName[line].chnum=channel;
			AChName[line].tfcn=prop;
			AChName[line].tbias=bias;
			AChName[line].maxvolts=vmax;
			AChName[line].minvolts=vmin;
			sprintf(AChName[line].chname,buff);
			sprintf(AChName[line].units,buff2);
			SetAnalogChannels();
		//	HidePanel(panelHandle2);

			//Next we will update the channel list and redisplay ;)
			break;
		}
	return 0;
}
//************************************************************************

int CVICALLBACK CMD_DONEANALOG_CALLBACK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
//  	Set all controls to indicator and hide the panel.
{
	switch (event)
		{
		case EVENT_COMMIT:
			SetCtrlAttribute (panelHandle2, ANLGPANEL_NUM_ACHANNEL, ATTR_CTRL_MODE, VAL_INDICATOR);
			SetCtrlAttribute (panelHandle2, ANLGPANEL_STR_CHANNELNAME, ATTR_CTRL_MODE, VAL_INDICATOR);
			SetCtrlAttribute (panelHandle2, ANLGPANEL_STRING_UNITS, ATTR_CTRL_MODE, VAL_INDICATOR);
			SetCtrlAttribute (panelHandle2, ANLGPANEL_NUM_CHANNELPROP, ATTR_CTRL_MODE, VAL_INDICATOR);
			SetCtrlAttribute (panelHandle2, ANLGPANEL_NUM_CHANNELBIAS, ATTR_CTRL_MODE, VAL_INDICATOR);	/* added by Seth, Oct 28, 2004 */
			SetCtrlAttribute (panelHandle2, ANLGPANEL_CHKBOX_RESET, ATTR_CTRL_MODE, VAL_INDICATOR); 
			SetCtrlAttribute (panelHandle2, ANLGPANEL_NUM_MINV, ATTR_CTRL_MODE, VAL_INDICATOR);
			SetCtrlAttribute (panelHandle2, ANLGPANEL_NUM_MAXV, ATTR_CTRL_MODE, VAL_INDICATOR); 
			
			SetCtrlAttribute (panelHandle2, ANLGPANEL_CMD_SETCHANGES,   ATTR_VISIBLE, 0);
			HidePanel(panelHandle2);
			break;
		}
	return 0;
}
//************************************************************************
// Update the main panel to display new values in the channel listing
void SetAnalogChannels()
{
	int i=0,j=0,k=0,line=0;
	char numbuff[20]="";
	for(i=1;i<=NUMBERANALOGCHANNELS;i++)
	{

		SetTableCellVal (panelHandle, PANEL_TBL_ANAMES, MakePoint(1,i), AChName[i].chname);
		SetTableCellVal (panelHandle, PANEL_TBL_ANAMES, MakePoint(2,i),AChName[i].chnum);
		SetTableCellVal (panelHandle, PANEL_TBL_ANALOGUNITS, MakePoint(1,i), AChName[i].units);
	}
	for (i=1;i<=NUMBERDDS;i++)
	{
		sprintf(AChName[i+NUMBERANALOGCHANNELS].chname,"DDS%d",i);
		SetTableCellVal (panelHandle, PANEL_TBL_ANAMES, MakePoint(1,i+NUMBERANALOGCHANNELS), AChName[i+NUMBERANALOGCHANNELS].chname);
	}
}

//********************************************************************
int CVICALLBACK NUM_ACH_LINE_CALLBACK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
// Changes the displayed information as we change the analog line that we look at through
// the ring control.
{   
	int line=0;
	switch (event)
		{
		case EVENT_COMMIT:
			
			GetCtrlVal (panelHandle2, ANLGPANEL_NUM_ACH_LINE, &line);
			
			SetCtrlVal (panelHandle2, ANLGPANEL_NUM_ACHANNEL, AChName[line].chnum);
			SetCtrlVal (panelHandle2, ANLGPANEL_NUM_CHANNELPROP,AChName[line].tfcn);
			SetCtrlVal (panelHandle2, ANLGPANEL_NUM_CHANNELBIAS,AChName[line].tbias);	/* added by Seth, Oct 28, 2004 */
			SetCtrlVal (panelHandle2, ANLGPANEL_STR_CHANNELNAME, AChName[line].chname);
			SetCtrlVal (panelHandle2, ANLGPANEL_STRING_UNITS, AChName[line].units);
			SetCtrlVal (panelHandle2, ANLGPANEL_CHKBOX_RESET, AChName[line].resettozero);
			SetCtrlVal (panelHandle2, ANLGPANEL_NUM_MINV, AChName[line].minvolts);
			SetCtrlVal (panelHandle2, ANLGPANEL_NUM_MAXV, AChName[line].maxvolts);
			break;
		}
																   
	return 0;
}
//************************************************************************************************
int CVICALLBACK CHKBOX_RESET_CALLBACK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int val=0,line=0;
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle2, ANLGPANEL_NUM_ACH_LINE, &line);
			GetCtrlVal (panelHandle2, ANLGPANEL_CHKBOX_RESET, &val);
			AChName[line].resettozero=val;
			break;
		}
	return 0;
}
