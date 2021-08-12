// Headers to be used
#include <udpsupp.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <formatio.h>
#include <rs232.h>
#include <ansi_c.h>
#include <utility.h>
#include <cvirte.h>     
#include <userint.h>
#include "DeviceControl.h"
#include "toolbox.h"
#include "panels.h"

// Constant Definitions
#define MAX_DATA_INPUT_LENGTH 300
#define MAX_IP_INPUT_LENGTH 15
#define READER_PORT 60100 //Reader port is selected arbitrarily from the Dynamic/Private port range which is 49152-65535

static int panelHandle;

// Lock Handle Variables    
static CmtThreadLockHandle lockHandleHotGet,lockHandleWarmGet,lockHandleColdGet,lockHandleTemp;
static CmtThreadLockHandle lockHandleHotSet,lockHandleWarmSet,lockHandleColdSet,lockHandleSabitDeg;
// These ones used to obtain whether CRC is valid or not
static int dummyCharMat24,dummyCharMat25;
static int bakimDegerleriCharMat17,bakimDegerleriCharMat18;
static int integrationType1CharMat12,integrationType1CharMat13;
static int integrationType2CharMat12,integrationType2CharMat13; 
static int integrationType3CharMat12,integrationType3CharMat13;
static int setIntegrationType1CharMat12,setIntegrationType1CharMat13;
static int setIntegrationType2CharMat12,setIntegrationType2CharMat13; 
static int setIntegrationType3CharMat12,setIntegrationType3CharMat13;

// Variables to be used for opening and closing port
static int iOpenedComPort,iOpenedBaudRate,iSelectedIntegrationType; 
static int iOpenComPortStatus=0,iSelectedComPort=0,iSelectedBaudRate=0;

// Thread on-off and counter variables
static int counter=0,sayac=0;
static int integrationVal=0;
static int sogutmaTime=0,currentTemperature=0,targettemperature=0;
static int runningGet1=0,runningGet2=0,runningGet3=0;
static int runningSet1=0,runningSet2=0,runningSet3=0;
static int runningReadReg=0;runningWriteReg=0;
static int running=1;
static int runningSabitDegerler=0,runningBakimDegerleri=0;
static unsigned int writerUDPChannel=0;
static unsigned int readerUDPChannel=0;
static int sizeUDP=0;
static unsigned int srcPort;
static unsigned char srcAddr;

// Variables to be used in determining the IP address
static unsigned char ipStr[15];
static unsigned int ipVal[4];

// Variables for table values
static int getHotTableTime,getWarmTableTime,getColdTableTime;
static int setHotTableTime,setWarmTableTime,setColdTableTime;

// Thread Function IDs
static int threadFuncID1=0,threadFuncID2=0,threadFuncID4=0,threadFuncID5=0,threadFuncID20=0;
static int threadFuncID7=0,threadFuncID8=0,threadFuncID9=0,threadFuncID10=0,threadFuncID11=0;
static int threadFuncID12=0,threadFuncID13=0,threadFuncID14=0,threadFuncID16=0,threadFuncID18=0;

//Thread functions' return values
static int threadFunctionStatus1,threadFunctionStatus2,threadFunctionStatus4,threadFunctionStatus5;
static int threadFunctionStatus7,threadFunctionStatus8,threadFunctionStatus9,threadFunctionStatus10;
static int threadFunctionStatus11,threadFunctionStatus12,threadFunctionStatus13,threadFunctionStatus14;
static int threadFunctionStatus16,threadFunctionStatus18,threadFunctionStatus20;

//Decimal to Hex converting array
static unsigned char hexDecNum[4];

// Arrays that store data to be sent
static unsigned char sendingData[9];
static unsigned char bakimDegerleriSendingData[19];
static unsigned char integrationType1SendingChar[10],integrationType2SendingChar[10],integrationType3SendingChar[10];
static unsigned char setIntegrationType1SendingChar[14],setIntegrationType2SendingChar[14],setIntegrationType3SendingChar[14];
static unsigned char receivedDataUDP[MAX_DATA_INPUT_LENGTH+1];

// Arrays that store incoming data
static unsigned char dummyChar[27];
static unsigned char bakimDegerleriChar[20];
static unsigned char integrationType1Char[15],integrationType2Char[15],integrationType3Char[15];
static unsigned char setIntegrationType1Char[14],setIntegrationType2Char[14],setIntegrationType3Char[14];
 
// Function prototypes
void substrateString(char ipStr[]);
void toStringConverter(char str[],int toBeConverted); 

/*MODIFIED
unsigned int CalculateCRC
*/

int CVICALLBACK UDPCallback (unsigned channel, int eventType, int errCode, void *callbackData); //UDP CallBack Func.
static int CVICALLBACK ThreadSabitDegerlerReceiver (void *functionData); //Sabit Degerler(version etc.) data sender thread prototype 
static int CVICALLBACK ThreadSabitDegerlerSender (void *functionData); //Sabit Degerler(version etc.) data receiver thread prototype 
static int CVICALLBACK ThreadOnPortTimer (void *functionData); // Time thread for when the port remains open prototype
static int CVICALLBACK ThreadOperationTimer (void *functionData); //Time thread for when program initialized prototype
static int CVICALLBACK ThreadBakimDegerleriSender (void *functionData); //Bakim Degerleridata sender thread prototype
static int CVICALLBACK ThreadBakimDegerleriReceiver (void *functionData); //Bakim Degerleridata receiver thread prototype
static int CVICALLBACK ThreadHotTableGetReceiver (void *functionData); //Hot Table GET Thread prototype 
static int CVICALLBACK ThreadHotTableGetReceiverAfterSetting (void *functionData); //Hot Table GET Thread prototype (after table value set something else) 
static int CVICALLBACK ThreadWarmTableGetReceiver (void *functionData); //Warm Table GET Thread prototype 
static int CVICALLBACK ThreadWarmTableGetReceiverAfterSetting (void *functionData); //Warm Table GET Thread prototype (after table value set something else) 
static int CVICALLBACK ThreadColdTableGetReceiver (void *functionData); //Cold Table GET Thread prototype 
static int CVICALLBACK ThreadColdTableGetReceiverAfterSetting (void *functionData); //Cold Table GET Thread prototype (after table value set something else)
static int CVICALLBACK ThreadHotTableSetReceiver (void *functionData); //Hot Table SET Thread prototype
static int CVICALLBACK ThreadWarmTableSetReceiver (void *functionData); //Warm Table SET Thread prototype 
static int CVICALLBACK ThreadColdTableSetReceiver (void *functionData); //Cold Table SET Thread prototype
int main (int argc, char *argv[])
{
    int error = 0;

	/* initialize and load resources */
    nullChk (InitCVIRTE (0, argv, 0));
    errChk (panelHandle = LoadPanel (0, "DeviceControl.uir", PANEL));
	
	// specifying end bits
	dummyChar[26]='\0';
	bakimDegerleriChar[19]='\0';
	integrationType1Char[14]='\0';
	integrationType2Char[14]='\0'; 
	integrationType3Char[14]='\0';

	// Operation timer thread setting
	errChk(CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE,ThreadOperationTimer,NULL,&threadFuncID5)); //Operation Timer starts counting when the program is run
	
	// Thread Locks 
	errChk(CmtNewLock("",OPT_TL_PROCESS_EVENTS_WHILE_WAITING,&lockHandleTemp));
	errChk(CmtNewLock("",OPT_TL_PROCESS_EVENTS_WHILE_WAITING,&lockHandleHotGet));
	errChk(CmtNewLock("",OPT_TL_PROCESS_EVENTS_WHILE_WAITING,&lockHandleWarmGet));
	errChk(CmtNewLock("",OPT_TL_PROCESS_EVENTS_WHILE_WAITING,&lockHandleColdGet));
	errChk(CmtNewLock("",OPT_TL_PROCESS_EVENTS_WHILE_WAITING,&lockHandleHotSet));
	errChk(CmtNewLock("",OPT_TL_PROCESS_EVENTS_WHILE_WAITING,&lockHandleWarmSet));
	errChk(CmtNewLock("",OPT_TL_PROCESS_EVENTS_WHILE_WAITING,&lockHandleColdSet));  
	errChk(CmtNewLock("",OPT_TL_PROCESS_EVENTS_WHILE_WAITING,&lockHandleSabitDeg));
	
	// UDP Write Channel Initializing
	errChk(CreateUDPChannel(UDP_ANY_LOCAL_PORT, &writerUDPChannel));
	
	//UDP Read Channel Initializing
	errChk(CreateUDPChannelConfig(READER_PORT,UDP_ANY_ADDRESS,0,UDPCallback,0,&readerUDPChannel)); 
	
	/* display the panel and run the user interface */
    errChk (DisplayPanel (panelHandle));
    errChk (RunUserInterface ());

Error:
    /* clean up */
	if (error < 0)
        MessagePopup("Error", GetGeneralErrorString(error));
	
    DiscardPanel (panelHandle);
    return 0;
}

/// HIFN Exit when the user dismisses the panel.
int CVICALLBACK panelCB (int panel, int event, void *callbackData,
        int eventData1, int eventData2)
{
    if (event == EVENT_CLOSE)
        QuitUserInterface (0);
    return 0;
}

/*CANCELLED
// Function to calculate CHECKSUM
unsigned int CalculateCRC...{

	return (unsigned int)crc;
}
*/

// Select Button Callback
// To open or close the port, selecting port number and baud rate, before attempting the open the port, is required. 
int CVICALLBACK iSelectPortPressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(iSelectedComPort==0){
			GetCtrlVal(PANEL,PANEL_ComPortList,&iSelectedComPort);
			SetCtrlVal(PANEL,PANEL_ComPortShow,iSelectedComPort);
			}
			else if(!(iSelectedComPort==0) && iSelectedBaudRate==0){
			GetCtrlVal(PANEL,PANEL_BaudRateList,&iSelectedBaudRate);
			SetCtrlVal(PANEL,PANEL_BaudRateShow,iSelectedBaudRate);
			}
			break;
	}
	return 0;
}

// COM Port Open Button CallBack Function
int CVICALLBACK iComPortOpenPressedCallBack (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int error=0;
	int iComPortOpenError=0;
	switch (event)
	{
		case EVENT_COMMIT:
			if(iSelectedComPort!=0 && iSelectedBaudRate!=0){
				errChk(iComPortOpenError=OpenComConfig(iSelectedComPort,"",iSelectedBaudRate,0,8,1,0,0));	
				if(iComPortOpenError>=0){
					//COM port opened successfully! 
					iOpenComPortStatus=1;
					SetCtrlAttribute(PANEL,PANEL_iComPortOpen,ATTR_DIMMED,1);
					SetCtrlAttribute(PANEL,PANEL_iComPortClose,ATTR_DIMMED,0);
					iOpenedBaudRate=iSelectedBaudRate;
					iOpenedComPort=iSelectedComPort;
					CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE,ThreadOnPortTimer,NULL,&threadFuncID4);
					SetCtrlVal(PANEL,PANEL_OutputTextBox,"COM Port has been opened succesfully!\n+-+");
					//Cutting UDP connection 
					DisposeUDPChannel(writerUDPChannel);
					DisposeUDPChannel(readerUDPChannel);
				}
				else{
					//COM port cannot be opened!
					SetCtrlVal(PANEL,PANEL_OutputTextBox,"COM Port Not Found!\n+-+"); 
					iOpenComPortStatus=0;
					iOpenedBaudRate=0;
					iOpenedComPort=0;
				}
				{
				Error:
					if (error < 0)
        			MessagePopup("Error", GetGeneralErrorString(error));
				}
				break;
			}
	}
	return 0;
}

// COM Port Close Button CallBack Function 
int CVICALLBACK iComPortClosePressedCallBack (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int iCloseComPortError=0;
	switch (event)
	{
		case EVENT_COMMIT:
			if(iOpenComPortStatus==1){
				iCloseComPortError=CloseCom(iOpenedComPort);
				if(iCloseComPortError>=0){
					//COM port closed succesfully
					SetCtrlAttribute(PANEL,PANEL_iComPortOpen,ATTR_DIMMED,0);
					SetCtrlAttribute(PANEL,PANEL_iComPortClose,ATTR_DIMMED,1);
					iOpenedComPort=0;
					iOpenedBaudRate=0;
					iOpenComPortStatus=0;
					SetCtrlVal(PANEL,PANEL_OutputTextBox,"COM Port has been closed succesfully!\n+-+");
					//Discarding all the locks
					CmtDiscardLock(lockHandleColdGet);
					CmtDiscardLock(lockHandleColdSet);
					CmtDiscardLock(lockHandleHotGet);
					CmtDiscardLock(lockHandleHotSet);
					CmtDiscardLock(lockHandleSabitDeg);
					CmtDiscardLock(lockHandleTemp);
					CmtDiscardLock(lockHandleWarmGet);
					CmtDiscardLock(lockHandleWarmSet);
					//Obtaining threads' return values to be used in exit command
					CmtGetThreadPoolFunctionAttribute(DEFAULT_THREAD_POOL_HANDLE,threadFuncID1,ATTR_TP_FUNCTION_EXECUTION_STATUS,&threadFunctionStatus1);
					CmtGetThreadPoolFunctionAttribute(DEFAULT_THREAD_POOL_HANDLE,threadFuncID2,ATTR_TP_FUNCTION_EXECUTION_STATUS,&threadFunctionStatus2);
					CmtGetThreadPoolFunctionAttribute(DEFAULT_THREAD_POOL_HANDLE,threadFuncID4,ATTR_TP_FUNCTION_EXECUTION_STATUS,&threadFunctionStatus4);
					CmtGetThreadPoolFunctionAttribute(DEFAULT_THREAD_POOL_HANDLE,threadFuncID5,ATTR_TP_FUNCTION_EXECUTION_STATUS,&threadFunctionStatus5);
					CmtGetThreadPoolFunctionAttribute(DEFAULT_THREAD_POOL_HANDLE,threadFuncID7,ATTR_TP_FUNCTION_EXECUTION_STATUS,&threadFunctionStatus7);
					CmtGetThreadPoolFunctionAttribute(DEFAULT_THREAD_POOL_HANDLE,threadFuncID8,ATTR_TP_FUNCTION_EXECUTION_STATUS,&threadFunctionStatus8);
					CmtGetThreadPoolFunctionAttribute(DEFAULT_THREAD_POOL_HANDLE,threadFuncID9,ATTR_TP_FUNCTION_EXECUTION_STATUS,&threadFunctionStatus9);
					CmtGetThreadPoolFunctionAttribute(DEFAULT_THREAD_POOL_HANDLE,threadFuncID10,ATTR_TP_FUNCTION_EXECUTION_STATUS,&threadFunctionStatus10);
					CmtGetThreadPoolFunctionAttribute(DEFAULT_THREAD_POOL_HANDLE,threadFuncID11,ATTR_TP_FUNCTION_EXECUTION_STATUS,&threadFunctionStatus11);
					CmtGetThreadPoolFunctionAttribute(DEFAULT_THREAD_POOL_HANDLE,threadFuncID12,ATTR_TP_FUNCTION_EXECUTION_STATUS,&threadFunctionStatus12);
					CmtGetThreadPoolFunctionAttribute(DEFAULT_THREAD_POOL_HANDLE,threadFuncID13,ATTR_TP_FUNCTION_EXECUTION_STATUS,&threadFunctionStatus13);
					CmtGetThreadPoolFunctionAttribute(DEFAULT_THREAD_POOL_HANDLE,threadFuncID14,ATTR_TP_FUNCTION_EXECUTION_STATUS,&threadFunctionStatus14);
					CmtGetThreadPoolFunctionAttribute(DEFAULT_THREAD_POOL_HANDLE,threadFuncID16,ATTR_TP_FUNCTION_EXECUTION_STATUS,&threadFunctionStatus16);
					CmtGetThreadPoolFunctionAttribute(DEFAULT_THREAD_POOL_HANDLE,threadFuncID18,ATTR_TP_FUNCTION_EXECUTION_STATUS,&threadFunctionStatus18);
					CmtGetThreadPoolFunctionAttribute(DEFAULT_THREAD_POOL_HANDLE,threadFuncID20,ATTR_TP_FUNCTION_EXECUTION_STATUS,&threadFunctionStatus20);
					//Waiting for thread to be completed so that they can be terminated without problem
					//Exiting all the thread functions except for opretion timer thread function  
					CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE,threadFuncID1,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
					CmtExitThreadPoolThread(threadFunctionStatus1);
					CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE,threadFuncID2,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
					CmtExitThreadPoolThread(threadFunctionStatus2);
					CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE,threadFuncID4,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
					CmtExitThreadPoolThread(threadFunctionStatus4);
					CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE,threadFuncID7,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
					CmtExitThreadPoolThread(threadFunctionStatus7);
					CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE,threadFuncID8,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
					CmtExitThreadPoolThread(threadFunctionStatus8);
					CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE,threadFuncID9,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
					CmtExitThreadPoolThread(threadFunctionStatus9);
					CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE,threadFuncID10,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
					CmtExitThreadPoolThread(threadFunctionStatus10);
					CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE,threadFuncID11,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
					CmtExitThreadPoolThread(threadFunctionStatus11); 
					CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE,threadFuncID12,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
					CmtExitThreadPoolThread(threadFunctionStatus12);
					CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE,threadFuncID13,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
					CmtExitThreadPoolThread(threadFunctionStatus13);
					CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE,threadFuncID14,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
					CmtExitThreadPoolThread(threadFunctionStatus14);
					CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE,threadFuncID16,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
					CmtExitThreadPoolThread(threadFunctionStatus16);
					CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE,threadFuncID18,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
					CmtExitThreadPoolThread(threadFunctionStatus18);
					CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE,threadFuncID20,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
					CmtExitThreadPoolThread(threadFunctionStatus20); 
				}
			}
			break;
	}
	return 0;
}
// Sabit Degerler CallBack Function			
int CVICALLBACK sabitDegerlerCallBack (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int error=0;
	switch (event)
	{
		case EVENT_COMMIT:
			ResetTextBox(PANEL,PANEL_OutputTextBox,"");
			//Thread control variables are updated to avoid interruption
			FlushOutQ(iOpenedComPort);
			FlushInQ(iOpenedComPort);
			runningSabitDegerler=1;
			runningBakimDegerleri=0;
			runningGet1=0;
			runningGet2=0;
			runningGet3=0;
			runningSet1=0;
			runningSet2=0;
			runningSet3=0;
			SetCtrlVal(PANEL,PANEL_OutputTextBox,"Sabit Degerler initializes...\n+-+");
			errChk(CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE,ThreadSabitDegerlerSender,NULL,&threadFuncID2));
			Delay(0.1);
			errChk(CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE,ThreadSabitDegerlerReceiver,NULL,&threadFuncID1));
			Error:
				if (error < 0)
        			MessagePopup("Error", GetGeneralErrorString(error));
			break;
	}
	return 0;		
}

// Sabit Degerler Komut Thread (Data Sender Thread)
static int CVICALLBACK ThreadSabitDegerlerSender (void *functionData){
	while(runningSabitDegerler){
		if(iOpenComPortStatus==1){
/*CANCELLED
*/
			//ComWrt(iOpenedComPort,sendingData,XXXX);
			Delay(0.4);
		}
	}
	return 0;
}

// Sabit Degerler Rapor Thread (Data Receiver Thread)
static int CVICALLBACK ThreadSabitDegerlerReceiver (void *functionData){
	//Bytes are received one by one till the verification completed succesfully
	while(runningSabitDegerler){
		if(iOpenComPortStatus==1){
			
/*CANCELLED
*/
		}
	}
	return 0;
}

// Bakim Degerleri CallBack Function
int CVICALLBACK bakimDegerlerCallBack (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			ResetTextBox(PANEL,PANEL_OutputTextBox,"");
			sogutmaTime=0;
			//Thread control variables are updated to avoid interruption 
			FlushOutQ(iOpenedComPort);
			FlushInQ(iOpenedComPort);
			sogutmaTime=0;
			runningBakimDegerleri=1;
			runningSabitDegerler=0;
			runningGet1=0;
			runningGet2=0;
			runningGet3=0;
			runningSet1=0;
			runningSet2=0;
			runningSet3=0;
			GetCtrlVal(PANEL,PANEL_TARGETTEMP,&targettemperature);
			SetCtrlVal(PANEL,PANEL_TARGETTEMP,targettemperature);
			SetCtrlVal(PANEL,PANEL_OutputTextBox,"Bakim Degerleri initializes...\n+-+");
			CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE,ThreadBakimDegerleriSender,NULL,&threadFuncID7);
			CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE,ThreadBakimDegerleriReceiver,NULL,&threadFuncID8);
			break;
	}
	return 0;
}

// Bakim Degerleri Komut Thread (Data Sender Thread)
static int CVICALLBACK ThreadBakimDegerleriSender (void *functionData){
	while(runningBakimDegerleri){
		if(iOpenComPortStatus==1){
/*CANCELLED
			//Sending the related bytes to receiveer board through COM port
			ComWrt(iOpenedComPort,bakimDegerleriSendingData,XXXX);
*/
			Delay(1);
		}
	}
	return 0;
}

// Bakim Degerleri Rapor Thread (Data Receiver Thread) 
static int CVICALLBACK ThreadBakimDegerleriReceiver (void *functionData){
	if(iOpenComPortStatus==1){
		//Bytes are received one by one till the verification completed succesfully 
		while(runningBakimDegerleri){
/*
CANCELLED
*/
		}
	}
	return 0;
}

// Table GET Button CallBack Function 
int CVICALLBACK integrationGetCallBack (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{																									
		case EVENT_COMMIT:
			ResetTextBox(PANEL,PANEL_OutputTextBox,"");
			//Thread control variables are updated to avoid interruption
			runningBakimDegerleri=0;
			runningSabitDegerler=0;
			runningGet1=1;
			runningGet2=1;
			runningGet3=1;
			runningSet1=0;
			runningSet2=0;
			runningSet3=0;
			/* 
			There is two option for this button. One of them is getting the table value before
			setting it to something else. And the other one is getting the table value after setting it.	  	
			*/
			if(iOpenComPortStatus==1){
				GetCtrlVal(PANEL,PANEL_RING,&iSelectedIntegrationType);
				if(iSelectedIntegrationType==1 && setPressedIntegrationType1==0){
					//Getting hot table
					runningGet1=1;
					FlushOutQ(iOpenedComPort);
/*CANCELLED
*/
					//Sending the related data the board through port
					//ComWrt(iOpenedComPort,integrationType1SendingChar,XXXX);
					//Initializing the HOT TABLE GET FUNC. THREAD 
					CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE,ThreadHotTableGetReceiver,NULL,&threadFuncID9);
				}															 
				else if(iSelectedIntegrationType==2 && setPressedIntegrationType2==0){
					//Getting warm table
					runningGet2=1;
					FlushOutQ(iOpenedComPort);
/*CANCELLED
*/
					//Sending the related data the board through port
					//ComWrt(iOpenedComPort,integrationType2SendingChar,XXXX);
					//Initializing the WARM TABLE GET FUNC. THREAD 
					CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE,ThreadWarmTableGetReceiver,NULL,&threadFuncID11);
				}
				else if(iSelectedIntegrationType==3 && setPressedIntegrationType3==0){
					//Getting cold table  
					runningGet3=1;
					FlushOutQ(iOpenedComPort);
/*CANCELLED
*/
					//Sending the related data the board through port 
					//ComWrt(iOpenedComPort,integrationType3SendingChar,XXXX);
					//Initializing the COLD TABLE GET FUNC. THREAD
					CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE,ThreadColdTableGetReceiver,NULL,&threadFuncID13);
				}
				else if(iSelectedIntegrationType==1 && setPressedIntegrationType1==1 ){
					//Getting hot table after setting is used
					runningGet1=1;
					FlushOutQ(iOpenedComPort);
/*CANCELLED
*/
					//Sending the related data the board through port 
					//ComWrt(iOpenedComPort,integrationType1SendingChar,XXXX);
					//Initializing the HOT TABLE GET FUNC.(After setting hot table) THREAD
					CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE,ThreadHotTableGetReceiverAfterSetting,NULL,&threadFuncID10); 
				}
				else if(iSelectedIntegrationType==2 && setPressedIntegrationType2==1){
					//Getting warm table after setting is used 
					runningGet2=1;
					FlushOutQ(iOpenedComPort);
/*CANCELLED
*/
					//Sending the related data the board through port  
					//ComWrt(iOpenedComPort,integrationType2SendingChar,XXXX);
					//Initializing the WARM TABLE GET FUNC.(After setting warm table) THREAD 
					CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE,ThreadWarmTableGetReceiverAfterSetting,NULL,&threadFuncID12);
				}
				else if(iSelectedIntegrationType==3 && setPressedIntegrationType3==1){
					//Getting cold table after setting is used 
					runningGet3=1;
					FlushOutQ(iOpenedComPort);
/*CANCELLED
*/
					//Sending the related data the board through port 
					//ComWrt(iOpenedComPort,integrationType3SendingChar,XXXX);
					//Initializing the COLD TABLE GET FUNC.(After setting cold table) THREAD
					CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE,ThreadColdTableGetReceiverAfterSetting,NULL,&threadFuncID14);
				}
			}
		break;
	}
	return 0;
}
// Hot Table GET Thread
static int CVICALLBACK ThreadHotTableGetReceiver (void *functionData){
	while(runningGet1){
		//Bytes are received one by one till the verification completed succesfully
/*CANCELLED
*/
	}
	return 0;
}

// Hot Table Get Function Thread(After Set Button Pressed At Least Once
static int CVICALLBACK ThreadHotTableGetReceiverAfterSetting (void *functionData){
	while(runningGet1){
		//Bytes are received one by one till the verification completed succesfully 
/*CANCELLED
*/
	}
	return 0;
}

// Warm Table GET Thread 
static int CVICALLBACK ThreadWarmTableGetReceiver (void *functionData){
		while(runningGet2){
			//Bytes are received one by one till the verification completed succesfully
/*CANCELLED
*/
		}
		return 0;
}

// Warm Table Get Function Thread(After Set Button Pressed At Least Once
static int CVICALLBACK ThreadWarmTableGetReceiverAfterSetting (void *functionData){
		while(runningGet2){
			//Bytes are received one by one till the verification completed succesfully 
/*CANCELLED
*/
		}
		return 0;
}

// Cold Table GET Thread
static int CVICALLBACK ThreadColdTableGetReceiver (void *functionData){
	while(runningGet3){
		//Bytes are received one by one till the verification completed succesfully 	
/*CANCELLED
*/
		}
		return 0;
}

// Cold Table Get Function Thread(After Set Button Pressed At Least Once
static int CVICALLBACK ThreadColdTableGetReceiverAfterSetting (void *functionData){
		while(runningGet3){
			//Bytes are received one by one till the verification completed succesfully
/*CANCELLED
*/
		}
		return 0;
}

// SET Button CallBack Function
int CVICALLBACK integrationSetCallBack (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{ 
	switch (event)
	{
		case EVENT_COMMIT:
			ResetTextBox(PANEL,PANEL_OutputTextBox,"");
			//Thread control variables are updated to avoid interruption  
			runningGet1=0;
			runningGet2=0;
			runningGet3=0;
			runningBakimDegerleri=0;
			runningSabitDegerler=0;
			if(iOpenComPortStatus==1){
				GetCtrlVal(PANEL,PANEL_RING,&iSelectedIntegrationType);
				GetCtrlVal(PANEL,PANEL_INTEGRATIONTIME,&integrationVal);
				if(iSelectedIntegrationType==1){
					//Setting Warm Table
					runningSet1=1;
					setPressedIntegrationType1=1;
					/*
					hexDecNum stands for the array that stores the data of the value that wants be set
					to related table. Incoming integer converts into DESIRED TYPE to be processed.
					*/
/*CANCELLED
*/
					//Sending the related data the board through port
					//ComWrt(iOpenedComPort,setIntegrationType1SendingChar,XXXX);
					//Initializing the WARM TABLE SET FUNC. THREAD
					CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE,ThreadHotTableSetReceiver,NULL,&threadFuncID18);
				}
				else if(iSelectedIntegrationType==2){
					//Setting Hot Table
					runningSet2=1;
					setPressedIntegrationType2=1;
					/*
					hexDecNum stands for the array that stores the data of the value that wants be set
					to related table. Incoming integer converts into three DESIRED TYPE to be processed.
					*/
/*CANCELLED
*/
					//Sending the related data the board through port 
					//ComWrt(iOpenedComPort,setIntegrationType2SendingChar,XX);
					//Initializing the HOT TABLE SET FUNC. THREAD 
					CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE,ThreadWarmTableSetReceiver,NULL,&threadFuncID16);
				}
				else if(iSelectedIntegrationType==3){
					//Setting Cold Table
					runningSet3=1;
					setPressedIntegrationType3=1;
					/*
					hexDecNum stands for the array that stores the data of the value that wants be set
					to related table. Incoming integer converts into DESIRED TYPE to be processed.
					*/
	/*CANCELLED
*/
					//Sending the related data the board through port
					//ComWrt(iOpenedComPort,setIntegrationType3SendingChar,XXXX);
					//Initializing the COLD TABLE SET FUNC. THREAD
					CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE,ThreadColdTableSetReceiver,NULL,&threadFuncID20);
				}
			}
			break;
	}
	return 0;
}

// Hot Table Set Thread
static int CVICALLBACK ThreadHotTableSetReceiver (void *functionData){
	if(iOpenComPortStatus==1){
		while(runningSet1){
			//Bytes are received one by one till the verification completed succesfully 
/*CANCELLED
*/
		}
	}
	return 0;
}

// Warm Table SET Table
static int CVICALLBACK ThreadWarmTableSetReceiver (void *functionData){
	if(iOpenComPortStatus==1){
		while(runningSet2){
			//Bytes are received one by one till the verification completed succesfully 
/*CANCELLED
*/
		}
	}
	return 0;
}

// Cold Table SET Thread
static int CVICALLBACK ThreadColdTableSetReceiver (void *functionData){
	if(iOpenComPortStatus==1){
		while(runningSet3){
			//Bytes are received one by one till the verification completed succesfully
/*CANCELLED
*/
		}
	}
	return 0;
}



// ON-Port Operation Time Thread
static int CVICALLBACK ThreadOnPortTimer (void *functionData){
	if(iOpenComPortStatus==1){
		while(running){
			counter+=1;
			Delay(1);
			SetCtrlVal(PANEL,PANEL_timetime,counter);
			if(iOpenComPortStatus==0){
				break;
			}
		}
	}
	return 0;
} 

// Whole Operation Time Thread
static int CVICALLBACK ThreadOperationTimer (void *functionData){
	while(running){	
		sayac+=1;
		Delay(1);
		SetCtrlVal(PANEL,PANEL_timetime2,sayac);
		if(QuitUserInterface==0){
			CmtReleaseThreadPoolFunctionID(DEFAULT_THREAD_POOL_HANDLE,threadFuncID5);
			break;
		}
	}
	return 0;
}

int CVICALLBACK QuitCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			CmtReleaseThreadPoolFunctionID(DEFAULT_THREAD_POOL_HANDLE,threadFuncID5);
			running=0;
			QuitUserInterface(0); 
			break;
	}
	return 0;
}

int CVICALLBACK FlushInCallBack (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(iOpenComPortStatus==1){
			FlushInQ(iOpenedComPort);
			}
			break;
	}
	return 0;
}
int CVICALLBACK FlushOutCallBack (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(iOpenComPortStatus==1){
			FlushOutQ(iOpenedComPort);
			}
			break;
	}
	return 0;
}

int CVICALLBACK outputTextBoxCallBack (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:
			ResetTextBox(PANEL,PANEL_OutputTextBox,"");
			break;
	}
	return 0;
}

const char* str1;  
// Function that determines the IP address components in int format
void substrateString(char ipStr[]){
	
	//char delimiter=".";
	char i=0;
//	while(str1!=NULL){
	//	str1=strtok(ipStr,delimiter);
	//	ipVal[i]=atoi(ipStr);
	//	toStringConverter(str1,ipVal[i]);
		//i++;
//	}
}

void toStringConverter(char str[],int toBeConverted){
	 int length=0;
	 while(toBeConverted!=0){
	 	length++;
		toBeConverted/=10;
	 }
	 
	 for(int i=0;i<length;i++){
		toBeConverted/=10;
		str[length-(i+1)]=(toBeConverted%10)+'0';
	 }
	 str[length]='\0';
}

int CVICALLBACK senderUdpCallBack (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int error=0;
	char dataUDP[MAX_DATA_INPUT_LENGTH+1];
	unsigned char destAddrUDP[MAX_IP_INPUT_LENGTH+1];
	unsigned char sendingdestAddrUDP[4];
	switch (event)
	{
		case EVENT_COMMIT:
			runningSabitDegerler=0;
			runningBakimDegerleri=0;
			runningGet1=0;
			runningGet2=0;
			runningGet3=0;
			runningSet1=0;
			runningSet2=0;
			runningSet3=0;
			errChk(GetCtrlVal(PANEL,PANEL_IPADDRBOX,destAddrUDP));
			errChk(GetCtrlVal(PANEL,PANEL_OutputTextBox,dataUDP));
			/*
			substrateString(destAddrUDP);
			sendingdestAddrUDP[0]=(char)ipVal[0];
			sendingdestAddrUDP[1]=(char)ipVal[1];
			sendingdestAddrUDP[2]=(char)ipVal[2];
			sendingdestAddrUDP[3]=(char)ipVal[3];
			*/
			
			//Write the message to a predetermined port number on which all readers will listening.    
			errChk(UDPWrite(writerUDPChannel,READER_PORT,destAddrUDP,dataUDP,strlen(dataUDP)+1));
			ResetTextBox(PANEL,PANEL_OutputTextBox,"");
			
			{
			Error:
    		if (error < 0)
       			MessagePopup("Error", GetGeneralErrorString(error));
			}
			break;
	}
	return 0;
}

int CVICALLBACK UDPCallback (unsigned channel, int eventType, int errCode, void *callbackData)
{
	int error=0;
	
	char srcAddr[16];
	if(eventType==UDP_DATAREADY){
		 //Pass NULL as the input buffer to determine the size of the arrived data.
        errChk(sizeUDP=UDPRead(channel,0,0,UDP_DO_NOT_WAIT,0,0));
			
		//Read the waiting message into the allocated buffer.
        errChk(UDPRead(channel, receivedDataUDP, sizeUDP, UDP_DO_NOT_WAIT, &srcPort, srcAddr));
			
		//Display the message, preceded by the sender's IP address and port number.
        sprintf(receivedDataUDP, "[%s:%d]\n", srcAddr, srcPort);
        SetCtrlVal(PANEL, PANEL_OutputTextBox, receivedDataUDP);
	        
		{
		Error:                                                             
	    	if (error<0)
	        	MessagePopup("Error", GetGeneralErrorString(error));
		}
	}
	return 0;
}


static int CVICALLBACK ThreadReadReg (void *functionData){
	if(iOpenComPortStatus==1){
		while(runningReadReg){
/*CANCELLED
*/
									//if(){
										SetCtrlVal(PANEL,PANEL_OutputTextBox,"CRC results for Register has been checked!\n+-+");
										/*CANCELLED
										readRegField=...*/
										SetCtrlVal(PANEL,PANEL_REGFIELD,readRegField);
										CmtReleaseLock(lockHandleReadReg);
										FlushInQ(iOpenedComPort);
										break;
									}
									else
										continue;
								}	
								else
									continue;
								}
							else
								continue;
							}
						else
							continue;
						}
					else
						continue;
					}
				else
					continue;
				}
			else
				continue;
			}
	return 0;
}
	

static int CVICALLBACK ThreadWriteReg (void *functionData){
	if(iOpenComPortStatus==1){
		while(runningWriteReg){
/*CANCELLED
*/
			}
		else
			continue;
		}
	}
	return 0;
}

int CVICALLBACK registerReadCallBack (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(PANEL,PANEL_REGNO,&registerNo);
/*CANCELLED
*/
			//ComWrt(iOpenedComPort,registerSendingChar,XXXX);
			CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE,ThreadReadReg,NULL,&threadFuncIDReg);(
			break;
	}
	return 0;
}
int CVICALLBACK registerWriteCallBack (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(PANEL,PANEL_REGNO,&registerNo);
/*CANCELLED
*/
			//ComWrt(iOpenedComPort,registerSendingChar,XX);
			CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE,ThreadWriteReg,NULL,&threadFuncIDReg);
			break;
	}
	return 0;
}
