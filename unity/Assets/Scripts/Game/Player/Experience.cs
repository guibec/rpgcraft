﻿using UnityEngine;
using System.Collections;
using UnityEngine.Assertions;

public class Experience
{
    public Experience()
    {
        XP = 0;
    }

    public int Level
    {
        get
        {
            for (int i = MaxLevel-1; i >= 0; --i)
            {
                if (XP >= NextLevels[i])
                    return i + 1;
            }

            Assert.IsTrue(false, "Invalid level state");
            return 1;
        }
    }

    public int XP { get; private set; }

    public int MaxLevel
    {
        get
        {
            return NextLevels.Length;
        }
    } 

    private int[] NextLevels = { 0, 50, 150, 375, 790, 1400, 2300, 3300};

}