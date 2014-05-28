<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class Main
    Inherits System.Windows.Forms.Form

    'Form overrides dispose to clean up the component list.
    <System.Diagnostics.DebuggerNonUserCode()> _
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        If disposing AndAlso components IsNot Nothing Then
            components.Dispose()
        End If
        MyBase.Dispose(disposing)
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Me.components = New System.ComponentModel.Container
        Dim resources As System.ComponentModel.ComponentResourceManager = New System.ComponentModel.ComponentResourceManager(GetType(Main))
        Me.MenuStrip1 = New System.Windows.Forms.MenuStrip
        Me.FileToolStripMenuItem = New System.Windows.Forms.ToolStripMenuItem
        Me.SampleToolStripMenuItem = New System.Windows.Forms.ToolStripMenuItem
        Me.ToolStripMenuItem1 = New System.Windows.Forms.ToolStripSeparator
        Me.ExitToolStripMenuItem = New System.Windows.Forms.ToolStripMenuItem
        Me.SerialPort1 = New System.IO.Ports.SerialPort(Me.components)
        Me.grpData = New System.Windows.Forms.GroupBox
        Me.Label10 = New System.Windows.Forms.Label
        Me.txtTheirCheckSum = New System.Windows.Forms.TextBox
        Me.Label9 = New System.Windows.Forms.Label
        Me.txtMyCheckSum = New System.Windows.Forms.TextBox
        Me.Label8 = New System.Windows.Forms.Label
        Me.txtTimerTick = New System.Windows.Forms.TextBox
        Me.Label7 = New System.Windows.Forms.Label
        Me.txtAngRateZ = New System.Windows.Forms.TextBox
        Me.Label6 = New System.Windows.Forms.Label
        Me.txtAngRateY = New System.Windows.Forms.TextBox
        Me.Label5 = New System.Windows.Forms.Label
        Me.txtAngRateX = New System.Windows.Forms.TextBox
        Me.Label4 = New System.Windows.Forms.Label
        Me.txtAccelZ = New System.Windows.Forms.TextBox
        Me.Label3 = New System.Windows.Forms.Label
        Me.txtAccelY = New System.Windows.Forms.TextBox
        Me.Label2 = New System.Windows.Forms.Label
        Me.txtAccelX = New System.Windows.Forms.TextBox
        Me.Label1 = New System.Windows.Forms.Label
        Me.txtHeader = New System.Windows.Forms.TextBox
        Me.grpCS = New System.Windows.Forms.GroupBox
        Me.lblCheckSum = New System.Windows.Forms.Label
        Me.grpEvent = New System.Windows.Forms.GroupBox
        Me.txtEvent = New System.Windows.Forms.TextBox
        Me.grpCommPort = New System.Windows.Forms.GroupBox
        Me.cmbCommPort = New System.Windows.Forms.ComboBox
        Me.MenuStrip1.SuspendLayout()
        Me.grpData.SuspendLayout()
        Me.grpCS.SuspendLayout()
        Me.grpEvent.SuspendLayout()
        Me.grpCommPort.SuspendLayout()
        Me.SuspendLayout()
        '
        'MenuStrip1
        '
        Me.MenuStrip1.Items.AddRange(New System.Windows.Forms.ToolStripItem() {Me.FileToolStripMenuItem})
        Me.MenuStrip1.Location = New System.Drawing.Point(0, 0)
        Me.MenuStrip1.Name = "MenuStrip1"
        Me.MenuStrip1.Size = New System.Drawing.Size(792, 24)
        Me.MenuStrip1.TabIndex = 0
        Me.MenuStrip1.Text = "MenuStrip1"
        '
        'FileToolStripMenuItem
        '
        Me.FileToolStripMenuItem.DropDownItems.AddRange(New System.Windows.Forms.ToolStripItem() {Me.SampleToolStripMenuItem, Me.ToolStripMenuItem1, Me.ExitToolStripMenuItem})
        Me.FileToolStripMenuItem.Name = "FileToolStripMenuItem"
        Me.FileToolStripMenuItem.Size = New System.Drawing.Size(35, 20)
        Me.FileToolStripMenuItem.Text = "File"
        '
        'SampleToolStripMenuItem
        '
        Me.SampleToolStripMenuItem.Name = "SampleToolStripMenuItem"
        Me.SampleToolStripMenuItem.Size = New System.Drawing.Size(119, 22)
        Me.SampleToolStripMenuItem.Text = "Sample"
        '
        'ToolStripMenuItem1
        '
        Me.ToolStripMenuItem1.Name = "ToolStripMenuItem1"
        Me.ToolStripMenuItem1.Size = New System.Drawing.Size(116, 6)
        '
        'ExitToolStripMenuItem
        '
        Me.ExitToolStripMenuItem.Name = "ExitToolStripMenuItem"
        Me.ExitToolStripMenuItem.Size = New System.Drawing.Size(119, 22)
        Me.ExitToolStripMenuItem.Text = "Exit"
        '
        'SerialPort1
        '
        '
        'grpData
        '
        Me.grpData.Controls.Add(Me.Label10)
        Me.grpData.Controls.Add(Me.txtTheirCheckSum)
        Me.grpData.Controls.Add(Me.Label9)
        Me.grpData.Controls.Add(Me.txtMyCheckSum)
        Me.grpData.Controls.Add(Me.Label8)
        Me.grpData.Controls.Add(Me.txtTimerTick)
        Me.grpData.Controls.Add(Me.Label7)
        Me.grpData.Controls.Add(Me.txtAngRateZ)
        Me.grpData.Controls.Add(Me.Label6)
        Me.grpData.Controls.Add(Me.txtAngRateY)
        Me.grpData.Controls.Add(Me.Label5)
        Me.grpData.Controls.Add(Me.txtAngRateX)
        Me.grpData.Controls.Add(Me.Label4)
        Me.grpData.Controls.Add(Me.txtAccelZ)
        Me.grpData.Controls.Add(Me.Label3)
        Me.grpData.Controls.Add(Me.txtAccelY)
        Me.grpData.Controls.Add(Me.Label2)
        Me.grpData.Controls.Add(Me.txtAccelX)
        Me.grpData.Controls.Add(Me.Label1)
        Me.grpData.Controls.Add(Me.txtHeader)
        Me.grpData.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.grpData.Location = New System.Drawing.Point(12, 94)
        Me.grpData.Name = "grpData"
        Me.grpData.Size = New System.Drawing.Size(168, 444)
        Me.grpData.TabIndex = 30
        Me.grpData.TabStop = False
        Me.grpData.Text = "Data Packet"
        '
        'Label10
        '
        Me.Label10.AutoSize = True
        Me.Label10.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!, CType((System.Drawing.FontStyle.Bold Or System.Drawing.FontStyle.Underline), System.Drawing.FontStyle), System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label10.Location = New System.Drawing.Point(13, 398)
        Me.Label10.Name = "Label10"
        Me.Label10.Size = New System.Drawing.Size(96, 13)
        Me.Label10.TabIndex = 45
        Me.Label10.Text = "TheirCheckSum"
        '
        'txtTheirCheckSum
        '
        Me.txtTheirCheckSum.Location = New System.Drawing.Point(16, 415)
        Me.txtTheirCheckSum.Name = "txtTheirCheckSum"
        Me.txtTheirCheckSum.Size = New System.Drawing.Size(143, 20)
        Me.txtTheirCheckSum.TabIndex = 44
        Me.txtTheirCheckSum.TabStop = False
        '
        'Label9
        '
        Me.Label9.AutoSize = True
        Me.Label9.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!, CType((System.Drawing.FontStyle.Bold Or System.Drawing.FontStyle.Underline), System.Drawing.FontStyle), System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label9.Location = New System.Drawing.Point(13, 357)
        Me.Label9.Name = "Label9"
        Me.Label9.Size = New System.Drawing.Size(83, 13)
        Me.Label9.TabIndex = 43
        Me.Label9.Text = "MyCheckSum"
        '
        'txtMyCheckSum
        '
        Me.txtMyCheckSum.Location = New System.Drawing.Point(16, 374)
        Me.txtMyCheckSum.Name = "txtMyCheckSum"
        Me.txtMyCheckSum.Size = New System.Drawing.Size(143, 20)
        Me.txtMyCheckSum.TabIndex = 42
        Me.txtMyCheckSum.TabStop = False
        '
        'Label8
        '
        Me.Label8.AutoSize = True
        Me.Label8.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!, CType((System.Drawing.FontStyle.Bold Or System.Drawing.FontStyle.Underline), System.Drawing.FontStyle), System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label8.Location = New System.Drawing.Point(13, 316)
        Me.Label8.Name = "Label8"
        Me.Label8.Size = New System.Drawing.Size(113, 13)
        Me.Label8.TabIndex = 41
        Me.Label8.Text = "Timer Tick in Secs"
        '
        'txtTimerTick
        '
        Me.txtTimerTick.Location = New System.Drawing.Point(16, 333)
        Me.txtTimerTick.Name = "txtTimerTick"
        Me.txtTimerTick.Size = New System.Drawing.Size(143, 20)
        Me.txtTimerTick.TabIndex = 40
        Me.txtTimerTick.TabStop = False
        '
        'Label7
        '
        Me.Label7.AutoSize = True
        Me.Label7.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!, CType((System.Drawing.FontStyle.Bold Or System.Drawing.FontStyle.Underline), System.Drawing.FontStyle), System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label7.Location = New System.Drawing.Point(13, 275)
        Me.Label7.Name = "Label7"
        Me.Label7.Size = New System.Drawing.Size(137, 13)
        Me.Label7.TabIndex = 39
        Me.Label7.Text = "Z AngRate in Deg/Sec"
        '
        'txtAngRateZ
        '
        Me.txtAngRateZ.Location = New System.Drawing.Point(16, 292)
        Me.txtAngRateZ.Name = "txtAngRateZ"
        Me.txtAngRateZ.Size = New System.Drawing.Size(143, 20)
        Me.txtAngRateZ.TabIndex = 38
        Me.txtAngRateZ.TabStop = False
        '
        'Label6
        '
        Me.Label6.AutoSize = True
        Me.Label6.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!, CType((System.Drawing.FontStyle.Bold Or System.Drawing.FontStyle.Underline), System.Drawing.FontStyle), System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label6.Location = New System.Drawing.Point(13, 234)
        Me.Label6.Name = "Label6"
        Me.Label6.Size = New System.Drawing.Size(137, 13)
        Me.Label6.TabIndex = 37
        Me.Label6.Text = "Y AngRate in Deg/Sec"
        '
        'txtAngRateY
        '
        Me.txtAngRateY.Location = New System.Drawing.Point(16, 251)
        Me.txtAngRateY.Name = "txtAngRateY"
        Me.txtAngRateY.Size = New System.Drawing.Size(143, 20)
        Me.txtAngRateY.TabIndex = 36
        Me.txtAngRateY.TabStop = False
        '
        'Label5
        '
        Me.Label5.AutoSize = True
        Me.Label5.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!, CType((System.Drawing.FontStyle.Bold Or System.Drawing.FontStyle.Underline), System.Drawing.FontStyle), System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label5.Location = New System.Drawing.Point(13, 193)
        Me.Label5.Name = "Label5"
        Me.Label5.Size = New System.Drawing.Size(137, 13)
        Me.Label5.TabIndex = 35
        Me.Label5.Text = "X AngRate in Rad/Sec"
        '
        'txtAngRateX
        '
        Me.txtAngRateX.Location = New System.Drawing.Point(16, 210)
        Me.txtAngRateX.Name = "txtAngRateX"
        Me.txtAngRateX.Size = New System.Drawing.Size(143, 20)
        Me.txtAngRateX.TabIndex = 34
        Me.txtAngRateX.TabStop = False
        '
        'Label4
        '
        Me.Label4.AutoSize = True
        Me.Label4.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!, CType((System.Drawing.FontStyle.Bold Or System.Drawing.FontStyle.Underline), System.Drawing.FontStyle), System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label4.Location = New System.Drawing.Point(13, 152)
        Me.Label4.Name = "Label4"
        Me.Label4.Size = New System.Drawing.Size(84, 13)
        Me.Label4.TabIndex = 33
        Me.Label4.Text = "Z Accel in Gs"
        '
        'txtAccelZ
        '
        Me.txtAccelZ.Location = New System.Drawing.Point(16, 169)
        Me.txtAccelZ.Name = "txtAccelZ"
        Me.txtAccelZ.Size = New System.Drawing.Size(143, 20)
        Me.txtAccelZ.TabIndex = 32
        Me.txtAccelZ.TabStop = False
        '
        'Label3
        '
        Me.Label3.AutoSize = True
        Me.Label3.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!, CType((System.Drawing.FontStyle.Bold Or System.Drawing.FontStyle.Underline), System.Drawing.FontStyle), System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label3.Location = New System.Drawing.Point(13, 111)
        Me.Label3.Name = "Label3"
        Me.Label3.Size = New System.Drawing.Size(84, 13)
        Me.Label3.TabIndex = 31
        Me.Label3.Text = "Y Accel in Gs"
        '
        'txtAccelY
        '
        Me.txtAccelY.Location = New System.Drawing.Point(16, 128)
        Me.txtAccelY.Name = "txtAccelY"
        Me.txtAccelY.Size = New System.Drawing.Size(143, 20)
        Me.txtAccelY.TabIndex = 30
        Me.txtAccelY.TabStop = False
        '
        'Label2
        '
        Me.Label2.AutoSize = True
        Me.Label2.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!, CType((System.Drawing.FontStyle.Bold Or System.Drawing.FontStyle.Underline), System.Drawing.FontStyle), System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label2.Location = New System.Drawing.Point(13, 70)
        Me.Label2.Name = "Label2"
        Me.Label2.Size = New System.Drawing.Size(84, 13)
        Me.Label2.TabIndex = 29
        Me.Label2.Text = "X Accel in Gs"
        '
        'txtAccelX
        '
        Me.txtAccelX.Location = New System.Drawing.Point(16, 87)
        Me.txtAccelX.Name = "txtAccelX"
        Me.txtAccelX.Size = New System.Drawing.Size(143, 20)
        Me.txtAccelX.TabIndex = 28
        Me.txtAccelX.TabStop = False
        '
        'Label1
        '
        Me.Label1.AutoSize = True
        Me.Label1.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!, CType((System.Drawing.FontStyle.Bold Or System.Drawing.FontStyle.Underline), System.Drawing.FontStyle), System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label1.Location = New System.Drawing.Point(13, 29)
        Me.Label1.Name = "Label1"
        Me.Label1.Size = New System.Drawing.Size(48, 13)
        Me.Label1.TabIndex = 27
        Me.Label1.Text = "Header"
        '
        'txtHeader
        '
        Me.txtHeader.CausesValidation = False
        Me.txtHeader.Location = New System.Drawing.Point(16, 46)
        Me.txtHeader.Name = "txtHeader"
        Me.txtHeader.Size = New System.Drawing.Size(143, 20)
        Me.txtHeader.TabIndex = 26
        Me.txtHeader.TabStop = False
        '
        'grpCS
        '
        Me.grpCS.Controls.Add(Me.lblCheckSum)
        Me.grpCS.Location = New System.Drawing.Point(186, 470)
        Me.grpCS.Name = "grpCS"
        Me.grpCS.Size = New System.Drawing.Size(164, 68)
        Me.grpCS.TabIndex = 29
        Me.grpCS.TabStop = False
        '
        'lblCheckSum
        '
        Me.lblCheckSum.AutoSize = True
        Me.lblCheckSum.Font = New System.Drawing.Font("Microsoft Sans Serif", 18.0!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.lblCheckSum.ForeColor = System.Drawing.Color.Red
        Me.lblCheckSum.Location = New System.Drawing.Point(11, 22)
        Me.lblCheckSum.Name = "lblCheckSum"
        Me.lblCheckSum.Size = New System.Drawing.Size(138, 29)
        Me.lblCheckSum.TabIndex = 0
        Me.lblCheckSum.Text = "CheckSum"
        '
        'grpEvent
        '
        Me.grpEvent.Controls.Add(Me.txtEvent)
        Me.grpEvent.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.grpEvent.Location = New System.Drawing.Point(186, 37)
        Me.grpEvent.Name = "grpEvent"
        Me.grpEvent.Size = New System.Drawing.Size(387, 427)
        Me.grpEvent.TabIndex = 28
        Me.grpEvent.TabStop = False
        Me.grpEvent.Text = "Event"
        '
        'txtEvent
        '
        Me.txtEvent.Location = New System.Drawing.Point(6, 19)
        Me.txtEvent.Multiline = True
        Me.txtEvent.Name = "txtEvent"
        Me.txtEvent.ScrollBars = System.Windows.Forms.ScrollBars.Vertical
        Me.txtEvent.Size = New System.Drawing.Size(375, 402)
        Me.txtEvent.TabIndex = 0
        Me.txtEvent.TabStop = False
        '
        'grpCommPort
        '
        Me.grpCommPort.Controls.Add(Me.cmbCommPort)
        Me.grpCommPort.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.grpCommPort.Location = New System.Drawing.Point(12, 37)
        Me.grpCommPort.Name = "grpCommPort"
        Me.grpCommPort.Size = New System.Drawing.Size(168, 51)
        Me.grpCommPort.TabIndex = 27
        Me.grpCommPort.TabStop = False
        Me.grpCommPort.Text = "Select Comm Port"
        '
        'cmbCommPort
        '
        Me.cmbCommPort.FormattingEnabled = True
        Me.cmbCommPort.Location = New System.Drawing.Point(6, 19)
        Me.cmbCommPort.Name = "cmbCommPort"
        Me.cmbCommPort.Size = New System.Drawing.Size(121, 21)
        Me.cmbCommPort.TabIndex = 2
        '
        'Main
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(792, 566)
        Me.Controls.Add(Me.grpData)
        Me.Controls.Add(Me.grpCS)
        Me.Controls.Add(Me.grpEvent)
        Me.Controls.Add(Me.grpCommPort)
        Me.Controls.Add(Me.MenuStrip1)
        Me.Icon = CType(resources.GetObject("$this.Icon"), System.Drawing.Icon)
        Me.MainMenuStrip = Me.MenuStrip1
        Me.Name = "Main"
        Me.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen
        Me.Text = "3DM-GX3-25 Sample Code"
        Me.MenuStrip1.ResumeLayout(False)
        Me.MenuStrip1.PerformLayout()
        Me.grpData.ResumeLayout(False)
        Me.grpData.PerformLayout()
        Me.grpCS.ResumeLayout(False)
        Me.grpCS.PerformLayout()
        Me.grpEvent.ResumeLayout(False)
        Me.grpEvent.PerformLayout()
        Me.grpCommPort.ResumeLayout(False)
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub
    Friend WithEvents MenuStrip1 As System.Windows.Forms.MenuStrip
    Friend WithEvents FileToolStripMenuItem As System.Windows.Forms.ToolStripMenuItem
    Friend WithEvents SampleToolStripMenuItem As System.Windows.Forms.ToolStripMenuItem
    Friend WithEvents ToolStripMenuItem1 As System.Windows.Forms.ToolStripSeparator
    Friend WithEvents ExitToolStripMenuItem As System.Windows.Forms.ToolStripMenuItem
    Friend WithEvents SerialPort1 As System.IO.Ports.SerialPort
    Friend WithEvents grpData As System.Windows.Forms.GroupBox
    Friend WithEvents Label10 As System.Windows.Forms.Label
    Friend WithEvents txtTheirCheckSum As System.Windows.Forms.TextBox
    Friend WithEvents Label9 As System.Windows.Forms.Label
    Friend WithEvents txtMyCheckSum As System.Windows.Forms.TextBox
    Friend WithEvents Label8 As System.Windows.Forms.Label
    Friend WithEvents txtTimerTick As System.Windows.Forms.TextBox
    Friend WithEvents Label7 As System.Windows.Forms.Label
    Friend WithEvents txtAngRateZ As System.Windows.Forms.TextBox
    Friend WithEvents Label6 As System.Windows.Forms.Label
    Friend WithEvents txtAngRateY As System.Windows.Forms.TextBox
    Friend WithEvents Label5 As System.Windows.Forms.Label
    Friend WithEvents txtAngRateX As System.Windows.Forms.TextBox
    Friend WithEvents Label4 As System.Windows.Forms.Label
    Friend WithEvents txtAccelZ As System.Windows.Forms.TextBox
    Friend WithEvents Label3 As System.Windows.Forms.Label
    Friend WithEvents txtAccelY As System.Windows.Forms.TextBox
    Friend WithEvents Label2 As System.Windows.Forms.Label
    Friend WithEvents txtAccelX As System.Windows.Forms.TextBox
    Friend WithEvents Label1 As System.Windows.Forms.Label
    Friend WithEvents txtHeader As System.Windows.Forms.TextBox
    Friend WithEvents grpCS As System.Windows.Forms.GroupBox
    Friend WithEvents lblCheckSum As System.Windows.Forms.Label
    Friend WithEvents grpEvent As System.Windows.Forms.GroupBox
    Friend WithEvents txtEvent As System.Windows.Forms.TextBox
    Friend WithEvents grpCommPort As System.Windows.Forms.GroupBox
    Friend WithEvents cmbCommPort As System.Windows.Forms.ComboBox

End Class
