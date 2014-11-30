' Visual Basic Hello World
' Some more comments about this file
VERSION 4.00
Begin VB.Form Form1 
   Caption         =   "Hello"
   ClientHeight    =   6030
   ClientLeft      =   1095
   ClientTop       =   1515
   ClientWidth     =   6720
   Height          =   6435
   Left            =   1035
   LinkTopic       =   "Form1"
   ScaleHeight     =   6030
   ScaleWidth      =   6720
   Top             =   1170
   Width           =   6840
   Begin VB.CommandButton Command1 
      Caption         =   "Hello World"
      Height          =   975
      Left            =   2040
      TabIndex        =   0
      Top             =   2280
      Width           =   2535
   End
End
Attribute VB_Name = "Form1"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Private Sub Command1_Click()
Cls
Print "Hello World"
End Sub