using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample46_TabsDynamic : AIExample46_TabsDynamic_Base
    {
        protected override Enum GetFirstTabs()
        {
            return DerivedTabs.DerivedFirst;
        }

        protected override Enum GetSecondTabs()
        {
            return DerivedTabs.DerivedSecond;
        }

        protected override Enum GetThirdTabs()
        {
            return DerivedTabs.DerivedThird;
        }

        private enum DerivedTabs
        {
            DerivedFirst,
            DerivedSecond,
            DerivedThird
        }
    }

    public abstract class AIExample46_TabsDynamic_Base : MonoBehaviour
    {
        [Tab("GetFirstTabs")]
        public float firstValue;

        [Tab("GetSecondTabs")]
        public float secondValue;

        [Tab("GetThirdTabs")]
        public float thirdValue;

        public float noTabValue;

        protected virtual Enum GetFirstTabs()
        {
            return BaseTabs.First;
        }

        protected virtual Enum GetSecondTabs()
        {
            return BaseTabs.Second;
        }

        protected virtual Enum GetThirdTabs()
        {
            return BaseTabs.Third;
        }

        private enum BaseTabs
        {
            First,
            Second,
            Third
        }
    }
}
