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
            InitializeComponent();
        }
        
        private void ui_btn_read_Click(object sender, EventArgs e){
            CabelPort = new SerialPort("COM3");
            CabelPort.Open();

            string data = "";
            for (int i =0; i<50000; i++){
                data += CabelPort.ReadExisting();
                ui_rtb_dataRead.Text = data;
                byte[] databyte;
                databyte = Encoding.ASCII.GetBytes(ui_rtb_dataRead.Text);
                File.WriteAllBytes(DateTime.Now.ToString("HHmmss") + ".txt", databyte);
            }
            //CabelPort.WriteLine();


        }

        List<string> frames;
        private void updateFrameData(List<byte> data){
            string idf = (BitConverter.ToUInt16(data.ToArray(), 0) & 0X7FF0).ToString("X2");
            string ids = (BitConverter.ToUInt32(data.ToArray(), 1) & 0X03FFFF00).ToString("X2");
            //string len = ((BitConverter.ToUInt16(data.ToArray(), 4) & 0X1E00) >> 9).ToString();
            string len = ((BitConverter.ToUInt16(data.ToArray(), 1) & 0X01E0) >> 5).ToString();
            frames.Add(idf + " " + ids + "  " + len);

            /*if (!frames.Exists(s => s == )){
                frames.Add();
            }*/
        }

        private void ui_btn_readfile_Click(object sender, EventArgs e){
            frames = new List<string>();
            int nof7Fs = 0;
            byte[] databyte = File.ReadAllBytes("data03.txt");
            List<byte> realdata = new List<byte>();
            for (int i = 0, charCount = 0; i < databyte.Length; i++){
                string byted = (databyte[i]).ToString("X2");
                if (byted != "3F" && byted != "29"){
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
                if (charCount == 1){
                    AppendText(this.ui_rtb_dataRead, Color.Black, Color.White, " ");
                    charCount = 0;
                }
            }
            ui_lbl_info.Text = "7F: " + nof7Fs;
            ui_rtb_frames.Text = frames.Aggregate((i, j) => i + "\n" + j); ;

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
    }
}
