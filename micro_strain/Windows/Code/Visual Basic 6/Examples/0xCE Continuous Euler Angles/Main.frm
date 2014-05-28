VERSION 5.00
Object = "{648A5603-2C6E-101B-82B6-000000000014}#1.1#0"; "mscomm32.ocx"
Begin VB.Form Main 
   Caption         =   "Main"
   ClientHeight    =   3090
   ClientLeft      =   60
   ClientTop       =   750
   ClientWidth     =   4680
   ControlBox      =   0   'False
   LinkTopic       =   "Form1"
   MDIChild        =   -1  'True
   ScaleHeight     =   3090
   ScaleWidth      =   4680
   WindowState     =   2  'Maximized
   Begin VB.Frame fraCS 
      Caption         =   "CheckSum Indicator"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1095
      Left            =   2760
      TabIndex        =   19
      Top             =   4680
      Width           =   2415
      Begin VB.Shape shpLED 
         FillColor       =   &H000000FF&
         FillStyle       =   0  'Solid
         Height          =   495
         Left            =   720
         Shape           =   3  'Circle
         Top             =   360
         Width           =   975
      End
   End
   Begin VB.Frame fraEvent 
      Caption         =   "Event"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   4335
      Left            =   2760
      TabIndex        =   17
      Top             =   240
      Width           =   5175
      Begin VB.TextBox txtEvent 
         Height          =   3975
         Left            =   120
         MultiLine       =   -1  'True
         ScrollBars      =   2  'Vertical
         TabIndex        =   18
         Top             =   240
         Width           =   4935
      End
   End
   Begin MSCommLib.MSComm MSComm1 
      Left            =   1920
      Top             =   120
      _ExtentX        =   1005
      _ExtentY        =   1005
      _Version        =   393216
      DTREnable       =   -1  'True
      BaudRate        =   115200
      InputMode       =   1
   End
   Begin VB.Frame fraCommPort 
      Caption         =   "Select Comm Port"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   855
      Left            =   240
      TabIndex        =   1
      Top             =   240
      Width           =   2415
      Begin VB.ComboBox cmbCommPort 
         Height          =   315
         Left            =   240
         Style           =   2  'Dropdown List
         TabIndex        =   2
         Top             =   360
         Width           =   1335
      End
   End
   Begin VB.Frame fraData 
      Caption         =   "Data Packet"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   4575
      Left            =   240
      TabIndex        =   0
      Top             =   1200
      Width           =   2415
      Begin VB.TextBox txtTheirCheckSum 
         Height          =   285
         Left            =   240
         TabIndex        =   9
         Top             =   4200
         Width           =   1935
      End
      Begin VB.TextBox txtMyCheckSum 
         Height          =   285
         Left            =   240
         TabIndex        =   8
         Top             =   3600
         Width           =   1935
      End
      Begin VB.TextBox txtTimerTick 
         Height          =   285
         Left            =   240
         TabIndex        =   7
         Top             =   3000
         Width           =   1935
      End
      Begin VB.TextBox txtYaw 
         Height          =   285
         Left            =   240
         TabIndex        =   6
         Top             =   2400
         Width           =   1935
      End
      Begin VB.TextBox txtPitch 
         Height          =   285
         Left            =   240
         TabIndex        =   5
         Top             =   1800
         Width           =   1935
      End
      Begin VB.TextBox txtRoll 
         Height          =   285
         Left            =   240
         TabIndex        =   4
         Top             =   1200
         Width           =   1935
      End
      Begin VB.TextBox txtHeader 
         Height          =   285
         Left            =   240
         TabIndex        =   3
         Top             =   600
         Width           =   1935
      End
      Begin VB.Label Label7 
         Caption         =   "TheirCheckSum"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   700
            Underline       =   -1  'True
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   240
         TabIndex        =   16
         Top             =   3960
         Width           =   1455
      End
      Begin VB.Label Label6 
         Caption         =   "MyCheckSum"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   700
            Underline       =   -1  'True
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   240
         TabIndex        =   15
         Top             =   3360
         Width           =   1215
      End
      Begin VB.Label Label5 
         Caption         =   "Yaw in degrees"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   700
            Underline       =   -1  'True
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   240
         TabIndex        =   14
         Top             =   2160
         Width           =   1455
      End
      Begin VB.Label Label4 
         Caption         =   "Roll in degrees"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   700
            Underline       =   -1  'True
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   240
         TabIndex        =   13
         Top             =   960
         Width           =   1335
      End
      Begin VB.Label Label3 
         Caption         =   "Pitch in degrees"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   700
            Underline       =   -1  'True
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   240
         TabIndex        =   12
         Top             =   1560
         Width           =   1455
      End
      Begin VB.Label Label2 
         Caption         =   "TimerTick in Secs"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   700
            Underline       =   -1  'True
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   240
         TabIndex        =   11
         Top             =   2760
         Width           =   1695
      End
      Begin VB.Label Label1 
         Caption         =   "Header"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   700
            Underline       =   -1  'True
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   240
         TabIndex        =   10
         Top             =   360
         Width           =   855
      End
   End
   Begin VB.Menu mnuFile 
      Caption         =   "&File"
      Begin VB.Menu mnuSample 
         Caption         =   "&Sample"
      End
      Begin VB.Menu mnuSep 
         Caption         =   "-"
      End
      Begin VB.Menu mnuExit 
         Caption         =   "E&xit"
      End
   End
End
Attribute VB_Name = "Main"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
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
Private Sub Form_Load()
'populate comm port combo box
Dim X As Integer
For X = 1 To 16
    cmbCommPort.AddItem X
Next X
End Sub

Private Sub mnuExit_Click()
'quit app
End
End Sub

Private Sub mnuSample_Click()
'set error handler
On Error GoTo ErrorHandler

'test for comm port selection
If cmbCommPort.Text = "" Then
    'event
    Call ReportEvent("Comm Port not selected")
    'exit
    Exit Sub
End If

'determine action based on menu check
If mnuSample.Checked = False Then
    'event
    Call ReportEvent("Sampling started")
    'set menu check
    mnuSample.Checked = True
    'disable combo
    cmbCommPort.Enabled = False
    'set comm port
    MSComm1.CommPort = Val(cmbCommPort.Text)
    'set receive threshold to 19 bytes
    MSComm1.RThreshold = 19
    'open comm port
    MSComm1.PortOpen = True
    'populate byte array with continuous command for 0xCE
    'decimal equivalent of 0xC4 = 196
    'decimal equivalent of 0xC1 = 193
    'decimal equivalent of 0x29 = 41
    'decimal equivalent of 0xCE = 206
    arrBytes = ChrB(196) + ChrB(193) + ChrB(41) + ChrB(206)
    'send continuous command
    MSComm1.Output = arrBytes
ElseIf mnuSample.Checked = True Then
    'event
    Call ReportEvent("Sampling stopped")
    'set menu check
    mnuSample.Checked = False
    'close comm port
    MSComm1.PortOpen = False
    'enable combo
    cmbCommPort.Enabled = True
    'set checksum indicator
    shpLED.FillColor = vbRed
End If

Exit Sub
ErrorHandler:
    'set menu check
    mnuSample.Checked = False
    'close comm port
    If MSComm1.PortOpen = True Then MSComm1.PortOpen = False
    'enable combo
    cmbCommPort.Enabled = True
    'event
    Call ReportEvent("Sampling errored: " + Err.Description)
    'set checksum indicator
    shpLED.FillColor = vbRed
    'exit
    Exit Sub
End Sub

Private Function ReportEvent(MyEvent As String)
'add event to text
If txtEvent.Text = "" Then
    txtEvent.Text = MyEvent
Else
    txtEvent.Text = txtEvent.Text + Chr(13) + Chr(10) + MyEvent
End If
End Function

Private Sub MSComm1_OnComm()

'get all bytes in comm buffer
arrBytes = MSComm1.Input

'test 19 byte return and header
If Not UBound(arrBytes) = 18 Or Not arrBytes(0) = 206 Then
    'set checksum indicator
    shpLED.FillColor = vbRed
    'exit sub
    Exit Sub
End If

'test checksum
If CheckSum = False Then
    'set checksum indicator
    shpLED.FillColor = vbRed
    'exit sub
    Exit Sub
End If

'set checksum indicator
shpLED.FillColor = vbGreen

'scale radians into degrees and round to 1 decimal point
MyRoll = Round(MyRoll * 57.2958, 1)
MyPitch = Round(MyPitch * 57.2958, 1)
MyYaw = Round(MyYaw * 57.2958, 1)

'display data
txtHeader.Text = MyHeader
txtRoll.Text = MyRoll
txtPitch.Text = MyPitch
txtYaw.Text = MyYaw
txtTimerTick.Text = TimerTick
txtMyCheckSum.Text = MyCheckSum
txtTheirCheckSum.Text = TheirCheckSum

End Sub

Private Function CheckSum() As Boolean
'set error handler
On Error GoTo ErrorHandler

'-----calc TheirCheckSum
'dim
Dim X As Double
'zero variable
TheirCheckSum = 0
'calc TheirCheckSum
For X = 0 To 16
    TheirCheckSum = TheirCheckSum + CDbl(arrBytes(X))
Next X

'handle CheckSum rollover
TheirCheckSum = TheirCheckSum Mod 65536

'calc MyCheckSum
MyCheckSum = (CDbl(arrBytes(17)) * 256) + arrBytes(18)

'compare checksums
If MyCheckSum = TheirCheckSum Then
    ''set checksum return
    CheckSum = True
    'calc datapoints
    MyHeader = arrBytes(0)
    MyRoll = FloatConversion(1)
    MyPitch = FloatConversion(5)
    MyYaw = FloatConversion(9)
    TimerTick = CalcTimerTick(13)
Else
    'set checksum return
    CheckSum = False
End If

'exit
Exit Function
ErrorHandler:
    'set checksum return
    CheckSum = False
    'exit
    Exit Function
End Function

