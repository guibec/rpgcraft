using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;

using UnityEngine;

namespace AdvancedInspector
{
    /// <summary>
    /// This class represent an event, and allows the Inspector to bind it to another method.
    /// Arguments can be sent to the invoked method from a static field, internal from the code invoking, or from another method returned value.
    /// </summary>
    [Serializable, AdvancedInspector]
    public class ActionBinding : ICopiable
    {
        [SerializeField]
        private string[] internalParameters = new string[0];

        [SerializeField]
        private GameObject gameObject;

        [Inspect]
        public GameObject GameObject
        {
            get { return gameObject; }
            set
            {
                if (gameObject != value)
                {
                    gameObject = value;
                    Component = null;
                }
            }
        }

        [SerializeField]
        private Component component;

        [Inspect, Restrict("GetComponents")]
        public Component Component
        {
            get { return component; }
            set
            {
                if (component != value)
                {
                    component = value;
                    Method = null;
                }
            }
        }

        private IList GetComponents()
        {
            List<DescriptorPair> components = new List<DescriptorPair>();
            if (gameObject == null)
                return components;

            foreach (Component component in gameObject.GetComponents(typeof(Component)))
                components.Add(new DescriptorPair(component, new DescriptorAttribute(component.GetType().Name, "")));

            return components;
        }

        [SerializeField]
        private string method;

        [Inspect, Restrict("GetMethods", RestrictDisplay.Toolbox)]
        public MethodInfo Method
        {
            get { return GetMethodInfo(); }
            set
            {
                if (value == null)
                {
                    parameters = new BindingParameter[0];
                    method = "";
                    return;
                }

                MethodInfo info = GetMethodInfo();
                if (info != value)
                {
                    method = value.Name;
                    ParameterInfo[] param = value.GetParameters();
                    parameters = new BindingParameter[param.Length];
                    for (int i = 0; i < param.Length; i++)
                    {
                        parameters[i] = new BindingParameter(internalParameters.Length > i);
                        parameters[i].Type = param[i].ParameterType;
                        if (internalParameters.Length > i)
                            parameters[i].binding = BindingParameter.BindingType.Internal;
                    }
                }
            }
        }

        private IList GetMethods()
        {
            List<DescriptorPair> methods = new List<DescriptorPair>();
            if (gameObject == null || component == null)
                return methods;

            foreach (MethodInfo info in component.GetType().GetMethods(BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.FlattenHierarchy))
            {
                if (info.IsGenericMethod || info.IsConstructor || info.IsFinal || info.IsSpecialName || info.DeclaringType == typeof(object) || info.DeclaringType == typeof(Component))
                    continue;

                if (!IsMethodValid(info))
                    continue;

                ParameterInfo[] param = info.GetParameters();

                methods.Add(new DescriptorPair(info, new DescriptorAttribute(GetParamNames(info.Name, param), "")));
            }

            foreach (PropertyInfo info in component.GetType().GetProperties(BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.FlattenHierarchy))
            {
                if (!info.CanWrite || info.DeclaringType == typeof(object) || info.DeclaringType == typeof(UnityEngine.Object) || info.DeclaringType == typeof(Component))
                    continue;

                MethodInfo method = info.GetSetMethod();
                if (method == null || !IsMethodValid(method))
                    continue;

                ParameterInfo[] param = method.GetParameters();

                methods.Add(new DescriptorPair(method, new DescriptorAttribute(GetParamNames(info.Name, param), "")));
            }

            return methods;
        }

        private bool IsMethodValid(MethodInfo info)
        {
            ParameterInfo[] param = info.GetParameters();

            bool valid = true;
            for (int i = 0; i < param.Length; i++)
            {
                if (!BindingParameter.IsValidType(param[i].ParameterType))
                {
                    valid = false;
                    break;
                }

                if (internalParameters.Length > i)
                {
                    Type type = Type.GetType(internalParameters[i]);
                    if (!type.IsAssignableFrom(param[i].ParameterType))
                    {
                        valid = false;
                        break;
                    }
                }
            }

            return valid;
        }

        private string GetParamNames(string name, ParameterInfo[] param)
        {
            string paramName = name + " (";
            for (int i = 0; i < param.Length; i++)
            {
                paramName += param[i].ParameterType.Name;
                if (i < param.Length - 1)
                    paramName += ", ";
            }
            paramName += ")";

            return paramName;
        }

        private MethodInfo GetMethodInfo()
        {
            if (gameObject == null || component == null || string.IsNullOrEmpty(method))
                return null;

            Type[] types = new Type[parameters.Length];
            for (int i = 0; i < parameters.Length; i++)
                types[i] = parameters[i].Type;

            return component.GetType().GetMethod(method, BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.FlattenHierarchy, null, types, null);
        }

        [Inspect, Collection(0, false), SerializeField]
        private BindingParameter[] parameters = new BindingParameter[0];

        public event ActionEventHandler OnInvoke;

        /// <summary>
        /// Parameter-less method constructor.
        /// </summary>
        public ActionBinding() { }

        /// <summary>
        /// The array of types is the method definition.
        /// Method with the wrong parameters are not selectable.
        /// </summary>
        public ActionBinding(Type[] types)
        {
            internalParameters = new string[types.Length];
            for (int i = 0; i < types.Length; i++)
                internalParameters[i] = types[i].AssemblyQualifiedName;
        }

        /// <summary>
        /// Invoke the method.
        /// Be careful to pass the proper type as args, otherwise they will be ignored.
        /// Args are only retained if the parameter is flagged as internal.
        /// </summary>
        public void Invoke(params object[] args)
        {
            if (gameObject == null || component == null || string.IsNullOrEmpty(method))
                return;

            MethodInfo info = GetMethodInfo();
            if (info == null)
                return;

            object[] values = new object[parameters.Length];
            for (int i = 0; i < parameters.Length; i++)
            {
                if (args.Length > i && parameters[i].binding == BindingParameter.BindingType.Internal)
                    values[i] = args[i];
                else
                    values[i] = parameters[i].Value;
            }

            info.Invoke(component, values);

            if (OnInvoke != null)
                OnInvoke(this, values);
        }

        public override string ToString()
        {
            string supplied = "(";
            for (int i = 0; i < internalParameters.Length; i++)
            {
                supplied += Type.GetType(internalParameters[i]).Name;
                if (i < internalParameters.Length - 1)
                    supplied += ", ";
            }

            supplied += ")";

            if (component != null)
                return component.GetType().Name + " : " + method + " : " + supplied;
            else
                return supplied;
        }

        public bool Copiable(object destination)
        {
            ActionBinding action = destination as ActionBinding;
            if (action == null)
                return false;

            if (action.internalParameters.Length != internalParameters.Length)
                return false;

            for (int i = 0; i < internalParameters.Length; i++)
                if (internalParameters[i] != action.internalParameters[i])
                    return false;

            return true;
        }

        /// <summary>
        /// The binding parameter define how each of the argument of the invoked method is handled.
        /// </summary>
        [Serializable, AdvancedInspector]
        public class BindingParameter : ICopy, ICopiable
        {
            [Inspect(-1), Restrict("RestrictBinding")]
            public BindingType binding;

            private IList RestrictBinding()
            {
                List<object> list = new List<object>();
                if (canBeInternal)
                    list.Add(BindingType.Internal);

                list.Add(BindingType.Static);
                list.Add(BindingType.External);
                return list;
            }

            private bool canBeInternal = true;

            private bool CanBeInternal
            {
                get { return canBeInternal; }
            }

            #region Values
            [SerializeField]
            private BindingValueType type;

            [SerializeField]
            private string qualifiedTypeName;

            public Type Type
            {
                get
                {
                    if (string.IsNullOrEmpty(qualifiedTypeName))
                        return null;
                    else
                        return Type.GetType(qualifiedTypeName);
                }

                set
                {
                    if (value == typeof(bool))
                        type = BindingValueType.Boolean;
                    else if (value == typeof(Bounds))
                        type = BindingValueType.Bounds;
                    else if (value == typeof(Color))
                        type = BindingValueType.Color;
                    else if (value == typeof(float))
                        type = BindingValueType.Float;
                    else if (value == typeof(int))
                        type = BindingValueType.Integer;
                    else if (value == typeof(Rect))
                        type = BindingValueType.Rect;
                    else if (typeof(UnityEngine.Object).IsAssignableFrom(value))
                        type = BindingValueType.Reference;
                    else if (value == typeof(string))
                        type = BindingValueType.String;
                    else if (value == typeof(Vector2))
                        type = BindingValueType.Vector2;
                    else if (value == typeof(Vector3))
                        type = BindingValueType.Vector3;
                    else if (value == typeof(Vector4))
                        type = BindingValueType.Vector4;
                    else
                        type = BindingValueType.None;

                    if (type != BindingValueType.None)
                        qualifiedTypeName = value.AssemblyQualifiedName;
                    else
                        qualifiedTypeName = "";
                }
            }

            [Inspect(-2)]
            public string BoundType
            {
                get
                {
                    if (Type == null)
                        return "";

                    return Type.Name;
                }
            }

            [SerializeField]
            private bool boolValue = false;
            [SerializeField]
            private int intValue = 0;
            [SerializeField]
            private float floatValue = 0;
            [SerializeField]
            private string stringValue = "";
            [SerializeField]
            private Vector2 vector2Value = Vector2.zero;
            [SerializeField]
            private Vector3 vector3Value = Vector3.zero;
            [SerializeField]
            private Vector4 vector4Value = Vector4.zero;
            [SerializeField]
            private Color colorValue = Color.black;
            [SerializeField]
            private Rect rectValue = new Rect(0, 0, 0, 0);
            [SerializeField]
            private Bounds boundsValue = new Bounds(Vector3.zero, Vector3.zero);
            [SerializeField]
            private UnityEngine.Object referenceValue = new UnityEngine.Object();
            #endregion

            [Inspect("IsStatic")]
            [RuntimeResolve("GetRuntimeType")]
            public object Value
            {
                get
                {
                    if (binding == BindingType.External)
                    {
                        object value = Invoke();
                        if (value.GetType().IsAssignableFrom(Type))
                            return value;

                        System.ComponentModel.TypeConverter converter = System.ComponentModel.TypeDescriptor.GetConverter(Type);
                        return converter.ConvertTo(value, Type);
                    }

                    switch (type)
                    {
                        case BindingValueType.Boolean:
                            return boolValue;
                        case BindingValueType.Bounds:
                            return boundsValue;
                        case BindingValueType.Color:
                            return colorValue;
                        case BindingValueType.Float:
                            return floatValue;
                        case BindingValueType.Integer:
                            return intValue;
                        case BindingValueType.Rect:
                            return rectValue;
                        case BindingValueType.Reference:
                            return referenceValue;
                        case BindingValueType.String:
                            return stringValue;
                        case BindingValueType.Vector2:
                            return vector2Value;
                        case BindingValueType.Vector3:
                            return vector3Value;
                        case BindingValueType.Vector4:
                            return vector4Value;
                        default:
                            return null;
                    }
                }

                set
                {
                    if (value == null && type != BindingValueType.Reference)
                        return;

                    switch (type)
                    {
                        case BindingValueType.Boolean:
                            boolValue = (bool)value;
                            break;
                        case BindingValueType.Bounds:
                            boundsValue = (Bounds)value;
                            break;
                        case BindingValueType.Color:
                            colorValue = (Color)value;
                            break;
                        case BindingValueType.Float:
                            floatValue = (float)value;
                            break;
                        case BindingValueType.Integer:
                            intValue = (int)value;
                            break;
                        case BindingValueType.Rect:
                            rectValue = (Rect)value;
                            break;
                        case BindingValueType.Reference:
                            referenceValue = (UnityEngine.Object)value;
                            break;
                        case BindingValueType.String:
                            stringValue = (string)value;
                            break;
                        case BindingValueType.Vector2:
                            vector2Value = (Vector2)value;
                            break;
                        case BindingValueType.Vector3:
                            vector3Value = (Vector3)value;
                            break;
                        case BindingValueType.Vector4:
                            vector4Value = (Vector4)value;
                            break;
                        default:
                            return;
                    }
                }
            }

            [SerializeField]
            private GameObject gameObject;

            [Inspect("IsExternal")]
            public GameObject GameObject
            {
                get { return gameObject; }
                set
                {
                    if (gameObject != value)
                    {
                        gameObject = value;
                        Component = null;
                    }
                }
            }

            [SerializeField]
            private Component component;

            [Inspect("IsExternal")]
            [Restrict("GetComponents")]
            public Component Component
            {
                get { return component; }
                set
                {
                    if (component != value)
                    {
                        component = value;
                        Method = null;
                    }
                }
            }

            private IList GetComponents()
            {
                List<DescriptorPair> components = new List<DescriptorPair>();
                if (gameObject == null)
                    return components;

                foreach (Component component in gameObject.GetComponents(typeof(Component)))
                    components.Add(new DescriptorPair(component, new DescriptorAttribute(component.GetType().Name, "")));

                return components;
            }

            [SerializeField]
            private string method;

            [Inspect("IsExternal")]
            [Restrict("GetMethods", RestrictDisplay.Toolbox)]
            public MethodInfo Method
            {
                get { return GetMethodInfo(); }
                set
                {
                    if (value == null)
                    {
                        method = "";
                        return;
                    }

                    MethodInfo info = GetMethodInfo();
                    if (info != value)
                        method = value.Name;
                }
            }

            private IList GetMethods()
            {
                List<DescriptorPair> methods = new List<DescriptorPair>();
                if (gameObject == null || component == null)
                    return methods;

                foreach (MethodInfo info in component.GetType().GetMethods(BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.FlattenHierarchy))
                {
                    if (info.IsGenericMethod || info.IsConstructor || info.IsFinal || info.IsSpecialName)
                        continue;

                    if (!IsMethodValid(info))
                        continue;

                    string paramName = info.ReturnType.Name + " " + info.Name + "()";
                    methods.Add(new DescriptorPair(info, new DescriptorAttribute(paramName, "")));
                }

                foreach (PropertyInfo info in component.GetType().GetProperties(BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.FlattenHierarchy))
                {
                    if (!info.CanRead)
                        continue;

                    MethodInfo method = info.GetGetMethod();
                    if (method == null || !IsMethodValid(method))
                        continue;

                    string paramName = method.ReturnType.Name + " " + info.Name + "()";
                    methods.Add(new DescriptorPair(method, new DescriptorAttribute(paramName, "")));
                }

                return methods;
            }

            private bool IsMethodValid(MethodInfo info)
            {
                ParameterInfo[] param = info.GetParameters();
                if (param.Length > 0)
                    return false;

                if (info.ReturnType == null || info.ReturnType == typeof(void))
                    return false;

                System.ComponentModel.TypeConverter converter = System.ComponentModel.TypeDescriptor.GetConverter(info.ReturnType);
                if (!Type.IsAssignableFrom(info.ReturnType) && !converter.CanConvertTo(Type))
                    return false;

                return true;
            }

            private MethodInfo GetMethodInfo()
            {
                if (gameObject == null || component == null || string.IsNullOrEmpty(method))
                    return null;

                return component.GetType().GetMethod(method, BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.FlattenHierarchy);
            }

            private bool IsStatic()
            {
                return binding == BindingType.Static;
            }

            private bool IsExternal()
            {
                return binding == BindingType.External;
            }

            private Type GetRuntimeType()
            {
                return Type;
            }

            public BindingParameter() { }

            public BindingParameter(bool canBeInternal)
            {
                this.canBeInternal = canBeInternal;
            }

            public static bool IsValidType(Type type)
            {
                if (type == typeof(bool))
                    return true;
                else if (type == typeof(Bounds))
                    return true;
                else if (type == typeof(Color))
                    return true;
                else if (type == typeof(float))
                    return true;
                else if (type == typeof(int))
                    return true;
                else if (type == typeof(Rect))
                    return true;
                else if (typeof(UnityEngine.Object).IsAssignableFrom(type))
                    return true;
                else if (type == typeof(string))
                    return true;
                else if (type == typeof(Vector2))
                    return true;
                else if (type == typeof(Vector3))
                    return true;
                else if (type == typeof(Vector4))
                    return true;

                return false;
            }

            public enum BindingType
            {
                Internal,
                Static,
                External
            }

            private enum BindingValueType
            {
                None,

                Boolean,
                Integer,
                Float,
                String,
                Vector2,
                Vector3,
                Vector4,
                Color,
                Rect,
                Bounds,
                Reference
            }

            private object Invoke()
            {
                if (gameObject == null || component == null || string.IsNullOrEmpty(method))
                    return null;

                MethodInfo info = GetMethodInfo();
                if (info == null)
                    return null;

                return info.Invoke(component, new object[0]);
            }

            public bool Copiable(object destination)
            {
                BindingParameter target = destination as BindingParameter;
                if (target == null)
                    return false;

                if (target.type != type)
                    return false;

                if (!target.canBeInternal && binding == BindingType.Internal)
                    return false;

                return true;
            }

            public object Copy(object destination)
            {
                BindingParameter target = destination as BindingParameter;

                BindingParameter copy = new BindingParameter();
                if (target != null)
                    copy.canBeInternal = target.canBeInternal;
                else
                    copy.canBeInternal = canBeInternal;

                copy.binding = binding;
                copy.boolValue = boolValue;
                copy.boundsValue = boundsValue;
                copy.colorValue = colorValue;
                copy.component = component;
                copy.floatValue = floatValue;
                copy.gameObject = gameObject;
                copy.intValue = intValue;
                copy.method = method;
                copy.qualifiedTypeName = qualifiedTypeName;
                copy.rectValue = rectValue;
                copy.referenceValue = referenceValue;
                copy.stringValue = stringValue;
                copy.type = type;
                copy.vector2Value = vector2Value;
                copy.vector3Value = vector3Value;
                copy.vector4Value = vector4Value;

                return copy;
            }

            public override string ToString()
            {
                return type.ToString();
            }
        }
    }
}