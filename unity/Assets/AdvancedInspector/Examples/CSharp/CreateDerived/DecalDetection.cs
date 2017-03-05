using UnityEngine;
using AdvancedInspector;

public class DecalDetection : MonoBehaviour
{
    [Inspect]
    public bool AnyDectected { get; private set; }
    [Inspect]
    private int numberOnDecalsDetected = 0;

    private void OnTriggerEnter(Collider decalCol)
    {
        if (decalCol.transform.CompareTag(""))
        {
            AnyDectected = true;
            numberOnDecalsDetected++;
        }
    }

    private void OnTriggerExit(Collider decalCol)
    {
        if (decalCol.transform.CompareTag(""))
        {
            numberOnDecalsDetected--;
            if (numberOnDecalsDetected <= 0)
            {
                AnyDectected = false;
            }
        }
    }
}