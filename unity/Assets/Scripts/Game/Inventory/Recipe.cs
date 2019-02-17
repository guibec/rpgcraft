using System.Collections.Generic;

class Recipe
    {
        public List<ItemCount> Sources
        {
            get => m_sources;
            set => m_sources = value;
        }

        public ItemCount Destination { get; set; }
        private List<ItemCount> m_sources = new List<ItemCount>();
    }
