Public Class Main
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

    Private Sub ExitToolStripMenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ExitToolStripMenuItem.Click
        'quit app
        End
    End Sub

    Private Sub SampleToolStripMenuItem_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles SampleToolStripMenuItem.Click
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
        If SampleToolStripMenuItem.Checked = False Then
            'event
            Call ReportEvent("Sampling started")
            'set menu check
            SampleToolStripMenuItem.Checked = True
            'disable combo
            cmbCommPort.Enabled = False
            'set serial port number
            SerialPort1.PortName() = "COM" + Trim(cmbCommPort.Text)
            'set receive threshold
            SerialPort1.ReceivedBytesThreshold = 31
            'open serial port
            SerialPort1.Open()
            'populate byte array with continuous command for 0xC2
            'decimal equivalent of 0xC4 = 196
            'decimal equivalent of 0xC1 = 193
            'decimal equivalent of 0x29 = 41
            'decimal equivalent of 0xC2 = 194
            ReDim arrBytes(0 To 3)
            arrBytes(0) = 196
            arrBytes(1) = 193
            arrBytes(2) = 41
            arrBytes(3) = 194
            'send command
            SerialPort1.Write(arrBytes, 0, 4)
        ElseIf SampleToolStripMenuItem.Checked = True Then
            'open serial port
            SerialPort1.Close()
            'event
            Call ReportEvent("Sampling stopped")
            'set menu check
            SampleToolStripMenuItem.Checked = False
            'set checksum indicator
            lblCheckSum.ForeColor = Color.Red
            'enable combo
            cmbCommPort.Enabled = True
        End If

        Exit Sub
ErrorHandler:
        'open serial port
        If SerialPort1.IsOpen = True Then SerialPort1.Close()
        'event
        Call ReportEvent("Sampling errored: " + Err.Description)
        'set menu check
        SampleToolStripMenuItem.Checked = False
        'set checksum indicator
        lblCheckSum.ForeColor = Color.Red
        'enable combo
        cmbCommPort.Enabled = True
        'exit
        Exit Sub
    End Sub

    Private Sub Main_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load
        'populate comm port combo box
        Dim X As Integer
        For X = 1 To 32
            cmbCommPort.Items.Add(Trim(Str(X)))
        Next

        'Sets a value indicating whether to catch calls on the wrong thread that access a control's Handle property 
        txtHeader.CheckForIllegalCrossThreadCalls = False
        txtAccelX.CheckForIllegalCrossThreadCalls = False
        txtAccelY.CheckForIllegalCrossThreadCalls = False
        txtAccelZ.CheckForIllegalCrossThreadCalls = False
        txtAngRateX.CheckForIllegalCrossThreadCalls = False
        txtAngRateY.CheckForIllegalCrossThreadCalls = False
        txtAngRateZ.CheckForIllegalCrossThreadCalls = False
        txtTimerTick.CheckForIllegalCrossThreadCalls = False
        txtMyCheckSum.CheckForIllegalCrossThreadCalls = False
        txtTheirCheckSum.CheckForIllegalCrossThreadCalls = False
    End Sub
    Private Function ReportEvent(ByVal MyEvent As String) As Boolean
        'add event to text
        If txtEvent.Text = "" Then
            txtEvent.Text = MyEvent
        Else
            txtEvent.Text = txtEvent.Text + Chr(13) + Chr(10) + MyEvent
        End If
    End Function

    Private Function CheckSum() As Boolean
        'set error handler
        On Error GoTo ErrorHandler

        '-----calc TheirCheckSum
        'dim
        Dim X As Double
        'zero variable
        TheirCheckSum = 0
        'calc TheirCheckSum
        For X = 0 To 28
            TheirCheckSum = TheirCheckSum + CDbl(arrBytes(X))
        Next X

        'handle CheckSum rollover
        TheirCheckSum = TheirCheckSum Mod 65536

        'calc MyCheckSum
        MyCheckSum = (CDbl(arrBytes(29)) * 256) + arrBytes(30)

        'compare checksums
        If MyCheckSum = TheirCheckSum Then
            ''set checksum return
            CheckSum = True
            'calc datapoints
            MyHeader = arrBytes(0)
            'convert array of 32bit floating point values in IEEE-754 format
            arrTemp(0) = arrBytes(4)
            arrTemp(1) = arrBytes(3)
            arrTemp(2) = arrBytes(2)
            arrTemp(3) = arrBytes(1)
            AccelX = BitConverter.ToSingle(arrTemp, 0)
            'round
            AccelX = Math.Round(AccelX, 4)
            'convert array of 32bit floating point values in IEEE-754 format
            arrTemp(0) = arrBytes(8)
            arrTemp(1) = arrBytes(7)
            arrTemp(2) = arrBytes(6)
            arrTemp(3) = arrBytes(5)
            AccelY = BitConverter.ToSingle(arrTemp, 0)
            'round
            AccelY = Math.Round(AccelY, 4)
            'convert array of 32bit floating point values in IEEE-754 format
            arrTemp(0) = arrBytes(12)
            arrTemp(1) = arrBytes(11)
            arrTemp(2) = arrBytes(10)
            arrTemp(3) = arrBytes(9)
            AccelZ = BitConverter.ToSingle(arrTemp, 0)
            'round
            AccelZ = Math.Round(AccelZ, 4)
            'convert array of 32bit floating point values in IEEE-754 format
            arrTemp(0) = arrBytes(16)
            arrTemp(1) = arrBytes(15)
            arrTemp(2) = arrBytes(14)
            arrTemp(3) = arrBytes(13)
            AngRateX = BitConverter.ToSingle(arrTemp, 0)
            'round
            AngRateX = Math.Round(AngRateX, 4)
            'convert array of 32bit floating point values in IEEE-754 format
            arrTemp(0) = arrBytes(20)
            arrTemp(1) = arrBytes(19)
            arrTemp(2) = arrBytes(18)
            arrTemp(3) = arrBytes(17)
            AngRateY = BitConverter.ToSingle(arrTemp, 0)
            'round
            AngRateY = Math.Round(AngRateY, 4)
            'convert array of 32bit floating point values in IEEE-754 format
            arrTemp(0) = arrBytes(24)
            arrTemp(1) = arrBytes(23)
            arrTemp(2) = arrBytes(22)
            arrTemp(3) = arrBytes(21)
            AngRateZ = BitConverter.ToSingle(arrTemp, 0)
            'round
            AngRateZ = Math.Round(AngRateZ, 4)
            'calc timer tick
            TimerTick = CalcTimerTick(25)
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

    Private Sub SerialPort1_DataReceived(ByVal sender As System.Object, ByVal e As System.IO.Ports.SerialDataReceivedEventArgs) Handles SerialPort1.DataReceived
        'set error handler
        On Error GoTo ErrorHandler

        'set variable
        UpperLimitOfArray = SerialPort1.BytesToRead - 1
        'prep array
        ReDim arrBytes(0 To UpperLimitOfArray)
        'get comm buffer
        SerialPort1.Read(arrBytes, 0, SerialPort1.BytesToRead)

        'test for no byte return
        If UBound(arrBytes) = -1 Then
            'set checksum indicator
            lblCheckSum.ForeColor = Color.Red
            'exit
            Exit Sub
        End If

        'test 31 byte return and header
        If Not UBound(arrBytes) = 30 And Not arrBytes(0) = 194 Then
            'set checksum indicator
            lblCheckSum.ForeColor = Color.Red
            'exit
            Exit Sub
        End If

        'test checksum
        If CheckSum() = False Then
            'set checksum indicator
            lblCheckSum.ForeColor = Color.Red
            'exit
            Exit Sub
        End If

        'set checksum indicator
        lblCheckSum.ForeColor = Color.Green

        'display data
        txtHeader.Text = MyHeader
        txtAccelX.Text = AccelX
        txtAccelY.Text = AccelY
        txtAccelZ.Text = AccelZ
        txtAngRateX.Text = AngRateX
        txtAngRateY.Text = AngRateY
        txtAngRateZ.Text = AngRateZ
        txtTimerTick.Text = TimerTick
        txtMyCheckSum.Text = MyCheckSum
        txtTheirCheckSum.Text = TheirCheckSum

        Exit Sub
ErrorHandler:

        'close serial port
        If SerialPort1.IsOpen = True Then SerialPort1.Close()

    End Sub
End Class
