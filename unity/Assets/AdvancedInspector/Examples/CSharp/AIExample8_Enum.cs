using UnityEngine;
using System.Collections;

using AdvancedInspector;

[AdvancedInspector]
public class AIExample8_Enum : MonoBehaviour 
{
    [Inspect]
    public MyEnum normalEnum;

    // The enum attribute is used to control how an enum is displayed.
    // The first parameter is a switch between a normal 1-choice enum, and a masked enum.
    [Inspect, Enum(true)]
    public MyEnum maskedEnum;

    // An enum can also be display as checkboxes or buttons
    [Inspect, Enum(EnumDisplay.Checkbox)]
    public MyEnum checkboxEnum;

    [Inspect]
    public ByteEnum byteEnum;
}

public enum MyEnum
{
    // Masked enum - or bitfield - must follow a bitwise values. (1, 2, 4, 8, 16, 32, 64, etc)
    // Otherwise the bitfield cannot be save properly.
    // Unity has issue with a masked value of an enum having a 0 value.
    // None = 0,
    FirstValue = 1,
    SecondValue = 2,
    ThirdValue = 4,
    ForthValue = 8
}

public enum ByteEnum : byte
{ 
    One = 1,
    Two = 2,
    Three = 3
}