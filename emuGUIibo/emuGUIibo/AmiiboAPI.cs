using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows.Forms;
using Newtonsoft.Json.Linq;

namespace emuGUIibo
{
    public class Amiibo
    {
        public string AmiiboName { get; set; }

        public string SeriesName { get; set; }

        public string CharacterName { get; set; }

        public string ImageURL { get; set; }

        public string AmiiboId { get; set; }
    }

    public static class AmiiboAPI
    {
        public static List<Amiibo> QueryAllAmiibos()
        {
            var list = new List<Amiibo>();
            try
            {
                var raw = new WebClient().DownloadString("https://www.amiiboapi.com/api/amiibo/");
                var json = JObject.Parse(raw);
                foreach (var amiibo in json["amiibo"])
                {
                    var name = amiibo["name"].ToString();
                    var series = amiibo["amiiboSeries"].ToString();
                    var character = amiibo["character"].ToString();
                    var image = amiibo["image"].ToString();
                    var idhead = amiibo["head"].ToString();
                    var idtail = amiibo["tail"].ToString();
                    list.Add(new Amiibo { AmiiboName = name, SeriesName = series, CharacterName = character, ImageURL = image, AmiiboId = idhead + idtail });
                }
                MessageBox.Show("Amiibo API was accessed. Amiibo list was loaded.");
            }
            catch
            {
                MessageBox.Show("Unable to download amiibo list from amiibo API.");
            }
            return list;
        }

        public static char MakeRandomHexChar(Random R)
        {
            string hexchars = "0123456789ABCDEF";
            int randidx = R.Next(0, hexchars.Length - 1);
            return hexchars[randidx];
        }

        public static string MakeRandomHexString(int Length)
        {
            string hex = "";
            Random r = new Random();
            for(int i = 0; i < Length; i++)
            {
                hex += MakeRandomHexChar(r);
            }
            return hex;
        }
    }
}
