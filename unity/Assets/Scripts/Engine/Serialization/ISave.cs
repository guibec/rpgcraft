using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public interface ISave
{
    /**
     * Return a string representation of the object that can be saved to a persistent store
     */
    string Save(); 
}