public struct ItemCount
    {
        public ItemCount(ETile item_, int count_) : this()
        {
            Item = item_;
            Count = count_;
        }

        public ETile Item { get; set; }
        public int Count { get; set; }
    }
