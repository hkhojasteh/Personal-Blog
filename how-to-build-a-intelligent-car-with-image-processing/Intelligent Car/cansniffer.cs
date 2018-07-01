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
                //ui_rtb_dataRead.Text += CabelPort.ReadExisting();
                data += CabelPort.ReadExisting();
                ui_rtb_dataRead.Text = data;
                byte[] databyte;
                databyte = Encoding.ASCII.GetBytes(ui_rtb_dataRead.Text);
                File.WriteAllBytes(DateTime.Now.ToString("HH:mm:ss") + ".txt", databyte);
            }
            //CabelPort.WriteLine();


        }

        List<string> frames;
        private void updateFrameData(List<byte> data){
            string s = (BitConverter.ToInt16(data.ToArray(), 0) & 0X7FF0).ToString("X");
            frames.Add(s);

            /*if (!frames.Exists(s => s == )){
                frames.Add();
            }*/
        }

        private void ui_btn_readfile_Click(object sender, EventArgs e){
            frames = new List<string>();
            int nof7Fs = 0;
            byte[] databyte = File.ReadAllBytes("data01.txt");
            List<byte> realdata = new List<byte>();
            for (int i = 0; i < databyte.Length; i += 2){
                string fbyted = (databyte[i]).ToString("X2");
                realdata.Add(databyte[i]);
                if (fbyted == "7F"){
                    updateFrameData(realdata);
                    nof7Fs++;
                    realdata.Clear();
                    AppendText(this.ui_rtb_dataRead, Color.Red, Color.Yellow, fbyted);
                }else{
                    AppendText(this.ui_rtb_dataRead, Color.Black, Color.White, fbyted);
                }
                if (i + 1 < databyte.Length){
                    string sbyted = (databyte[i + 1]).ToString("X2");
                    realdata.Add(databyte[i + 1]);
                    if (sbyted == "7F"){
                        updateFrameData(realdata);
                        nof7Fs++;
                        realdata.Clear();
                        AppendText(this.ui_rtb_dataRead, Color.Red, Color.Yellow, sbyted + " ");
                    }else{
                        AppendText(this.ui_rtb_dataRead, Color.Black, Color.White, sbyted + " ");
                    }
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
