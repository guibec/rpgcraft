using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

namespace AdvancedInspector
{
    /// <summary>
    /// ScriptableObject are not serializable within the scope of a GameObject.
    /// Therefore, they are improper to prefab, copy and so on.
    /// This class' goal is to provide a polymorphic solution and give an easy way of handling up front.
    /// 
    /// Derived from Object; no polymorphism supported.
    /// Derived from ScriptableObject; cannot be prefabed, copied, duplicated or instanced.
    /// Derived from MonoBehaviour; can only live on a GameObject.
    /// 
    /// A ComponentMonoBehaviour is created from another MonoBehaviour; its parent.
    /// A parent can be another ComponentMonoBehaviour.
    /// If the parent is destroyed, the subcomponent is destroyed too.
    /// If a subcomponent is found without a parent, it's destroyed too.
    /// </summary>
    public abstract class ComponentMonoBehaviour : MonoBehaviour
    {
        [SerializeField, HideInInspector]
        private MonoBehaviour owner;

        /// <summary>
        /// The owner of a "subcomponent".
        /// Use to know if this component lost its parent.
        /// If so, the AdvancedInspector will delete any unused component.
        /// </summary>
        public MonoBehaviour Owner
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

                if (value is ComponentMonoBehaviour)
                {
                    ComponentMonoBehaviour component = value as ComponentMonoBehaviour;

                    if (component.Owner == this)
                        component.Erase();
                }
            }

            DestroyImmediate(this, true);
        }

        /// <summary>
        /// Instanciate an existing Component on the same owner GameObject
        /// </summary>
        public ComponentMonoBehaviour Instantiate()
        {
            return Instantiate(gameObject, owner);
        }

        /// <summary>
        /// Instanciate an existing Component on the same owner GameObject but with a new onwer.
        /// </summary>
        public ComponentMonoBehaviour Instantiate(MonoBehaviour owner)
        {
            return Instantiate(gameObject, owner);
        }

        /// <summary>
        /// Instanciate an existing Component on the target GameObject.
        /// </summary>
        public ComponentMonoBehaviour Instantiate(GameObject go, MonoBehaviour owner)
        {
            return CopyObject(go, owner, this) as ComponentMonoBehaviour;
        }

        private static object CopyObject(GameObject go, MonoBehaviour owner, object original)
        {
            if (original == null)
                return null;

            Type type = original.GetType();

            if (type == typeof(string))
#if !NETFX_CORE
                return ((string)original).Clone();
#else
                return original;
#endif
            else if (type.Namespace == "System")
                return original;
            else if (typeof(IList).IsAssignableFrom(type))
                return CopyList(go, owner, (IList)original);
            else if (typeof(ComponentMonoBehaviour).IsAssignableFrom(type) && ((ComponentMonoBehaviour)original).Owner == owner)
                return CopyComponent(go, owner, (ComponentMonoBehaviour)original);
            else if (typeof(Component).IsAssignableFrom(type))
                return original;
            else if (typeof(ScriptableObject).IsAssignableFrom(type))
                return ScriptableObject.Instantiate((ScriptableObject)original);
            else if (typeof(UnityEngine.Object).IsAssignableFrom(type))
                return original;
#if !NETFX_CORE
            else if (type.IsClass)
                return CopyClass(go, owner, original);
            else
                return original;
#else
            else
                return CopyClass(go, owner, original);
#endif
        }

        private static IList CopyList(GameObject go, MonoBehaviour owner, IList original)
        {
            Type type = original.GetType();
            IList copy;

            if (type.IsArray)
            {
                copy = Array.CreateInstance(type.GetElementType(), original.Count);
                for (int i = 0; i < original.Count; i++)
                    copy[i] = CopyObject(go, owner, original[i]);
            }
            else
            {
                copy = Activator.CreateInstance(type) as IList;
                for (int i = 0; i < original.Count; i++)
                    copy.Add(CopyObject(go, owner, original[i]));

            }

            return copy;
        }

        private static ComponentMonoBehaviour CopyComponent(GameObject go, MonoBehaviour owner, ComponentMonoBehaviour original)
        {
            Type type = original.GetType();
            ComponentMonoBehaviour copy = go.AddComponent(type) as ComponentMonoBehaviour;

            foreach (FieldInfo info in TypeUtility.GetFields(type))
            {
                if (info.IsLiteral)
                    continue;

                info.SetValue(copy, CopyObject(go, copy, info.GetValue(original)));
            }

            copy.Owner = owner;

            return copy;
        }

        private static object CopyClass(GameObject go, MonoBehaviour owner, object original)
        {
            Type type = original.GetType();
            object copy = Activator.CreateInstance(type, true);

            foreach (FieldInfo info in TypeUtility.GetFields(type))
            {
                if (info.IsLiteral)
                    continue;

                info.SetValue(copy, CopyObject(go, owner, info.GetValue(original)));
            }

            return copy;
        }
    }
}