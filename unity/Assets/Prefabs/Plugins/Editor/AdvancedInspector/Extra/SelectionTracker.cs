using System.Collections.Generic;
using System.Linq;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [InitializeOnLoad]
    public sealed class SelectionTracker
    {
        private static List<List<UnityEngine.Object>> selections = new List<List<UnityEngine.Object>>();
        private static int index = 0;

        static SelectionTracker()
        {
            EditorApplication.update += Update;
        }

        [MenuItem("Edit/Selection/Previous Selection %LEFT")]
        private static void Previous()
        {
            if (index == 0)
                return;

            index--;
            Selection.objects = selections[index].ToArray();
        }

        [MenuItem("Edit/Selection/Next Selection %RIGHT")]
        private static void Next()
        {
            if (index >= selections.Count - 1)
                return;

            index++;
            Selection.objects = selections[index].ToArray();
        }

        private static void Update()
        {
            if (selections.Count == 0)
            {
                AddSelection();
                return;
            }

            TestCache();

            if (selections.Count == 0 && Selection.objects.Length > 0)
                AddSelection();

            if (selections.Count == 0)
                return;
            else
                index = Mathf.Clamp(index, 0, selections.Count - 1);

            if (index == -1 || selections.Count <= index)
                return;

            UnityEngine.Object[] current = Selection.objects;
            UnityEngine.Object[] previous = selections[index].ToArray();

            if (previous.Length != current.Length)
            {
                AddSelection();
                return;
            }

            for (int i = 0; i < current.Length; i++)
                if (current[i] != previous[i])
                    AddSelection();
        }

        private static void AddSelection()
        {
            if (Selection.objects.Length == 0)
                return;

            selections.Add(Selection.objects.ToList());
            index = selections.Count - 1;

            if (selections.Count > 20)
                for (int i = selections.Count - 20; i >= 0; i--)
                    selections.RemoveAt(i);
        }

        private static void TestCache()
        {
            for (int i = selections.Count - 1; i >= 0; i--)
            {
                List<UnityEngine.Object> collection = selections[i];
                for (int j = collection.Count - 1; j >= 0; j--)
                {
                    if (collection[j] == null || !collection[j])
                        collection.RemoveAt(j);
                }

                if (collection.Count == 0)
                    selections.RemoveAt(i);
            }
        }
    }
}