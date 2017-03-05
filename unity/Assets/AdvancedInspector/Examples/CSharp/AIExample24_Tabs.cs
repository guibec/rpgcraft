using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample24_Tabs : MonoBehaviour
    {
        // Tabs work in a manner similar to groups.
        // However, only the fields of a single tab is display at the same time.
        [Tab(TabEnum.First)]
        public float myFirstField;

        [Inspect, Tab(TabEnum.First)]
        public float MyFirstProperty
        {
            get { return myFirstField; }
            set { myFirstField = value; }
        }

        [Tab(TabEnum.Second)]
        public float mySecondField;

        [Inspect, Tab(TabEnum.Second)]
        public float MySecondProperty
        {
            get { return mySecondField; }
            set { mySecondField = value; }
        }

        // Field that are not bound to a tab are displayed after the currently selected tab.
        public float myThirdField;

        public enum TabEnum
        {
            [Descriptor("TabStatic.GetTabName")]
            First,
            Second,
            Third
        }
    }

    public class TabStatic
    {
        public static Description GetTabName(DescriptorAttribute attribute, object instance, object value)
        {
            if (instance is AIExample24_Tabs)
                return new Description("Tabs!");
            else
                return new Description("Not Tabs!");
        }
    }
}