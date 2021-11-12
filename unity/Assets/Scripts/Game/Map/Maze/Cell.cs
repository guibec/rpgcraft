using System;
using UnityEngine;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using Debug = UnityEngine.Debug;

public class Cell
{
    /// <summary>
    /// Current x and y position of the cell
    /// North-West Corner is 0, 0
    /// Going eastward increases X value
    /// Going southward increases Y value
    /// </summary>
    public int x, y;

    /// <summary>
    /// Any neighbors, null if non existant, doesn't mean it is accessible
    /// </summary>
    public Cell North, East, South, West;

    public HashSet<Cell> Neighbors
    {
        get
        {
            var neighbors = new HashSet<Cell>();
            if (North != null) neighbors.Add(North);
            if (South != null) neighbors.Add(South);
            if (East != null) neighbors.Add(East);
            if (West != null) neighbors.Add(West);

            return neighbors;
        }
    }

    /// <summary>
    /// Neighbors you can access
    /// </summary>
    private HashSet<Cell> Links = new HashSet<Cell>();

    public Cell(int x_, int y_)
    {
        x = x_;
        y = y_;
    }

    /// <summary>
    /// Link two cells together
    /// </summary>
    /// <param name="other"></param>
    public void AddLink(Cell other)
    {
        Debug.Assert(other != null);

        Links.Add(other);
        other.Links.Add(this);
    }

    public void RemoveLink(Cell other)
    {
        Links.Remove(other);
        if (other != null)
        {
            other.Links.Remove(this);
        }
    }

    /// <summary>
    /// Get all the cells accessible from this cell
    /// </summary>
    /// <returns></returns>
    public HashSet<Cell> GetLinks()
    {
        return Links;
    }

    /// <summary>
    /// Check if a specific cell is linked from this cell
    /// </summary>
    /// <param name="other"></param>
    /// <returns></returns>
    public bool IsLinked(Cell other)
    {
        if (other == null) return false;
        return Links.Contains(other);
    }
};