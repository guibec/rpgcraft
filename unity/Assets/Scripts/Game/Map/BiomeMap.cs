using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BiomeMap
{
    private const int m_width = 256;
    private const int m_height = 256;

    EBiome[,] m_biomes = new EBiome[m_width, m_height];

    // We build build this based on https://gamedev.stackexchange.com/questions/79049/generating-tile-map
    // and https://en.wikipedia.org/wiki/Fortune's_algorithm
    // and http://www-cs-students.stanford.edu/~amitp/game-programming/polygon-map-generation/
    // 
    public BiomeMap()
    {
        const int NumNodes = 400;

        // For the purpose of this exercise, we will put all the numbers between 0.0f and 1.0f
        // we will remap them into the m_biomes after


    }
}
