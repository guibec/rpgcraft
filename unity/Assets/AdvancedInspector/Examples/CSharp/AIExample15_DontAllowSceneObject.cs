using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample15_DontAllowSceneObject : MonoBehaviour
    {
        // "DontAllowSceneObject" does exactly what it means; it prevent this field from being able to target an object in the scene.
        // It is useful if you want to force a user to target a prefab.
        [DontAllowSceneObject]
        public GameObject myProjectObject;

        // Fields that can target a scene object have the "picker" icon tool on its right.
        public GameObject mySceneObject;
    }
}