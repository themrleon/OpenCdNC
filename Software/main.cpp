/*The MIT License (MIT)

Copyright (c) 2014 Leonardo Ciocari

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <string> 

using namespace std;
HANDLE hPort;

//--------------------------------------------------------- 
BOOL WriteByte(BYTE bybyte)
{
    DWORD iBytesWritten=0;
    DWORD iBytesToRead = 1;
    if(WriteFile(hPort,(LPCVOID)
        &bybyte,iBytesToRead,&iBytesWritten,NULL)==0)
        return FALSE;
    else return TRUE;
}

//--------------------------------------------------------- 
BOOL WriteString(const void *instring, int length)
{
    int index;
    BYTE *inbyte = (BYTE *) instring;
    for(index = 0; index< length; ++index)
    {
        if (WriteByte(inbyte[index]) == FALSE)
            return FALSE;
    }
    return TRUE;
}

//--------------------------------------------------------- 
BOOL ReadByte(BYTE  &resp)
{
    BOOL bReturn = TRUE;
    BYTE rx;
    DWORD dwBytesTransferred=0;
 
    if (ReadFile (hPort, &rx, 1, &dwBytesTransferred, 0)> 0)
    {
        if (dwBytesTransferred == 1)
        {
            resp=rx;
            bReturn  = TRUE;
        }
        else bReturn = FALSE;
    }
    else    bReturn = FALSE;
    return bReturn;
}

//--------------------------------------------------------- 
void ClosePort()
{
    CloseHandle(hPort);
    return;
}

//---------------------------------------------------------
HANDLE ConfigureSerialPort(LPCSTR  lpszPortName)
{
    HANDLE hComm = NULL;
    DWORD dwError;
    DCB PortDCB;
    COMMTIMEOUTS CommTimeouts;
    // Open the serial port.
    hComm = CreateFile (lpszPortName, // Pointer to the name of the port
        GENERIC_READ | GENERIC_WRITE,
        // Access (read-write) mode
        0,              // Share mode
        NULL,           // Pointer to the security attribute
        OPEN_EXISTING,  // How to open the serial port
        0,              // Port attributes
        NULL);          // Handle to port with attribute
    // to copy
 
    // Initialize the DCBlength member.
    PortDCB.DCBlength = sizeof (DCB);
    // Get the default port setting information.
    GetCommState (hComm, &PortDCB);
    // Change the DCB structure settings.
    PortDCB.BaudRate = 9600;              // Current baud
    PortDCB.fBinary = TRUE;               // Binary mode; no EOF check
    PortDCB.fParity = TRUE;               // Enable parity checking
    PortDCB.fOutxCtsFlow = FALSE;         // No CTS output flow control
    PortDCB.fOutxDsrFlow = FALSE;         // No DSR output flow control
    PortDCB.fDtrControl = DTR_CONTROL_ENABLE;
    // DTR flow control type
    PortDCB.fDsrSensitivity = FALSE;      // DSR sensitivity
    PortDCB.fTXContinueOnXoff = TRUE;     // XOFF continues Tx
    PortDCB.fOutX = FALSE;                // No XON/XOFF out flow control
    PortDCB.fInX = FALSE;                 // No XON/XOFF in flow control
    PortDCB.fErrorChar = FALSE;           // Disable error replacement
    PortDCB.fNull = FALSE;                // Disable null stripping
    PortDCB.fRtsControl = RTS_CONTROL_ENABLE;
    // RTS flow control
    PortDCB.fAbortOnError = FALSE;        // Do not abort reads/writes on
    // error
    PortDCB.ByteSize = 8;                 // Number of bits/byte, 4-8
    PortDCB.Parity = NOPARITY;            // 0-4=no,odd,even,mark,space
    PortDCB.StopBits = ONESTOPBIT;        // 0,1,2 = 1, 1.5, 2
 
    // Configure the port according to the specifications of the DCB
    // structure.
    if (!SetCommState (hComm, &PortDCB))
    {
        printf("Could not configure serial port\n");
        return NULL;
    }
    // Retrieve the time-out parameters for all read and write operations
    // on the port.
    GetCommTimeouts (hComm, &CommTimeouts);
    // Change the COMMTIMEOUTS structure settings.
    CommTimeouts.ReadIntervalTimeout = MAXDWORD;
    CommTimeouts.ReadTotalTimeoutMultiplier = 0;
    CommTimeouts.ReadTotalTimeoutConstant = 0;
    CommTimeouts.WriteTotalTimeoutMultiplier = 0;
    CommTimeouts.WriteTotalTimeoutConstant = 0;
    if (!SetCommTimeouts (hComm, &CommTimeouts))
    {
        printf("Could not set timeouts\n");
        return NULL;
    }
    return hComm;
}

//---------------------------------------------------------
int main(void)
{   
    unsigned int x_offset,y_offset;
    char out[20]="",tmp[20]="";
    bool first_line=1;
    BYTE rx;
    
    ClosePort();
    hPort = ConfigureSerialPort("COM5");

    if(hPort == NULL)
     printf("Com port configuration failed\n");
    else
    {
     FILE *f = fopen("nelson","rb");
     char in[20];
     
     while(fgets(in,50,f)!=NULL)
     {
      if(in[0]=='X')
      {
       //Get X value
       printf("\n\nIN - %s",in); 
       memcpy( tmp, &in[1], 6 );
       tmp[6] = '\0';
       unsigned int x = atol(tmp);
       sprintf(tmp, "%5.0f", x*2.54);
       strcpy(out,"x");
       strcat(out,tmp);
       
       //Get Y value
       memcpy( tmp, &in[8], 6 );
       tmp[6] = '\0';
       unsigned int y = atol(tmp);
       sprintf(tmp, "%5.0f", y*2.54);
       strcat(out,"y");
       strcat(out,tmp);
       
       //Get Z value
       memcpy( tmp, &in[16], 1 );
       tmp[1] = '\0';
       strcat(out,"z");
       strcat(out,tmp);
       
       //Replace ' ' with '0'
       for(int a=0;a<strlen(out);a++)
       {
        if(out[a]==' ')
         out[a]='0';        
       }
       
       //Out to CNC
       printf("OUT=%s",out);
       WriteString(out,strlen(out));
       while(rx!='*')
        ReadByte(rx);
       rx=0;
      }
     }//while
     
     //Last command to CNC, reset position
     WriteString("x00000y00000z2",14);
     while(rx!='*')
      ReadByte(rx);
     rx=0;

     ClosePort();
    }
   
    printf("\n");
    system("pause");
    return 0;
}
