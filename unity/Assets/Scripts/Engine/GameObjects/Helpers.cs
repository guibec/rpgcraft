using UnityEngine;

public static class Helpers 
{
    public static GameObject FindChildWithName(GameObject parent, string name)
    {
        if (parent.name.Equals(name)) return parent;
        foreach (Transform child in parent.transform)
        {
            GameObject result = FindChildWithName(child.gameObject, name);
            if (result != null) return result;
        }
        return null;
    }
}
