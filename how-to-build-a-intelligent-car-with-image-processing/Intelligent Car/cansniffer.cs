using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CANReadWrite{
    public partial class Form1 : Form{
        public SerialPort CabelPort;
        public Form1(){
            //CabelPort = new SerialPort("COM3");
            //CabelPort.Open();

            InitializeComponent();
        }
        
        private void ui_btn_read_Click(object sender, EventArgs e){
            string data = "";
            for (int i =0; i<50000; i++){
                //ui_rtb_dataRead.Text += CabelPort.ReadExisting();
                data += CabelPort.ReadExisting();
                ui_rtb_dataRead.Text = data;
                byte[] databyte;
                databyte = Encoding.ASCII.GetBytes(ui_rtb_dataRead.Text);
                File.WriteAllBytes(DateTime.Now.ToString("HH:mm:ss") + ".txt", databyte);
            }
            //CabelPort.WriteLine();


        }

        private void ui_btn_readfile_Click(object sender, EventArgs e){
            List<string> frames = new List<string>(); 
            int nof7Fs = 0;
            byte[] databyte = File.ReadAllBytes("data01.txt");
            for (int i = 0; i < databyte.Length; i += 2){
                string fbyted = (databyte[i]).ToString("X2");
                if (fbyted == "7F"){
                    nof7Fs++;
                    frames.Add(fbyted);
                }
                if (i + 1 < databyte.Length){
                    string sbyted = (databyte[i + 1]).ToString("X2");
                    if (sbyted == "7F"){
                        nof7Fs++;
                    }
                    ui_rtb_dataRead.Text += fbyted + sbyted + " ";
                }else{
                    ui_rtb_dataRead.Text += fbyted + " ";
                }
                //Refresh form intervally
                if (i % 200 == 0){
                    refreshRichTextBox();
                }
            }
            ui_lbl_info.Text = "7F: " + nof7Fs;
            ui_rtb_frames.Text = frames.Aggregate((i, j) => i + " " + j); ;
            refreshRichTextBox();



            //ui_rtb_dataRead.Text += (BitConverter.ToInt16(databyte, i) | 0x7F).ToString("X");
        }

        private void refreshRichTextBox(){
            for (int i = 0; i < ui_rtb_dataRead.Text.Length; i++){
                if (ui_rtb_dataRead.Text[i] == '7' && ui_rtb_dataRead.Text[i + 1] == 'F'){
                    ui_rtb_dataRead.SelectionStart = i;
                    ui_rtb_dataRead.SelectionLength = 2;
                    ui_rtb_dataRead.SelectionColor = Color.Red;
                    ui_rtb_dataRead.SelectionBackColor = Color.Yellow;
                }
            }
            ui_rtb_dataRead.ScrollToCaret();
            Application.DoEvents();
        }
    }
}
