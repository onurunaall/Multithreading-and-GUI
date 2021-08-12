#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                            1       /* callback function: panelCB */
#define  PANEL_iComPortOpen               2       /* control type: command, callback function: iComPortOpenPressedCallBack */
#define  PANEL_iSelectPort                3       /* control type: command, callback function: iSelectPortPressed */
#define  PANEL_BAKIM_DEGERLER             4       /* control type: command, callback function: bakimDegerlerCallBack */
#define  PANEL_SABIT_DEGERLER             5       /* control type: command, callback function: sabitDegerlerCallBack */
#define  PANEL_UDPSEND                    6       /* control type: command, callback function: senderUdpCallBack */
#define  PANEL_INTEGRATION_SET            7       /* control type: command, callback function: integrationSetCallBack */
#define  PANEL_INTEGRATION_GET            8       /* control type: command, callback function: integrationGetCallBack */
#define  PANEL_iComPortClose              9       /* control type: command, callback function: iComPortClosePressedCallBack */
#define  PANEL_QUITBUTTON                 10      /* control type: command, callback function: QuitCallback */
#define  PANEL_BaudRateShow               11      /* control type: numeric, callback function: (none) */
#define  PANEL_ComPortShow                12      /* control type: numeric, callback function: (none) */
#define  PANEL_timetime2                  13      /* control type: numeric, callback function: (none) */
#define  PANEL_timetime                   14      /* control type: numeric, callback function: (none) */
#define  PANEL_FLUSHINQ                   15      /* control type: command, callback function: FlushInCallBack */
#define  PANEL_FLUSHOUTQ                  16      /* control type: command, callback function: FlushOutCallBack */
#define  PANEL_OutputTextBox              17      /* control type: textBox, callback function: outputTextBoxCallBack */
#define  PANEL_RING                       18      /* control type: ring, callback function: (none) */
#define  PANEL_INTEGRATIONTIME            19      /* control type: numeric, callback function: (none) */
#define  PANEL_BaudRateList               20      /* control type: ring, callback function: (none) */
#define  PANEL_ComPortList                21      /* control type: ring, callback function: (none) */
#define  PANEL_REACHINGTIME               22      /* control type: numeric, callback function: (none) */
#define  PANEL_CURRENTTEMP                23      /* control type: numeric, callback function: (none) */
#define  PANEL_TARGETTEMP                 24      /* control type: numeric, callback function: (none) */
#define  PANEL_DECORATION_2               25      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION_4               26      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION_3               27      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION_5               28      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION_6               29      /* control type: deco, callback function: (none) */
#define  PANEL_IPADDRBOX                  30      /* control type: string, callback function: (none) */

     /* Callback Prototypes: */

int  CVICALLBACK bakimDegerlerCallBack(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FlushInCallBack(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FlushOutCallBack(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK iComPortClosePressedCallBack(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK iComPortOpenPressedCallBack(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK integrationGetCallBack(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK integrationSetCallBack(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK iSelectPortPressed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK outputTextBoxCallBack(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK panelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK QuitCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK sabitDegerlerCallBack(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK senderUdpCallBack(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endifs
