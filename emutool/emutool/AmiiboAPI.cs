using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.IO;

namespace emutool
{
    public static class AmiiboAPI
    {
        public class Amiibo
        {
            public string AmiiboName { get; set; }

            public string SeriesName { get; set; }

            public string CharacterName { get; set; }

            public string ImageURL { get; set; }

            public string AmiiboId { get; set; }
        }

        public class AmiiboList
        {
            public List<Amiibo> Amiibos { get; set; }

            public AmiiboList()
            {
                Amiibos = new List<Amiibo>();
            }

            public int GetAmiiboCount()
            {
                return Amiibos.Count;
            }

            public List<Amiibo> GetAmiibosBySeries(string series)
            {
                var list = new List<Amiibo>();
                if(Amiibos.Any())
                {
                    list.AddRange(Amiibos.Where(amiibo => amiibo.SeriesName == series));
                }
                return list;
            }

            public List<string> GetAmiiboSeries()
            {
                var list = new List<string>();
                if(Amiibos.Any())
                {
                    foreach(var amiibo in Amiibos)
                    {
                        var series = amiibo.SeriesName;
                        if(!list.Contains(series))
                        {
                            list.Add(series);
                        }
                    }
                }
                return list;
            }
        };

        private const string AmiiboAPIURL = "https://www.amiiboapi.com/api/amiibo/";

        public static AmiiboList LoadedAmiibos = null;

        public static bool LoadedLocal = false;

        public static bool HasLoadedAmiibos => LoadedAmiibos != null;

        public static string AmiiboAPIJSONPath => Path.Combine(Environment.CurrentDirectory, "amiibo_api.json");

        public static string ValidateAmiiboName(string name)
        {
            // Avoid amiibo names conflicting with system paths
            return Utils.TruncateString(name.Replace('/', '_'), 10);
        }

        private static AmiiboList ParseAmiiboAPIJSON(JObject json)
        {
            var list = new AmiiboList();
            foreach(var entry in json["amiibo"])
            {
                var amiibo = new Amiibo
                {
                    AmiiboName = entry["name"].ToString(),
                    SeriesName = entry["amiiboSeries"].ToString(),
                    CharacterName = entry["character"].ToString(),
                    ImageURL = entry["image"].ToString(),
                    AmiiboId = entry["head"].ToString() + entry["tail"].ToString(),
                };
                list.Amiibos.Add(amiibo);
            }
            return list;
        }

        public static AmiiboList TryGetAllAmiibosRemote()
        {
            try
            {
                var json_str = Utils.GetFromURL(AmiiboAPIURL);
                File.WriteAllText(AmiiboAPIJSONPath, json_str);
                var json = JObject.Parse(json_str);
                return ParseAmiiboAPIJSON(json);
            }
            catch(Exception ex)
            {
                Utils.LogExceptionMessage(ex);
            }
            return null;
        }

        public static AmiiboList TryGetAllAmiibosLocal() {
            try
            {
                var json_str = File.ReadAllText(AmiiboAPIJSONPath);
                var json = JObject.Parse(json_str);
                return ParseAmiiboAPIJSON(json);
            }
            catch(Exception ex)
            {
                Utils.LogExceptionMessage(ex);
            }
            return null;
        }

        public static void LoadAllAmiibos()
        {
            LoadedLocal = false;
            LoadedAmiibos = TryGetAllAmiibosRemote();
            if(LoadedAmiibos == null)
            {
                LoadedAmiibos = TryGetAllAmiibosLocal();
                if(LoadedAmiibos != null)
                {
                    LoadedLocal = true;
                }
            }
        }
    }
}