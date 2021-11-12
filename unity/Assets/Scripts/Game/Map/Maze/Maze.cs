using System;
using UnityEngine;
using System.Collections.Generic;

public class Maze
{
    public int Width { private set; get; }
    public int Height { private set; get; }

    private Cell[,] cells;

    public int Size { get { return Width * Height; } }
    public Cell GetRandomCell()
    {
        int x = RandomManager.Next(0, Width - 1);
        int y = RandomManager.Next(0, Height - 1);
        return this[x, y];
    }

    public Maze(int width, int height)
    {
        Width = width;
        Height = height;

        // Prepare the grid itself
        cells = new Cell[Width, Height];

        for (int x = 0; x < Width; x++)
        {
            for (int y = 0; y < Height; y++)
            {
                cells[x, y] = new Cell(x, y);
            }
        }

        for (int x = 0; x < Width; x++)
        {
            for (int y = 0; y < Height; y++)
            {
                cells[x, y].North = this[x, y - 1];
                cells[x, y].South = this[x, y + 1];
                cells[x, y].West = this[x - 1, y];
                cells[x, y].East = this[x + 1, y];
            }
        }
    }

    public Cell this[int x, int y]
    {
        get
        {
            if (x < 0 || x >= Width) return null;
            if (y < 0 || y >= Height) return null;

            return cells[x, y];
        }
    }

    public override string ToString()
    {
        string output = "+";
        for (int x = 0; x < Width; x++)
        {
            output += "---+";
        }
        output += "\n";

        for (int y = 0; y < Height; y++)
        {
            string top = "|";
            string bottom = "+";

            for (int x = 0; x < Width; x++)
            {
                var cell = cells[x, y];

                string body = "   ";
                string east_boundary = cell.IsLinked(cell.East) ? " " : "|";

                top += body + east_boundary;

                string south_boundary = cell.IsLinked(cell.East) ? "   " : "---";
                string corner = "+";
                bottom += south_boundary + corner;
            }

            output += top + "\n";
            output += bottom + "\n";
        }

        return output;
    }

    public IEnumerable<Cell> GetCells()
    {
        for (int x = 0; x < Width; x++)
        {
            for (int y = 0; y < Height; y++)
            {
                yield return cells[x, y];
            }
        }
    }


    public void Generate()
    {
        GenerateBinaryTree();
    }

    private void GenerateBinaryTree()
    {
        foreach (var cell in GetCells()) {
            var neighbors = new List<Cell>();
            if (cell.North != null) neighbors.Add(cell.North);
            if (cell.East != null) neighbors.Add(cell.East);

            // possible we have no index
            if (neighbors.Count == 0)
            {
                continue;
            }
       
            int index = RandomManager.Next(0, neighbors.Count);
            cell.AddLink(neighbors[index]);
        }
    }
}



