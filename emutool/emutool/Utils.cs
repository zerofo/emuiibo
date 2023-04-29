using System;
using System.Net;
using System.IO;
using System.Linq;
using System.Text;
using System.Globalization;
using System.Windows.Forms;
using System.Runtime.CompilerServices;
using Newtonsoft.Json.Linq;

namespace emutool
{
    public static class Utils
    {
        public static Random Rng = new Random();

        public static void RecreateDirectory(string dir)
        {
            if(Directory.Exists(dir))
            {
                Directory.Delete(dir, true);
            }
            Directory.CreateDirectory(dir);
        }
        public static void CreateEmptyFile(string path)
        {
            File.Create(path).Dispose();
        }

        public static void SaveJSON(string path, JObject json)
        {
            File.WriteAllText(path, json.ToString());
        }

        public static void PrepareNet()
        {
            ServicePointManager.Expect100Continue = true;
            ServicePointManager.SecurityProtocol = SecurityProtocolType.Tls12;
        }

        public static string GetFromURL(string url)
        {
            PrepareNet();
            using(var web = new WebClient())
            {
                return web.DownloadString(url);
            }
        }
        
        public static void SaveFromURL(string url, string path)
        {
            PrepareNet();
            using(var web = new WebClient())
            {
                web.DownloadFile(url, path);
            }
        }

        public static void LogExceptionMessage(Exception ex, [CallerMemberName] string fn_name = "<unknown>")
        {
            MessageBox.Show("Caught exception (" + ex.GetType().Name + ") at " + fn_name + ": " + ex.ToString(), "Caught exception!");
            Environment.Exit(1);
        }

        public static void Unless(bool cond, string message)
        {
            if(!cond)
            {
                throw new Exception(message);
            }
        }

        public static ushort ReverseUInt16(ushort val)
        {
            var bytes = BitConverter.GetBytes(val);
            Array.Reverse(bytes);
            return BitConverter.ToUInt16(bytes, 0);
        }

        public static string RemoveAccents(string input)
        {
            return new string(input.Normalize(NormalizationForm.FormD).ToCharArray().Where(c => CharUnicodeInfo.GetUnicodeCategory(c) != UnicodeCategory.NonSpacingMark).ToArray());
        }
    }
}
