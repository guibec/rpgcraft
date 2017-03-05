using System.Collections;
using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample9_Method : MonoBehaviour
    {
        // They show up as a button.
        [Inspect]
        private void MyMethod()
        {
            Debug.Log("You pressed my button");
        }

        // Using the Method attribute, with the Invoke variable, this function is called every time the inspector is refreshed.
        // This way, you can perform action or draw stuff on the inspector.
        [Inspect, Method(MethodDisplay.Invoke)]
        private void MyInvokedMethod()
        {
            GUILayout.Label("This was drawn from an invoked method.");
        }

        // You can also inspect and call static method.
        [Inspect]
        private static void MyStaticMethod()
        {
            Debug.Log("You pressed my static method button");
        }

        // Since 1.68, coroutines can also be invoked in the editor.
        // This method print a log every second for 25 seconds.
        [Inspect, Method(true)]
        private IEnumerator MyCoroutine()
        {
            int tick = 0;
            while (tick < 25)
            {
                yield return new WaitForSeconds(1);
                tick++;
                Debug.Log("Tick");
            }
        }
    }
}