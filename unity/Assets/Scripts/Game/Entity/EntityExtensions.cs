﻿using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

public static class EntityExtensions
{
    public static EntitiesWithinEntityRadiusEnumerator EntitiesWithinRadius(this Entity source, float radius)
    {
        return CollisionManager.Instance.EntitiesWithinEntityRadius(source, radius);
    }
}