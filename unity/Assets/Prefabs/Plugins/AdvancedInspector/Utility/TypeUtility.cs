using System;
using System.Collections.Generic;
using System.Reflection;

namespace AdvancedInspector
{
    public class TypeUtility
    {
#if !NETFX_CORE
        public static Type GetTypeByName(string name)
        {
            foreach (Assembly assembly in AppDomain.CurrentDomain.GetAssemblies())
            {
                foreach (Type type in assembly.GetTypes())
                {
                    if (type.Name == name)
                        return type;
                }
            }

            return null;
        }
#endif

        public static List<FieldInfo> GetFields(Type type)
        {
            List<FieldInfo> infos = new List<FieldInfo>();

#if !NETFX_CORE
            while (type != null)
            {
                infos.AddRange(type.GetFields(BindingFlags.Instance | BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.DeclaredOnly));

                bool ignoreBase = type.GetCustomAttributes(typeof(IgnoreBase), true).Length > 0;
                if (!ignoreBase && type.BaseType != null && type.BaseType != typeof(object) && type.BaseType != typeof(UnityEngine.Object))
                    type = type.BaseType;
                else
                    break;
            }
#endif

            return infos;
        }
    }
}
