/**
@file DigitalSettings.c
Configure the digital lines.
For each line we configure:
- chnum : channel number (1-32 on first card, 101-132 on second card)
- chname : name of the channel.
- resettolow :  Not implemented
*/
#include "DigitalSettings.h"

#include <ansi_c.h>
#include <userint.h>

#include "DigitalSettings2.h"
#include "GUIDesign.h"
#include "vars.h"

/**
Numeric control to update information displayed on the digital settings panel,
based on the current line
*/
int CVICALLBACK NUM_DIG_LINE_CALLBACK(int panel, int control, int event,
    void* callbackData, int eventData1,
    int eventData2)
{
    int line = 0;
    switch (event) {
    case EVENT_COMMIT:
        GetCtrlVal(panelHandle3, DIGPANEL_NUM_DIGCH_LINE, &line);
        SetCtrlVal(panelHandle3, DIGPANEL_NUM_DIGCHANNEL, DChName[line].chnum);
        SetCtrlVal(panelHandle3, DIGPANEL_STR_DIGCHANNELNAME,
            DChName[line].chname);
        SetCtrlVal(panelHandle3, DIGPANEL_CHK_DIGRESET, DChName[line].resettolow);
        break;
    }
    return 0;
}

/**
Button to allow changes to be made to textboxes.
Changes textbox from dimmed (indicator) to activate (hot) mode
*/
int CVICALLBACK CMD_DIGALLOWCHANGE_CALLBACK(int panel, int control, int event,
    void* callbackData, int eventData1,
    int eventData2)
{
    SetCtrlAttribute(panelHandle3, DIGPANEL_CMD_DIGSETCHANGES, ATTR_VISIBLE, 1);
    switch (event) {
    case EVENT_COMMIT:

        SetCtrlAttribute(panelHandle3, DIGPANEL_NUM_DIGCHANNEL, ATTR_CTRL_MODE,
            VAL_HOT);
        SetCtrlAttribute(panelHandle3, DIGPANEL_STR_DIGCHANNELNAME,
            ATTR_CTRL_MODE, VAL_HOT);
        SetCtrlAttribute(panelHandle3, DIGPANEL_CHK_DIGRESET, ATTR_CTRL_MODE,
            VAL_HOT);
        break;
    }
    return 0;
}

/**
Set the digital line information DChName, based on information in text boxes.
*/
int CVICALLBACK CMD_DIGSETCHANGES_CALLBACK(int panel, int control, int event,
    void* callbackData, int eventData1,
    int eventData2)
{
    int channel = 0, line = 0, resetlow = 0;
    char buff[50];
    switch (event) {
    case EVENT_COMMIT:
        GetCtrlVal(panelHandle3, DIGPANEL_NUM_DIGCHANNEL, &channel);
        GetCtrlVal(panelHandle3, DIGPANEL_NUM_DIGCH_LINE, &line);
        GetCtrlVal(panelHandle3, DIGPANEL_STR_DIGCHANNELNAME, buff);
        GetCtrlVal(panelHandle3, DIGPANEL_CHK_DIGRESET, &resetlow);
        DChName[line].chnum = channel;
        sprintf(DChName[line].chname, buff);
        DChName[line].resettolow = resetlow;
        SetDigitalChannels();
        break;
    }
    return 0;
}

/**
Turn control boxes into inactive (indicator) mode and hide the panel
*/
int CVICALLBACK CMD_DONEDIG_CALLBACK(int panel, int control, int event,
    void* callbackData, int eventData1,
    int eventData2)
{
    switch (event) {
    case EVENT_COMMIT:
        SetCtrlAttribute(panelHandle3, DIGPANEL_NUM_DIGCHANNEL, ATTR_CTRL_MODE,
            VAL_INDICATOR);
        SetCtrlAttribute(panelHandle3, DIGPANEL_STR_DIGCHANNELNAME,
            ATTR_CTRL_MODE, VAL_INDICATOR);
        SetCtrlAttribute(panelHandle3, DIGPANEL_CMD_DIGSETCHANGES, ATTR_VISIBLE,
            0);
        SetCtrlAttribute(panelHandle3, DIGPANEL_CHK_DIGRESET, ATTR_CTRL_MODE,
            VAL_INDICATOR);
        HidePanel(panelHandle3);
        break;
    }
    return 0;
}

/**
Set the digital channel list (on main panel) with the information contained in
the DChName array
*/
void SetDigitalChannels(void)
{
    for (int i = 1; i <= NUMBERDIGITALCHANNELS; i++) {
        SetTableCellAttribute(panelHandle, PANEL_TBL_DIGNAMES, MakePoint(1, i),
            ATTR_CTRL_VAL, DChName[i].chname);
        SetTableCellAttribute(panelHandle, PANEL_TBL_DIGNAMES, MakePoint(2, i),
            ATTR_CTRL_VAL, DChName[i].chnum);
    }
}
