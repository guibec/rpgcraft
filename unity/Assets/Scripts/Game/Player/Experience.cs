using System;
using UnityEngine.Assertions;

public struct Experience_Data
{
    public int xp;
}

public class Experience
{
    public delegate void XPChangedEventHandler(object sender, System.EventArgs e);
    public event XPChangedEventHandler Changed;

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

    private Experience_Data m_experienceData;
    public Experience_Data ExperienceData
    {
        get
        {
            return m_experienceData;
        }

        set
        {
            m_experienceData = value;
            Changed?.Invoke(this, new EventArgs());
        }
    }

    public int XP
    {
        get
        {
            return ExperienceData.xp;
        }
        private set
        {
            if (value !=  ExperienceData.xp)
            {
                m_experienceData.xp = value;
                Changed?.Invoke(this, new EventArgs());
            }
        }
    }

    public int GetXPRequiredForNextLevel()
    {
        if (Level == MaxLevel)
        {
            return int.MaxValue;
        }

        return NextLevels[Level];
    }

    public int GetXPRequiredForCurrentLevel()
    {
        return NextLevels[Level - 1];
    }

    public int MaxLevel
    {
        get
        {
            return NextLevels.Length;
        }
    } 
                               // How much you need to achieve Level1, Level2, ...
    private int[] NextLevels = { 0, 50, 150, 375, 790, 1400, 2300, 3300};

    public void AddXP(int amount)
    {
        XP += amount;
    }

}
