using System;

// If we need something more extensive, we should investigate: https://www.codeproject.com/Articles/19028/Building-a-Generic-Range-class

/// <summary>The Range class.</summary>
/// <typeparam name="T">Generic parameter.</typeparam>
public class Range<T> where T : IComparable<T>
{
    /// <summary>Minimum value of the range.</summary>
    public T Minimum { get; set; }

    /// <summary>Maximum value of the range.</summary>
    public T Maximum { get; set; }

    public Range(T min, T max)
    {
        Minimum = min;
        Maximum = max;
    }

    /// <summary>Presents the Range in readable format.</summary>
    /// <returns>String representation of the Range</returns>
    public override string ToString()
    {
        return $"[{Minimum} - {Maximum}]";
    }

    /// <summary>Determines if the range is valid.</summary>
    /// <returns>True if range is valid, else false</returns>
    public bool IsValid()
    {
        return Minimum.CompareTo(Maximum) <= 0;
    }

    /// <summary>Determines if the provided value is inside the range.</summary>
    /// <param name="value">The value to test</param>
    /// <returns>True if the value is inside Range, else false</returns>
    public bool ContainsValue(T value)
    {
        return (Minimum.CompareTo(value) <= 0) && (value.CompareTo(Maximum) <= 0);
    }

    /// <summary>Determines if this Range is inside the bounds of another range.</summary>
    /// <param name="Range">The parent range to test on</param>
    /// <returns>True if range is inclusive, else false</returns>
    public bool IsInsideRange(Range<T> range)
    {
        return IsValid() && range.IsValid() && range.ContainsValue(Minimum) && range.ContainsValue(Maximum);
    }

    /// <summary>Determines if another range is inside the bounds of this range.</summary>
    /// <param name="Range">The child range to test</param>
    /// <returns>True if range is inside, else false</returns>
    public bool ContainsRange(Range<T> range)
    {
        return IsValid() && range.IsValid() && ContainsValue(range.Minimum) && ContainsValue(range.Maximum);
    }
}
