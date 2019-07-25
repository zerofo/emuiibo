using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.IO;
using Newtonsoft.Json.Linq;
using System.Windows.Forms;

namespace emuGUIibo
{
    public partial class Form1 : Form
    {
        public static List<Amiibo> QueryAmiibos;

        public Form1()
        {
            InitializeComponent();
            QueryAmiibos = AmiiboAPI.QueryAllAmiibos();
            if(QueryAmiibos.Any())
            {
                foreach(var amiibo in QueryAmiibos)
                {
                    comboBox1.Items.Add(amiibo.AmiiboName + " (" + amiibo.SeriesName + ")");
                }
                comboBox1.SelectedIndex = 0;
            }
        }

        private void ComboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            pictureBox1.ImageLocation = QueryAmiibos[comboBox1.SelectedIndex].ImageURL;
        }

        private void Button1_Click(object sender, EventArgs e)
        {
            if(string.IsNullOrEmpty(textBox1.Text))
            {
                MessageBox.Show("No amiibo name was specified.");
                return;
            }
            string dir = "";
            var drives = DriveInfo.GetDrives();
            if(drives.Any())
            {
                foreach(var drive in drives)
                {
                    if(drive.IsReady)
                    {
                        if(Directory.Exists(drive.Name + "emuiibo\\amiibo")) dir = drive.Name + "emuiibo\\amiibo";
                        else if(Directory.Exists(drive.Name + "emuiibo"))
                        {
                            Directory.CreateDirectory(drive.Name + "emuiibo\\amiibo");
                            dir = drive.Name + "emuiibo\\amiibo";
                        }
                        if(!string.IsNullOrEmpty(dir)) MessageBox.Show("Emuiibo directory was found in drive '" + drive.VolumeLabel + "', so defaulting to that directory.");
                    }
                }
            }
            var fbd = new FolderBrowserDialog
            {
                Description = "Select root directory to generate the virtual amiibo on",
                ShowNewFolderButton = false,
                SelectedPath = dir
            };
            if(fbd.ShowDialog() == DialogResult.OK)
            {
                string amiibodir = fbd.SelectedPath + "\\" + textBox1.Text;
                if(MessageBox.Show("Virtual amiibo will be created in '" + amiibodir + "'.\nThe directory will be deleted if it already exists.\n\nProceed with amiibo creation?", "Amiibo creation", MessageBoxButtons.OKCancel) != DialogResult.OK) return;
                if(Directory.Exists(amiibodir)) Directory.Delete(amiibodir, true);
                try
                {
                    Directory.CreateDirectory(amiibodir);
                    var tag = new JObject();
                    if (checkBox1.Checked) tag["randomUuid"] = true;
                    else tag["uuid"] = AmiiboAPI.MakeRandomHexString(18);
                    File.WriteAllText(amiibodir + "\\tag.json", tag.ToString());
                    var model = new JObject()
                    {
                        ["amiiboId"] = QueryAmiibos[comboBox1.SelectedIndex].AmiiboId
                    };
                    File.WriteAllText(amiibodir + "\\model.json", model.ToString());
                    var datestr = DateTime.Now.ToString("yyyy-MM-dd");
                    var register = new JObject()
                    {
                        ["name"] = textBox1.Text,
                        ["firstWriteDate"] = datestr,
                        ["miiCharInfo"] = "mii-charinfo.bin"
                    };
                    File.WriteAllText(amiibodir + "\\register.json", register.ToString());
                    var common = new JObject()
                    {
                        ["lastWriteDate"] = datestr,
                        ["writeCounter"] = 0,
                        ["version"] = 0
                    };
                    File.WriteAllText(amiibodir + "\\common.json", common.ToString());
                    MessageBox.Show("Virtual amiibo was successfully created.");
                }
                catch
                {
                    MessageBox.Show("An error ocurred attempting to create the virtual amiibo.");
                }
            }
        }
    }
}
