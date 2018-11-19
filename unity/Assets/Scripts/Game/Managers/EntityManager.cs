using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class EntityManager : MonoSingleton<EntityManager>
{
    public GameObject m_attackPrefab;
    private static int m_nextId = 0;

    List<Entity> m_entities = new List<Entity>();

    // All known players (cache of m_entities)
    List<Player> m_players = new List<Player>();

    private void RegisterPlayer(Player player_)
    {
        //Debug.Assert(!m_players.Contains(player_), String.Format("Player {0} is already registered.", player_));
        m_players.Add(player_);
    }

    private void UnregisterPlayer(Player player_)
    {
        //Debug.Assert(m_players.Contains(player_), String.Format("Player {0} is not registered.", player_));
        m_players.Remove(player_);
    }

    public int Register(Entity entity_)
    {
        if (entity_ is Player)
            RegisterPlayer(entity_ as Player);

        //Debug.Assert(!m_entities.Contains(entity_), String.Format("Entity {0} is already registered.", entity_));
        m_entities.Add(entity_);

        return ++m_nextId;
    }

    public void Unregister(Entity entity_)
    {
        //Debug.Assert(m_entities.Contains(entity_), String.Format("Entity {0} is not registered.", entity_));
        m_entities.Remove(entity_);

        if (entity_ is Player)
            UnregisterPlayer(entity_ as Player);

        //OnDestroy.Invoke(entity_);
    }

    public Player Player
    {
        get
        {
            if (m_players.Count > 0)
                return m_players[0];
            else
                return null;
        }
    }

    public int Count<T>()
    {
        //return m_entities.Count(e => e is T);
        int count = 0;
        foreach (var entity in m_entities)
        {
            if (entity is T)
                count++;
        }
        return count;
    }

	public IEnumerable<Entity> Entities
    {
        get { return m_entities.AsReadOnly(); }
    }
}
