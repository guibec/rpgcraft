namespace AdvancedInspector
{
    // You don't need [AdvancedInspector] if the parent class has it.
    public class AIExample_DerivedClassC : AIExample_BaseClass
    {
        [Inspect]
        public string myDerivedStringC;

        [Inspect, CreateDerived]
        public AIExample_BaseClass subDerived;
    }
}