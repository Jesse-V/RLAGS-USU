Module General
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
    '*----------------------------------------------------------------------
    'MicroStrain, Inc.
    'VB 2005 Sample Code for 3DM-GX3-25
    'Demonstrates continuous mode for 0xCE Euler Angles
    'Uses Data Communication Protocol 1.0
    'Version 1.0.0
    '1 August 2009
    'Barry, developer
    'Notes:
    'Written with Visual Basic 2005 Service Pack 1
    'Built at 1024 X 768 (Highest 32-bit)
    'Uses native controls in standard IDE; no third party controls
    'Decimal equivalent of hex is used for all protocol commands
    'Non-packet protocol used
    'SerialPort1 set at 115200,n,8,1
    '*-----------------------------------------------------------------------
    'Add revisions here
    '*-----------------------------------------------------------------------
    'byte array
    Public arrBytes() As Byte
    'array upper limit
    Public UpperLimitOfArray As Double
    'data packet variables
    Public MyHeader As Double
    Public TimerTick As Double
    Public MyCheckSum As Double
    Public TheirCheckSum As Double
    Public MyRoll As Double
    Public MyPitch As Double
    Public MyYaw As Double
    'temporary array to send 4 bytes to bit converter; convert to float
    Public arrTemp(0 To 3) As Byte
    Public Function CalcTimerTick(ByVal byteStart As Integer) As Double
        'calc 32 bit unsigned integer
        CalcTimerTick = CDbl(CDbl(arrBytes(byteStart)) * 16777216) + _
                                    CDbl(CDbl(arrBytes(byteStart + 1)) * 65536) + _
                                    CDbl(CDbl(arrBytes(byteStart + 2)) * 256) + _
                                   CDbl(CDbl(arrBytes(byteStart + 3)))

        'scale and round
        CalcTimerTick = Math.Round(CalcTimerTick / 62500, 2)
    End Function
End Module
