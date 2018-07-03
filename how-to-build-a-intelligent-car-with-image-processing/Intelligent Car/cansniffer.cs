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
        public SerialPort serialPort;
        public Form1(){
            InitializeComponent();
        }

        private void ui_btn_read_Click(object sender, EventArgs e){
            if (serialPort == null || !serialPort.IsOpen){
                serialPort = new SerialPort("COM7");

                serialPort.Open();
            }else{
                serialPort.Close();
            }

            if (ui_btn_read.Text == "ReadPort"){
                ui_btn_read.Text = "StopRead";
            }else{
                ui_btn_read.Text = "ReadPort";
            }
            
            isRead = !isRead;
            readData();
        }
        
        private bool isRead = false;
        private void readData(){
            string fileName = DateTime.Now.ToString("HHmmss") + ".txt";
            byte[] data = new byte[20];
            while (true){
                if (!isRead){
                    return;
                }
                int len = serialPort.BytesToRead;
                len = len > 20 ? 20 : len;
                serialPort.Read(data, 0, len);
                using (var stream = new FileStream(fileName, FileMode.Append)){
                    stream.Write(data, 0, data.Length);
                }
                StringBuilder hex = new StringBuilder(data.Length * 2);
                for (int i = 0; i < data.Length; i += 2){
                    hex.AppendFormat("{0:x2}{1:x2} ", data[i], data[i + 1]);
                }
                ui_rtb_dataRead.Text += hex.ToString();
                Application.DoEvents();
            }
        }

        List<string> frames;
        private void updateFrameData(List<byte> data){
            try {
                string idf = (BitConverter.ToUInt16(data.ToArray(), 0) & 0X7FF0).ToString("X2");
                string ids = (BitConverter.ToUInt32(data.ToArray(), 1) & 0X03FFFF00).ToString("X2");
                //string len = ((BitConverter.ToUInt16(data.ToArray(), 4) & 0X1E00) >> 9).ToString();
                string len = ((BitConverter.ToUInt16(data.ToArray(), 1) & 0X01E0) >> 5).ToString();


                StringBuilder formatdata = new StringBuilder(data.Count * 2);
                for (int i = 0; i < data.Count; i += 2){
                    formatdata.AppendFormat("{0:x2}{1:x2} ", data[i], data[i + 1]);
                }
                frames.Add(idf + " " + ids + "  " + len + " " );
            }catch{ };
            /*if (!frames.Exists(s => s == )){
                frames.Add();
            }*/
        }

        private void ui_btn_readfile_Click(object sender, EventArgs e){
            frames = new List<string>();
            int nof7Fs = 0;
            byte[] databyte = File.ReadAllBytes("222809.txt");
            List<byte> realdata = new List<byte>();
            for (int i = 0, charCount = 0; i < databyte.Length; i++){
                string byted = (databyte[i]).ToString("X2");
                if (byted != "3F"){
                    realdata.Add(databyte[i]);
                    if (byted == "7F"){
                        updateFrameData(realdata);
                        nof7Fs++;
                        realdata.Clear();
                        AppendText(this.ui_rtb_dataRead, Color.Red, Color.Yellow, byted);
                        charCount++;
                    }else{
                        AppendText(this.ui_rtb_dataRead, Color.Black, Color.White, byted);
                        charCount++;
                    }
                }
                if (charCount == 2){
                    AppendText(this.ui_rtb_dataRead, Color.Black, Color.White, " ");
                    charCount = 0;
                }
            }
            ui_lbl_info.Text = "7F: " + nof7Fs;
            ui_rtb_frames.Text = frames.Aggregate((i, j) => i + "\n" + j);

            //ui_rtb_dataRead.Text += (BitConverter.ToInt16(databyte, i) | 0x7F).ToString("X");
        }

        void AppendText(RichTextBox box, Color fcolor, Color bcolor, string text){
            int start = box.TextLength;
            box.AppendText(text);
            int end = box.TextLength;

            // Textbox may transform chars, so (end-start) != text.Length
            box.Select(start, end - start);{
                box.SelectionColor = fcolor;
                box.SelectionBackColor = bcolor;
                // could set box.SelectionBackColor, box.SelectionFont too.
            }
            box.SelectionLength = 0; // clear
        }

        private void button1_Click(object sender, EventArgs e){
            frames = new List<string>();
            int nof7Fs = 0;
            string databyte = File.ReadAllText("p5.txt");
            List<byte> realdata = new List<byte>();
            for (int i = 0, charCount = 0; i < databyte.Length - 1; i += 2){
                string byted = databyte[i].ToString() + databyte[i + 1].ToString();
                if (byted != "3F"){
                    realdata.Add(Encoding.ASCII.GetBytes(byted)[0]);
                    if (byted == "7f"){
                        updateFrameData(realdata);
                        nof7Fs++;
                        realdata.Clear();
                        AppendText(this.ui_rtb_dataRead, Color.Red, Color.Yellow, byted);
                        charCount++;
                    }else{
                        AppendText(this.ui_rtb_dataRead, Color.Black, Color.White, byted);
                        charCount++;
                    }
                }
                if (charCount == 2){
                    AppendText(this.ui_rtb_dataRead, Color.Black, Color.White, " ");
                    charCount = 0;
                }
            }
            ui_lbl_info.Text = "7F: " + nof7Fs;
            ui_rtb_frames.Text = frames.Aggregate((i, j) => i + "\n" + j);
        }

        private void button2_Click(object sender, EventArgs e){
            string databyte = File.ReadAllText("f_2180.txt"); //f_47A0, f_2180, f_5880
            List<byte> realdata = new List<byte>();

            if (serialPort == null || !serialPort.IsOpen){
                serialPort = new SerialPort("COM7");
                serialPort.Open();
            }
            for (int i = 0; i < databyte.Length - 1; i++){
                string byted = databyte[i].ToString() + databyte[i + 1].ToString();
                serialPort.Write(byted.ToLower());
            }
        }
    }
}
