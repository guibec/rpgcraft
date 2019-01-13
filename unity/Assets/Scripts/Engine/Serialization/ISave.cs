using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public interface ISave
{
    /**
     * Returns an object that can be serialized for persistent storage
     */
    object Save(); 
}