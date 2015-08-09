using UnityEngine;
using UnityEditor;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UniToolsEditor;

namespace AdvancedInspector
{
    public class ObjectEditor : FieldEditor
    {
        private static Texture picker;

        private static Texture pickerPro;

        private static Texture Picker
        {
            get
            {
                if (EditorGUIUtility.isProSkin)
                {
                    if (pickerPro == null)
                        pickerPro = Helper.Load(EditorResources.PickerPro);

                    return pickerPro;
                }
                else
                {
                    if (picker == null)
                        picker = Helper.Load(EditorResources.Picker);

                    return picker;
                }
            }
        }

        private static int s_ObjectFieldHash = "s_ObjectFieldHash".GetHashCode();
        private static Type validator = null;
        private static MethodInfo doObjectField = null;

        public override bool EditDerived
        {
            get { return true; }
        }

        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(UnityEngine.Object) }; }
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            if (field.GetAttribute<NoPicker>() != null)
            {
                object obj = field.GetValue();
                if (field.Mixed)
                    GUILayout.Label("---");
                else if (obj == null)
                    GUILayout.Label("None");
                else if (field.OverloadToString)
                    GUILayout.Label(obj.ToString());
                else if (field.Type != null)
                    GUILayout.Label(field.Type.Name);

                return;
            }

            if (field.DisplayAsParent)
                return;

            if (validator == null)
                validator = typeof(EditorGUI).GetNestedType("ObjectFieldValidator", BindingFlags.NonPublic);

            if (doObjectField == null)
                doObjectField = typeof(EditorGUI).GetMethod("DoObjectField", BindingFlags.NonPublic | BindingFlags.Static, null, new Type[] { typeof(Rect), typeof(Rect), typeof(int),
                                            typeof(UnityEngine.Object), typeof(Type), typeof(SerializedProperty), validator, typeof(bool), typeof(GUIStyle) }, null);

            DontAllowSceneObjectAttribute dontAllow = field.GetAttribute<DontAllowSceneObjectAttribute>(); ;

            UnityEngine.Object value = (UnityEngine.Object)GetValue(field);

            EditorGUILayout.BeginHorizontal();
            EditorGUI.BeginChangeCheck();
                
            Type type = field.Type;

            UnityEngine.Object result = null;

            if (type.IsInterface)
            {
                Rect position = EditorGUILayout.GetControlRect(false, 16f);
                int id = GUIUtility.GetControlID(s_ObjectFieldHash, EditorGUIUtility.native, position);
                Delegate validation = Delegate.CreateDelegate(validator, typeof(ObjectEditor).GetMethod("ValidateObjectFieldAssignment", BindingFlags.NonPublic | BindingFlags.Static));

                result = doObjectField.Invoke(null, new object[] { position, position, id, value, type, null, validation, dontAllow == null, EditorStyles.objectField } ) as UnityEngine.Object;
            }
            else
                result = EditorGUILayout.ObjectField(value, type, dontAllow == null);

            if (EditorGUI.EndChangeCheck())
                field.SetValue(result);

            if (dontAllow == null && (field.Type == typeof(GameObject) || 
                typeof(Component).IsAssignableFrom(field.Type) || field.Type.IsAssignableFrom(typeof(Component))))
                if (GUILayout.Button(Picker, GUIStyle.none, GUILayout.Width(18), GUILayout.Height(18)))
                    InspectorEditor.StartPicking(Picked, field);

            EditorGUILayout.EndHorizontal();

            DrawObjectSelector(field);
        }

        private static UnityEngine.Object ValidateObjectFieldAssignment(UnityEngine.Object[] references, Type objType, SerializedProperty property)
        {
            if (references.Length > 0)
            {
                if (((references[0] != null) && (references[0].GetType() == typeof(GameObject))))
                {
                    foreach (UnityEngine.Object obj in ((GameObject)references[0]).GetComponents(typeof(Component)))
                    {
                        if ((obj != null) && objType.IsAssignableFrom(obj.GetType()))
                        {
                            return obj;
                        }
                    }
                }
            }

            return null;
        }

        private void Picked(GameObject go, object tag)
        {
            InspectorField field = tag as InspectorField;
            if (field == null)
                return;

            if (field.Type == typeof(GameObject))
                field.SetValue(go);
            else if (typeof(Component).IsAssignableFrom(field.Type))
                field.SetValue(go.GetComponent(field.Type));
        }

        public override void OnLabelDoubleClick(InspectorField field)
        {
            UnityEngine.Object target = field.GetValue() as UnityEngine.Object;
            if (target == null)
                return;

            if (!string.IsNullOrEmpty(AssetDatabase.GetAssetPath(target)))
                EditorGUIUtility.PingObject(target);
            else
            {
                SceneView view = SceneView.lastActiveSceneView;
                Quaternion rotation = view.camera.transform.rotation;

                if (target is GameObject)
                    SceneView.lastActiveSceneView.LookAt(((GameObject)target).transform.position, rotation, 10);
                else if (target is Component)
                    SceneView.lastActiveSceneView.LookAt(((Component)target).transform.position, rotation, 10);
            }
        }

        private void DrawObjectSelector(InspectorField field)
        {
            MonoBehaviour behaviour = field.GetValue() as MonoBehaviour;
            if (behaviour == null)
                return;

            List<Component> components = new List<Component>(behaviour.gameObject.GetComponents(field.BaseType));
            if (components.Count == 1)
                return;

            int index = components.IndexOf(behaviour);
            string[] texts = new string[components.Count];

            for (int i = 0; i < components.Count; i++)
                texts[i] = i.ToString() + " : " + components[i].ToString();

            EditorGUILayout.BeginHorizontal();
            int selection = EditorGUILayout.Popup(index, texts);
            EditorGUILayout.EndHorizontal();

            if (selection == index)
                return;

            field.SetValue(components[selection]);
        }
    }
}