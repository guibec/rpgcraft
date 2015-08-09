using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

    class Recipe
    {
        public List<ItemCount> Sources
        {
            get
            {
                return m_sources;
            }
            set
            {
                m_sources = value;
            }
        }

        public ItemCount Destination { get; set; }
        private List<ItemCount> m_sources = new List<ItemCount>();
    }
