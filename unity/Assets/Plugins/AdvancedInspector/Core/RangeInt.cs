using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AdvancedInspector
{
    [Serializable]
    public struct RangeInt
    {
        public int min;
        public int max;

        public RangeInt(int min, int max)
        {
            this.min = min;
            this.max = max;
        }
    }
}