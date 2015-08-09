using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

    public struct ItemCount
    {
        public ItemCount(EItem item_, int count_) : this()
        {
            Item = item_;
            Count = count_;
        }

        public EItem Item { get; set; }
        public int Count { get; set; }
    }
