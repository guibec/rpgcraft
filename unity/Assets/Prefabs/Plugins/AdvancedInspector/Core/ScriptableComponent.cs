using System;
using System.Collections;
using System.Reflection;

using UnityEngine;

namespace AdvancedInspector
{
    public class ScriptableComponent : ScriptableObject
    {
        [SerializeField, HideInInspector]
        private ScriptableObject owner;

        /// <summary>
        /// The owner of a "subcomponent".
        /// Use to know if this component lost its parent.
        /// If so, the AdvancedInspector will delete any unused component.
        /// </summary>
        public ScriptableObject Owner
        {
            get { return owner; }
            set
            {
                if (value != null)
                    owner = value;
            }
        }

        /// <summary>
        /// A subcomponent is not visible the normal way in the Inspector.
        /// It's shown as being part of another item.
        /// </summary>
        protected virtual void Reset()
        {
            hideFlags = HideFlags.HideInInspector;
        }

        /// <summary>
        /// Called when the inspector is about to destroy this one.
        /// Loop in all the internal and destroy sub-components.
        /// </summary>
        public void Erase()
        {
            foreach (FieldInfo info in TypeUtility.GetFields(GetType()))
            { 
                object value = info.GetValue(this);

                if (value is ScriptableComponent)
                {
                    ScriptableComponent component = value as ScriptableComponent;

                    if (component.Owner == Owner)
                        component.Erase();
                }
            }

            DestroyImmediate(this, true);
        }

        /// <summary>
        /// Instanciate an existing Component on the same owner GameObject
        /// </summary>
        public ScriptableComponent Instantiate()
        {
            return Instantiate(Owner);
        }

        /// <summary>
        /// Instanciate an existing Component on the target GameObject.
        /// </summary>
        public ScriptableComponent Instantiate(ScriptableObject owner)
        {
            return CopyObject(owner, this) as ScriptableComponent;
        }

        private static object CopyObject(ScriptableObject owner, object original)
        {
            if (original == null)
                return null;

            Type type = original.GetType();

            if (type == typeof(string))
                return ((string)original).Clone();
            else if (type.Namespace == "System")
                return original;
            else if (typeof(IList).IsAssignableFrom(type))
                return CopyList(owner, (IList)original);
            else if (typeof(ScriptableComponent).IsAssignableFrom(type) && ((ScriptableComponent)original).Owner == owner)
                return CopyComponent(owner, (ScriptableComponent)original);
            else if (typeof(Component).IsAssignableFrom(type))
                return original;
            else if (typeof(ScriptableObject).IsAssignableFrom(type))
                return ScriptableObject.Instantiate((ScriptableObject)original);
            else if (typeof(UnityEngine.Object).IsAssignableFrom(type))
                return original;
            else if (type.IsClass)
                return CopyClass(owner, original);
            else
                return original;
        }

        private static IList CopyList(ScriptableObject owner, IList original)
        {
            Type type = original.GetType();
            IList copy;

            if (type.IsArray)
            {
                copy = Array.CreateInstance(type.GetElementType(), original.Count);
                for (int i = 0; i < original.Count; i++)
                    copy[i] = CopyObject(owner, original[i]);
            }
            else
            {
                copy = Activator.CreateInstance(type) as IList;
                for (int i = 0; i < original.Count; i++)
                    copy.Add(CopyObject(owner, original[i]));
            }

            return copy;
        }

        private static ScriptableComponent CopyComponent(ScriptableObject owner, ScriptableComponent original)
        {
            Type type = original.GetType();
            ScriptableComponent copy = original.Instantiate();

            foreach (FieldInfo info in TypeUtility.GetFields(type))
            {
                if (info.IsLiteral)
                    continue;

                info.SetValue(copy, CopyObject(copy, info.GetValue(original)));
            }

            copy.Owner = owner;

            return copy;
        }

        private static object CopyClass(ScriptableObject owner, object original)
        {
            Type type = original.GetType();
            object copy = Activator.CreateInstance(type, true);

            foreach (FieldInfo info in TypeUtility.GetFields(type))
            {
                if (info.IsLiteral)
                    continue;

                info.SetValue(copy, CopyObject(owner, info.GetValue(original)));
            }

            return copy;
        }
    }
}