using System;
using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample27_CreateDerived : MonoBehaviour
    {
        // Usually in Unity, polymorphism is a rather annoying issue.
        // If you don't know what polymorphism is; http://en.wikipedia.org/wiki/Polymorphism_%28computer_science%29
        // Read up about it, it's VERY important!
        // To fix the issue Unity has with polymorphism, you can use the CreateDerived attribute with a base type that derive from ComponentMonoBehaviour;
        // This allows you to create instances directly from that field.
        [CreateDerived]
        public AIExample_BaseClass[] myField = null;

        [CreateDerived(true)]
        public AIExample_BaseClass[] myNoNameField = null;

        // In case you would like to add manually a ComponentMonoBehaviour.
        private void ManuallyAddComponent()
        {
            // It's a MonoBehaviour, so it has to be added the same way as any other.
            myField[0] = gameObject.AddComponent<AIExample_DerivedClassA>();
            // You must flag who is its owner, otherwise it will be handled as an orphant that his parent was destroyed.
            // An orphant is automatically removed by AI, to make sure you don't get invisible component you cannot access and clean.
            myField[0].Owner = this;
            // Note that a ComponentMonoBehaviour's owner can also be a ComponentMonoBehaviour, making it a chain which a non-ComponentMonoBehaviour resides at the top.
        }
    }
}