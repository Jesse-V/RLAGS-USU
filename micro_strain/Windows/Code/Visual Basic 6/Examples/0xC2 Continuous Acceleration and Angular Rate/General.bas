Attribute VB_Name = "General"
'*----------------------------------------------------------------------
'* (c) 2009 Microstrain, Inc.
'*----------------------------------------------------------------------
'* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING
'* CUSTOMERS WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER
'* FOR THEM TO SAVE TIME. AS A RESULT, MICROSTRAIN SHALL NOT BE HELD LIABLE
'* FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY
'* CLAIMS ARISING FROM THE CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY
'* CUSTOMERS OF THE CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH
'* THEIR PRODUCTS.
'*-----------------------------------------------------------------------
'MicroStrain, Inc.
'VB6 Sample Code for 3DM-GX3-25
'Demonstrates continuous mode for 0xC2 Acceleration and Angular Rate
'Uses Data Communication Protocol 1.0
'Version 1.0.0
'1 August 2009
'Barry, developer
'Notes:
    'Written with Visual Basic 6.0 Service Pack 5
    'Built at 1024 x 768
    'Uses native controls in standard IDE; no third party controls
    'Decimal equivalent of hex is used for all protocol commands
    'MSComm1 set at 115200,n,8,1 and InputMode=1(binary)
'*-----------------------------------------------------------------------
'Add revisions here
'*-----------------------------------------------------------------------
'byte array
Public arrBytes() As Byte
'data packet variables
Public MyHeader As Double
Public TimerTick As Double
Public MyCheckSum As Double
Public TheirCheckSum As Double
Public AccelX As Double
Public AccelY As Double
Public AccelZ As Double
Public AngRateX As Double
Public AngRateY As Double
Public AngRateZ As Double
'declare API call for float function
Public Declare Sub CopyMemory Lib "kernel32" Alias "RtlMoveMemory" (Destination As Any, Source As Any, ByVal Length As Long)

Public Function FloatConversion(byteStart As Integer) As Single
'dim byte array
Dim bytArray(0 To 3) As Byte
'populate byte array
bytArray(3) = arrBytes(byteStart)
bytArray(2) = arrBytes(byteStart + 1)
bytArray(1) = arrBytes(byteStart + 2)
bytArray(0) = arrBytes(byteStart + 3)
'copy into the float
CopyMemory FloatConversion, bytArray(0), 4
End Function

Public Function CalcTimerTick(byteStart As Integer) As Double
'calc 32 bit unsigned integer
CalcTimerTick = CDbl(CDbl(arrBytes(byteStart)) * 16777216) + _
                            CDbl(CDbl(arrBytes(byteStart + 1)) * 65536) + _
                            CDbl(CDbl(arrBytes(byteStart + 2)) * 256) + _
                           CDbl(CDbl(arrBytes(byteStart + 3)))
    
'scale and round to 2 decimal points
CalcTimerTick = Round(CalcTimerTick / 62500, 2)
End Function



