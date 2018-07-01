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
        }

        private void ui_btn_write_Click(object sender, EventArgs e){
            //CabelPort.WriteLine();
            ui_lbl_info.Text = "+ ";
            byte[] databyte = File.ReadAllBytes("data02.txt");
            for (int i = 0; i < databyte.Length; i++){
                string byted = (databyte[i]).ToString("X");
                if (byted == "7F"){
                    ui_rtb_dataRead.Text += "[" + (databyte[i]).ToString("X") + "]         ";
                    ui_lbl_info.Text += "7F ";
                }else{
                    ui_rtb_dataRead.Text += (databyte[i]).ToString("X");
                }
                //ui_rtb_dataRead.Text += (BitConverter.ToInt16(databyte, i) | 0x7F).ToString("X");
                ui_rtb_dataRead.Text += " ";
            }
        }
    }
}
