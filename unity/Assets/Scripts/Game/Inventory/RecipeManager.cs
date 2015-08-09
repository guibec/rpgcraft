using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Xml;
using System.Xml.Linq;

    public class RecipeManager
    {
        private static readonly RecipeManager instance = new RecipeManager();
        public static RecipeManager Instance
        {
            get
            {
                return instance;
            }
        }

        private RecipeManager()
        {

        }

        /************************************************************************/
        /*  <?xml version="1.0" ?> 
            <recipes>
	            <recipe>
		            <source item="Wood" count="4"/>
		            <dest item="Stick" count="1"/>
	            </recipe>
            </recipes>                                                                     
        /************************************************************************/
        public bool Load(string file="./data/items/recipes.xml")
        {
        //    XDocument doc;

        //    try
        //    {
        //        doc = XDocument.Load(file);
        //    }
        //    catch (System.Exception ex)
        //    {
        //        UnityEngine.Debug.LogException(ex);
        //        return false;
        //    }

        //    IEnumerable<XElement> recipes = (from xml2 in doc.Elements("recipes").Elements("recipe")
        //                        select xml2);

        //    foreach (var recipe in recipes)
        //    {
        //        Recipe newRecipe = new Recipe();
        //        bool success;

        //        IEnumerable<XElement> sources = (from xml2 in recipes.Elements("source") select xml2);
        //        foreach (var source in sources)
        //        {
        //            //XAttribute sourceItem = (from xml2 in source.Attributes("item") select xml2).FirstOrDefault();
        //            XAttribute sourceCount = (from xml2 in source.Attributes("count") select xml2).FirstOrDefault();

        //            int numSource;
        //            success = Int32.TryParse(sourceCount.Value, out numSource);
        //            if (!success)
        //            {
        //                UnityEngine.Debug.LogError("Invalid recipe item count (source) " + sourceCount.Value);
        //                return false;
        //            }

        //            //newRecipe.Sources.Add(new ItemCount(sourceItem.Value, numSource));
        //        }


        //        //XAttribute destItem = (from xml2 in recipes.Elements("dest").Attributes("item") select xml2).FirstOrDefault();
        //        XAttribute destCount = (from xml2 in recipes.Elements("dest").Attributes("count") select xml2).FirstOrDefault();

        //        int numDest;
        //        success = Int32.TryParse(destCount.Value, out numDest);
        //        if (!success)
        //        {
        //            UnityEngine.Debug.LogError("Invalid recipe item count (dest)" + destCount.Value);
        //            return false;
        //        }

        //        //newRecipe.Destination = new ItemCount(destItem.Value, numDest);
        //    }

            return true;
        }
    }
