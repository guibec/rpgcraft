using UnityEngine;
using UnityEditor;
using System;
using System.Collections;

namespace AdvancedInspector
{
    public class DateTimeEditor : FieldEditor, IModal
    {
        private InspectorField field;

        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(DateTime) }; }
        }

        private static Texture calendar;

        internal static Texture Calendar
        {
            get
            {
                if (calendar == null)
                    calendar = Helper.Load(EditorResources.Calendar);

                return calendar;
            }
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            DateTime time = field.GetValue<DateTime>();
            if (time == DateTime.MinValue)
                time = DateTime.Now;

            string title = time.ToString();
            if (field.Mixed)
                title = " - - - ";

            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.Space();
            if (GUILayout.Button(Calendar, GUIStyle.none, GUILayout.Width(18), GUILayout.Height(18)))
            {
                this.field = field;
                DateTimeDialog.Create(this, time, GUIUtility.GUIToScreenPoint(Event.current.mousePosition));
            }

            TextAnchor anchor = GUI.skin.label.alignment;
            GUI.skin.label.alignment = TextAnchor.MiddleLeft;
            GUILayout.Label(title);
            GUI.skin.label.alignment = anchor;

            GUILayout.FlexibleSpace();
            EditorGUILayout.EndHorizontal();
        }

        public void ModalRequest(bool shift) { }

        public void ModalClosed(ModalWindow window) 
        {
            if (window.Result != WindowResult.Ok)
                return;

            field.SetValue(((DateTimeDialog)window).Time);
        }
    }
}